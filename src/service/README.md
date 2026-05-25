Service layer

Purpose: provide thin service facades over repository operations and coordinate cross-repository workflows.

Structure:

Guidelines
 - Keep business rules in services, persistence details in `repository/`.
 - Depend on abstractions across layers: UI and controllers use service interfaces, services use repository interfaces.
 - Restrict concrete wiring (`SqliteNoteRepository`, `NoteService`, etc.) to the composition root.
 - Prefer writing unit tests for `impl/` classes using an in-memory SQLite database (`":memory:"`).

Purpose
 - Provide business logic and coordinate workflows that span repositories, plugins and UI interactions. Services translate UI intents into persistence operations while enforcing rules (e.g., snapshot retention, encryption requirements, trash policies).

Structure
 - `interfaces/`: stable public interfaces consumed by the UI and controllers (`INoteService`, `ISnapshotService`, `ITrashService`). Interfaces are intentionally lightweight and stable so the UI need not change when implementations evolve.
 - `impl/`: concrete implementations that fulfill the interfaces. Implementations depend only on repository interfaces (`INoteRepository`) and other service interfaces to avoid tight coupling to concrete DB types.

How services fit in the design
 - Controllers (UI) call `INoteService` methods to perform operations like save/load/create/search/trash. `NoteService` implements higher-level flows (validate, create safety snapshots, call repository save, emit audit events).
 - `SnapshotService` owns snapshot lifecycle concerns and interacts with `INoteRepository` for snapshot storage; it enforces retention policies and triggers pruning via repository calls.
 - `TrashService` implements soft-delete semantics and schedules or triggers permanent purges using repository primitives.

Key responsibilities and invariants
 - Keep validation, retention and audit decisions inside services. Do not execute SQL here: services must call repository interfaces only.
 - Services should be side-effect minimal for read-only operations; mutating actions should be transactional at the repository layer.

Testing guidance
 - Unit test implementations in `impl/` by mocking `INoteRepository` (or instantiating `SqliteNoteRepository` against `":memory:"` for integration-style unit tests).
 - Exercise edge-cases: encryption-required flows (wrong password), concurrent snapshot creation, and recover-from-corruption workflows via tests that simulate repository failures.

Cross references
 - See `src/repository/README.md` for repository responsibilities and `tests/TESTS.md` for tests that exercise service-level behaviors.
