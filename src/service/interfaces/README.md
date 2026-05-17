Service interfaces

Files:
- `INoteService.h`: Public interface for note operations used by UI/controllers (load, save, create, search, trash). Implemented by `NoteService` in `impl/`.
- `ISnapshotService.h`: Interface for snapshot/version-history operations (save snapshot, list snapshots, get by id, delete, revert). Implemented by `SnapshotService`.
- `ITrashService.h`: Interface for trash/retention management (trash, restore, purge, list trashed). Implemented by `TrashService`.

Guidelines:
- Keep these headers stable: UI and controllers depend on them. Keep implementation details out of these files.
