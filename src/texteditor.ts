import { bind } from "./imgui";
import * as Bind from "./bind-imgui";

export type TextEditor = Bind.TextEditor;
export type TextEditorErrorMarker = Bind.TextEditorErrorMarker;

/**
 * Create a code editor widget (ImGuiColorTextEdit). Must be called after
 * `await ImGui.default()` has loaded the WASM module. Starts in GLSL mode with
 * the dark palette. Free it with `editor.delete()` when you no longer need it.
 */
export function CreateTextEditor(): TextEditor {
    return new bind.TextEditor();
}
