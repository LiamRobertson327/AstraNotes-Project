Logging

Purpose
 - Provide an append-only local audit stream for important application events such as save, autosave, create, open, trash, restore, purge, snapshot, encrypt, and decrypt.

Files and responsibilities
 - `AuditLogger.h` / `AuditLogger.cpp`: Implements the Qt message handler and writes human-readable audit lines to `audit.log` in the application's writable data directory. It also emits `logAppended` so the UI can update in real time.

Storage
 - The log path is derived from `QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)` and falls back to the application directory if the standard location is unavailable.
 - On typical desktop installs this resolves to a per-user app data folder, not the repository tree.

Design considerations
 - The audit log is append-only to reduce the risk of accidental data loss.
 - The current format is plain text lines with timestamp, severity, and normalized action text.

Integration
 - `NoteService` and other services write log messages through Qt's logging system.
 - UI panels subscribe to `AuditLogger::logAppended` to show recent events without reading the file directly.

Testing
 - Tests should verify that expected events are emitted during save, restore, purge, and encryption flows.
 - Use a temporary profile or writable temp directory when testing file output.

Cross references
 - See `src/ui/README.md` for how the `AuditLogPanel` consumes audit events.
 - See `tests/TESTS.md` and `docs/TEST_VALIDATION_TRACEABILITY.md` for coverage and validation mapping.
