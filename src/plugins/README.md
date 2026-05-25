Plugins

Purpose
 - Provide a lightweight extension point for note types and UI formatting actions. Plugins let the UI discover formatting options (bold, italic, color, emoji, etc.) and note-type renderers (markdown, plaintext) without hard-coding behavior in the `MainWindow`.

Files and responsibilities
 - `PluginManager.h` / `PluginManager.cpp`: Runtime registry that discovers and holds plugin instances. UI queries `PluginManager` to get the active plugin for a note type and to collect `IFormattingAction` instances used to populate the formatting toolbar.
 - `PlaintextPlugin.h` / `PlaintextPlugin.cpp`: Plaintext note-type plugin. Provides formatting actions that operate on the editor (apply `QTextCharFormat` changes) and returns the appropriate editor widget type.
 - `MarkdownPlugin.h` / `MarkdownPlugin.cpp`: Markdown note-type plugin. Provides a renderer and markdown-specific formatting actions. The markdown plugin may expose actions that insert markdown tokens instead of rich-text char formats.
 - `MarkdownFormattingPlugin.h`: Extension defining markdown-specific formatting action helpers.
 - `IFormattingAction.h`: Interface for formatting actions. Actions implement a single `execute(QTextEdit*)`-style method and expose metadata used by the UI to render toolbar buttons (icon, tooltip).

How plugins integrate with the rest of the app
 - At runtime `MainWindow` (composition root) asks `PluginManager` for the plugin that handles a given note's `typeId`. The UI then requests formatting actions from that plugin and creates toolbar buttons bound to each action.
 - Formatting actions should be UI-only and stateless where possible. They must not perform persistence; if an action must change persisted note metadata it should call into a service via the composition root instead of reaching into the repository.

Ownership and lifecycle
 - `PluginManager` owns plugin instances for the app lifetime. UI code that creates action wrappers for toolbar buttons should document ownership rules (which side deletes action objects). Prefer smart pointers to make ownership explicit.

Testing and extension guidance
 - Unit tests can instantiate plugins directly and exercise `IFormattingAction` implementations in headless mode by using a `QTextDocument` and `QTextCursor` operations.
 - To add a new plugin: implement the plugin class, register it with `PluginManager` in the composition root, and add tests that verify action behavior and that `MainWindow` correctly populates the toolbar.

Cross references
 - See `src/ui/README.md` for how the UI consumes plugins and `src/model/README.md` for `typeId` usage in `Note`.
