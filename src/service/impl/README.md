Service implementation details

Overview
 - The `impl/` folder contains concrete service implementations that orchestrate workflows across repositories, plugins and other services. These classes implement the stable interfaces from `src/service/interfaces` and encapsulate business rules.

Files and responsibilities
 - `NoteService.h` / `NoteService.cpp`: Implements high-level note lifecycle operations (create, save, load, search, trash). Responsibilities include validation, creating safety snapshots before risky operations, forwarding persistence calls to `INoteRepository`, and emitting audit/log events.
 - `SnapshotService.h` / `SnapshotService.cpp`: Manages snapshot creation, retrieval, deletion and revert workflow. Orchestrates retention pruning policies and coordinates with the repository for efficient snapshot storage.
 - `TrashService.h` / `TrashService.cpp`: Implements soft-delete (trash), restore, purge and retention/purge scheduling logic. Uses the repository to mark notes as trashed and to permanently delete notes when retention policies trigger a purge.

How implementations interact with other layers
 - `NoteService` calls `INoteRepository` for all DB operations and `EncryptionService` (in `src/crypto`) when saving/loading secured notes. It also calls `SnapshotService` to create snapshots during mutating operations.
 - Services must not perform SQL themselves — database access belongs exclusively to `src/repository` implementations.

Design & ownership guidelines
 - Keep services injectable via constructor injection. Do not instantiate concrete repositories inside service constructors — that responsibility belongs to the composition root.
 - Prefer returning copyable value objects or `std::shared_ptr<Model>` for domain objects to make ownership explicit.

Testing guidance
 - Unit-test `NoteService`/`SnapshotService`/`TrashService` by providing mocked `INoteRepository` implementations or by wiring a `SqliteNoteRepository` with `":memory:"` for deterministic integration-style tests.
 - Validate behavior for encryption edge-cases, snapshot ordering and retention policies using integration tests that write/read snapshots and assert expected ordering/pruning.
