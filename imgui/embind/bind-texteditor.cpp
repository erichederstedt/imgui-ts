// Embind wrapper exposing ImGuiColorTextEdit (TextEditor) to imgui-ts.
// Compiled into the same WASM module as Dear ImGui, so it draws into the
// current ImGui window like any other widget.
//
// Line numbers in this API are 1-based (matching GLSL compiler error logs).

#include "imgui.h"
#include "TextEditor.h"

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <algorithm>
#include <string>
#include <vector>

using emscripten::val;

class WrapTextEditor
{
public:
    WrapTextEditor()
    {
        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
        editor.SetPalette(TextEditor::GetDarkPalette());
    }

    // --- text ---
    void SetText(const std::string& text) { editor.SetText(text); }
    std::string GetText() const { return editor.GetText(); }
    bool IsTextChanged() const { return editor.IsTextChanged(); }
    int GetTotalLines() const { return editor.GetTotalLines(); }
    std::string GetSelectedText() const { return editor.GetSelectedText(); }

    // --- drawing (call between ImGui.Begin/End) ---
    void Render(const std::string& title, float width, float height, bool border)
    {
        editor.Render(title.c_str(), ImVec2(width, height), border);
    }

    // --- options ---
    void SetReadOnly(bool value) { editor.SetReadOnly(value); }
    bool IsReadOnly() const { return editor.IsReadOnly(); }
    void SetTabSize(int value) { editor.SetTabSize(value); }
    void SetShowWhitespaces(bool value) { editor.SetShowWhitespaces(value); }
    void SetHandleKeyboardInputs(bool value) { editor.SetHandleKeyboardInputs(value); }
    void SetHandleMouseInputs(bool value) { editor.SetHandleMouseInputs(value); }
    void SetGutterTogglesBreakpoints(bool value) { editor.SetGutterTogglesBreakpoints(value); }

    // "dark" | "light" | "retroblue"
    void SetPalette(const std::string& name)
    {
        if (name == "light")          editor.SetPalette(TextEditor::GetLightPalette());
        else if (name == "retroblue") editor.SetPalette(TextEditor::GetRetroBluePalette());
        else                          editor.SetPalette(TextEditor::GetDarkPalette());
    }

    // "glsl" | "hlsl" | "c" | "cpp"
    void SetLanguage(const std::string& name)
    {
        if (name == "hlsl")     editor.SetLanguageDefinition(TextEditor::LanguageDefinition::HLSL());
        else if (name == "c")   editor.SetLanguageDefinition(TextEditor::LanguageDefinition::C());
        else if (name == "cpp") editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
        else                    editor.SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
    }

    // --- cursor (line 1-based, column 0-based with tabs expanded) ---
    int GetCursorLine() const { return editor.GetCursorPosition().mLine + 1; }
    int GetCursorColumn() const { return editor.GetCursorPosition().mColumn; }
    void SetCursorPosition(int line, int column)
    {
        editor.SetCursorPosition(TextEditor::Coordinates(std::max(0, line - 1), std::max(0, column)));
    }

    // --- undo ---
    bool CanUndo() const { return editor.CanUndo(); }
    bool CanRedo() const { return editor.CanRedo(); }
    void Undo() { editor.Undo(); }
    void Redo() { editor.Redo(); }

    // --- breakpoints ---
    bool ToggleBreakpoint(int line) { return editor.ToggleBreakpoint(line); }
    bool ConsumeBreakpointsChanged() { return editor.ConsumeBreakpointsChanged(); }

    // Takes a JS array of line numbers.
    void SetBreakpoints(val lines)
    {
        TextEditor::Breakpoints bps;
        const int n = lines["length"].as<int>();
        for (int i = 0; i < n; ++i)
            bps.insert(lines[i].as<int>());
        editor.SetBreakpoints(bps);
    }

