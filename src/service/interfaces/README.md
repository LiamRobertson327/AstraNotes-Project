Service interfaces

Purpose
 - Define stable, minimal contracts that the UI and controllers use to interact with application behavior. Interfaces reduce coupling and make unit testing controllers straightforward.

Files and summary
 - `INoteService.h`: Core public contract for note operations used by the UI and controllers. Key responsibilities: load, save, create, search, trash, restore. Implemented by `NoteService` in `impl/`.
 - `ISnapshotService.h`: Contract for snapshot/version-history operations (save snapshot, list snapshots, get by id, delete, revert). Implemented by `SnapshotService`.
 - `ITrashService.h`: Contract for trash/retention management (trash, restore, purge, list trashed). Implemented by `TrashService`.

Design guidance and interaction
 - Keep these interfaces small and stable. UI code should only depend on these headers; implementation changes should not require UI recompilation.
 - Prefer returning value objects or smart pointers rather than raw pointers to avoid ownership ambiguities.

Testing
 - Tests should mock these interfaces when verifying controller logic. Integration tests should register the real implementations with a test composition root.
