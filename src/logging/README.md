Logging

Purpose
 - Provide an append-only audit stream of important application events (save, autosave, create, open, trash, restore, purge, snapshot, encrypt/decrypt). The audit subsystem is designed for observability and simple recovery analysis.

Files and responsibilities
 - `AuditLogger.h` / `AuditLogger.cpp`: Implements an append-only logger that writes structured entries to an audit file (timestamp, event type, note id, optional metadata). Emits `logAppended` Qt signals so the UI (`AuditLogPanel`) can subscribe and update in real time.

Design considerations
 - The audit log is append-only by design to simplify integrity assumptions and reduce risk of accidental data loss. If entries must be removed for privacy reasons, provide a well-documented exported/erase workflow rather than in-place edits.
 - Keep the log format stable and machine-parseable (JSON lines or a compact binary representation) so external tools can consume it.

Integration
 - `NoteService` and other services should write structured audit events rather than free-form text. UI panels subscribe to `AuditLogger::logAppended` to show recent events.

Testing
 - Tests should verify that expected events are emitted during save/restore/purge flows and that `AuditLogger` correctly writes entries to disk. Use a temporary file during tests and ensure it is removed afterwards.

Cross references
 - See `src/ui/README.md` for how the `AuditLogPanel` consumes audit events and `tests/TESTS.md` for integration tests that assert audit entries.