    // Returns a sorted JS array of line numbers.
    val GetBreakpoints() const
    {
        std::vector<int> sorted(editor.GetBreakpoints().begin(), editor.GetBreakpoints().end());
        std::sort(sorted.begin(), sorted.end());
        val out = val::array();
        for (int l : sorted)
            out.call<void>("push", l);
        return out;
    }

    // --- execution line (0 = none) ---
    void SetExecutionLine(int line, bool scrollTo) { editor.SetExecutionLine(line, scrollTo); }
    int GetExecutionLine() const { return editor.GetExecutionLine(); }
    void SetExecutionLineColor(int r, int g, int b, int a) { editor.SetExecutionLineColor(IM_COL32(r, g, b, a)); }

    // --- error markers: JS array of { line: number, message: string } ---
    void SetErrorMarkers(val markers)
    {
        TextEditor::ErrorMarkers m;
        const int n = markers["length"].as<int>();
        for (int i = 0; i < n; ++i)
        {
            val item = markers[i];
            int line = item["line"].as<int>();
            std::string msg = item["message"].as<std::string>();
            auto it = m.find(line);
            if (it != m.end())
                it->second += "\n" + msg;   // several errors on one line
            else
                m.emplace(line, msg);
        }
        editor.SetErrorMarkers(m);
    }
    void ClearErrorMarkers() { editor.SetErrorMarkers(TextEditor::ErrorMarkers()); }

private:
    TextEditor editor;
};

EMSCRIPTEN_BINDINGS(TextEditor) {
    emscripten::class_<WrapTextEditor>("TextEditor")
        .constructor<>()
        .function("SetText", &WrapTextEditor::SetText)
        .function("GetText", &WrapTextEditor::GetText)
        .function("IsTextChanged", &WrapTextEditor::IsTextChanged)
        .function("GetTotalLines", &WrapTextEditor::GetTotalLines)
        .function("GetSelectedText", &WrapTextEditor::GetSelectedText)
        .function("Render", &WrapTextEditor::Render)
        .function("SetReadOnly", &WrapTextEditor::SetReadOnly)
        .function("IsReadOnly", &WrapTextEditor::IsReadOnly)
        .function("SetTabSize", &WrapTextEditor::SetTabSize)
        .function("SetShowWhitespaces", &WrapTextEditor::SetShowWhitespaces)
        .function("SetHandleKeyboardInputs", &WrapTextEditor::SetHandleKeyboardInputs)
        .function("SetHandleMouseInputs", &WrapTextEditor::SetHandleMouseInputs)
        .function("SetGutterTogglesBreakpoints", &WrapTextEditor::SetGutterTogglesBreakpoints)
        .function("SetPalette", &WrapTextEditor::SetPalette)
        .function("SetLanguage", &WrapTextEditor::SetLanguage)
        .function("GetCursorLine", &WrapTextEditor::GetCursorLine)
        .function("GetCursorColumn", &WrapTextEditor::GetCursorColumn)
        .function("SetCursorPosition", &WrapTextEditor::SetCursorPosition)
        .function("CanUndo", &WrapTextEditor::CanUndo)
        .function("CanRedo", &WrapTextEditor::CanRedo)
        .function("Undo", &WrapTextEditor::Undo)
        .function("Redo", &WrapTextEditor::Redo)
        .function("ToggleBreakpoint", &WrapTextEditor::ToggleBreakpoint)
        .function("ConsumeBreakpointsChanged", &WrapTextEditor::ConsumeBreakpointsChanged)
        .function("SetBreakpoints", &WrapTextEditor::SetBreakpoints)
        .function("GetBreakpoints", &WrapTextEditor::GetBreakpoints)
        .function("SetExecutionLine", &WrapTextEditor::SetExecutionLine)
        .function("GetExecutionLine", &WrapTextEditor::GetExecutionLine)
        .function("SetExecutionLineColor", &WrapTextEditor::SetExecutionLineColor)
        .function("SetErrorMarkers", &WrapTextEditor::SetErrorMarkers)
        .function("ClearErrorMarkers", &WrapTextEditor::ClearErrorMarkers)
        ;
}
