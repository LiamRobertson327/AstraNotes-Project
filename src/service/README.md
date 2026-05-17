Service layer

Purpose: provide thin service facades over repository operations and coordinate cross-repository workflows.

Structure:
- interfaces/: public service interfaces (INoteService, ISnapshotService, ITrashService)
- impl/: concrete implementations that depend on `SqliteNoteRepository` and other core components

Guidelines:
- Keep business rules in services, persistence details in `repository/`.
- Prefer writing unit tests for `impl/` classes using an in-memory SQLite database (`":memory:"`).
