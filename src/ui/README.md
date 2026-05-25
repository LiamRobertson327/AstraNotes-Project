UI layer

Purpose
 - The UI layer contains the Qt widgets, dialogs and controllers used to present notes to the user and forward user intent to the service layer. It is strictly a presentation / input-handling layer and should not contain persistence or business rules.

Files and responsibilities
 - `mainwindow.ui` / `mainwindow.h` / `mainwindow.cpp`: The composition root for the application UI. Builds the main layout (editor, list, toolbar), wires actions to service interfaces, creates concrete implementations at startup, and owns long-lived widgets. MainWindow is the place where concrete types are composed (e.g., `NoteService`, `SqliteNoteRepository`) and passed to controllers.
 - `NoteListController.h` / `NoteListController.cpp`: Encapsulates the left-hand notes list and pagination logic. Subscribes to `INoteService` to refresh lists and emits selection signals to the `MainWindow` or editor controller.
 - `AuditLogPanel.h` / `AuditLogPanel.cpp`: Read-only view bound to the `AuditLogger` (in `src/logging`). Receives `logAppended` signals and appends rows to the panel.
 - `SettingsDialog.h` / `SettingsDialog.cpp`: Modal dialog to display and mutate application settings (auto-save interval, snapshot retention, encryption defaults). Settings are persisted by services via the composition root.
 - `TrashDialog.h` / `TrashDialog.cpp`: Dialog exposing trash/restore/purge flows. Operates against `ITrashService` so it remains testable and implementation-agnostic.

How the UI interacts with other layers
 - Controllers and widgets talk to the service layer via interfaces defined in `src/service/interfaces` (for example, `INoteService`). This keeps UI code decoupled from concrete repositories and enables easier testing and mocking.
 - `MainWindow` acts as the composition root: it instantiates concrete services and repositories (only here), registers plugin-provided UI actions (via the `PluginManager`), and wires signals between controllers. Example flow: user clicks Save → `MainWindow` invokes `INoteService::save()` → `NoteService` delegates to `INoteRepository`.
 - UI widgets receive domain models (e.g., `Note`) and present content. For plaintext/HTML editing the editor widget serializes its state (via `toHtml()`/`toPlainText()`) and hands it to the service.

Ownership and lifecycle notes
 - The `MainWindow` owns major widgets and the lifetime of concrete services created at startup. Short-lived dialogs (settings, trash) should accept service interfaces in their constructors rather than constructing them.
 - Avoid storing raw pointers to services in controllers; prefer `std::shared_ptr` or explicit lifetime notes in the composition root to avoid dangling references.

Testing notes
 - Unit tests should target controllers by mocking service interfaces (see `tests/`). Integration / UI tests can use QtTest and a test composition root that wires an in-memory `SqliteNoteRepository`.

Cross references
 - See `src/service/README.md` for service responsibilities and `src/plugins/README.md` for how toolbar formatting actions are discovered and provided to the UI.
