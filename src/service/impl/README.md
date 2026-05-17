Service implementation details

Files:
- `NoteService.h` / `NoteService.cpp`: Implements high-level note lifecycle operations (create, save, load, search, trash). Delegates persistence to `SqliteNoteRepository` and exposes convenience helpers such as `loadNoteRobust` which can detect if a password is required.
- `SnapshotService.h` / `SnapshotService.cpp`: Manages snapshot creation, retrieval, deletion and the revert workflow. Orchestrates safety-snapshot creation and enforces snapshot retention/pruning policy.
- `TrashService.h` / `TrashService.cpp`: Implements soft-delete (trash), restore, purge and retention/purge scheduling logic. Uses the repository to modify trashed state and to permanently delete notes when purged.

Notes for maintainers:
- Keep business logic here; do not perform direct SQL operations in service layer — use `SqliteNoteRepository`.
- Unit tests for these classes should use an in-memory SQLite database (`":memory:"`) to avoid file I/O and to ensure deterministic tests.
