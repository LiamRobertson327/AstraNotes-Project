Logging

Files:
- `AuditLogger.h` / `AuditLogger.cpp`: Append-only audit logger used to record application events (save, autosave, create, open, trash, restore, purge, snapshot, encrypt/decrypt). Emits `logAppended` signals for UI panels to display new entries.

Notes:
- The audit log file is intentionally append-only and should not be modified in-place.
