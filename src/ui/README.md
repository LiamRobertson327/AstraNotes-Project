UI layer

Files:
- `mainwindow.ui` / `mainwindow.h` / `mainwindow.cpp`: Main application window, builds the layout, wires UI actions to services.
- `NoteListController.h` / `NoteListController.cpp`: Sidebar controller managing pagination, selection, and updates to the saved-notes list.
- `AuditLogPanel.h` / `AuditLogPanel.cpp`: Read-only panel that displays append-only audit log entries.
- `SettingsDialog.h` / `SettingsDialog.cpp`: Application settings UI (auto-save, retention, etc.).
- `TrashDialog.h` / `TrashDialog.cpp`: Dialog to view/restore/purge trashed notes; uses `ITrashService` interface.

Notes:
- UI code should depend only on service interfaces (`src/service/interfaces`) rather than concrete implementations when possible.
