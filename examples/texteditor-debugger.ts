// Example: GLSL source view for a shader debugger, using the TextEditor
// exported by this fork of imgui-ts.
import { ImGui, CreateTextEditor, TextEditor, TextEditorErrorMarker } from "../src/index";

let editor: TextEditor | null = null;

// Call once, after `await ImGui.default()` and ImGui_Impl.Init(...)
export function initShaderView(source: string): void {
    editor = CreateTextEditor();          // GLSL language, dark palette
    editor.SetText(source);
    editor.SetTabSize(4);
}

// Call every frame, inside your normal ImGui frame.
export function drawShaderView(onBreakpointsChanged: (lines: number[]) => void): void {
    if (!editor) return;

    ImGui.Begin("Shader");
    // 0,0 = fill the remaining space of the window
    editor.Render("##glsl", 0, 0, false);
    ImGui.End();

    // Gutter clicks, and edits that move lines, both set this flag.
    if (editor.ConsumeBreakpointsChanged())
        onBreakpointsChanged(editor.GetBreakpoints());
}

// Debugger paused at `line` (1-based): highlight it, lock editing, scroll to it.
export function onPaused(line: number): void {
    editor?.SetReadOnly(true);
    editor?.SetExecutionLine(line, true);
}

export function onResumed(): void {
    editor?.SetExecutionLine(0, false);
    editor?.SetReadOnly(false);
}

// Turn a WebGL shader info log into red error lines with hover tooltips.
// Format: "ERROR: 0:12: 'foo' : undeclared identifier"
export function showCompileErrors(gl: WebGL2RenderingContext, shader: WebGLShader): void {
    if (!editor) return;
    const log = gl.getShaderInfoLog(shader) ?? "";
    const markers: TextEditorErrorMarker[] = [];
    for (const m of log.matchAll(/^ERROR:\s*\d+:(\d+):\s*(.*)$/gm))
        markers.push({ line: Number(m[1]), message: m[2] });
    editor.SetErrorMarkers(markers);
}

export function disposeShaderView(): void {
    editor?.delete();                     // frees the C++ object
    editor = null;
}
