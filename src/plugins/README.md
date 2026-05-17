Plugins

Files:
- `PluginManager.h` / `PluginManager.cpp`: Runtime plugin registry used by the UI to discover formatting or note-type plugins.
- `PlaintextPlugin.h` / `PlaintextPlugin.cpp`: Simple plaintext formatting plugin used for basic notes.
- `MarkdownPlugin.h` / `MarkdownPlugin.cpp`: Markdown rendering plugin used for notes with markdown content.
- `MarkdownFormattingPlugin.h`: Extension for markdown-specific formatting actions.
- `IFormattingAction.h`: Interface for plugin formatting actions.

Notes:
- Plugins are lightweight and UI-oriented. Keep plugin logic separated from persistence and services.
