Service layer

Purpose: provide thin service facades over repository operations and coordinate cross-repository workflows.

Structure:
- interfaces/: public service interfaces (INoteService, ISnapshotService, ITrashService)
- impl/: concrete implementations of those interfaces; services depend on `INoteRepository` (not concrete repository types)

Guidelines:
- Keep business rules in services, persistence details in `repository/`.
- Depend on abstractions across layers: UI/controllers use service interfaces, services use repository interfaces.
- Restrict concrete wiring (`SqliteNoteRepository`, `NoteService`, etc.) to the composition root.
- Prefer writing unit tests for `impl/` classes using an in-memory SQLite database (`":memory:"`).
