Repository layer

Purpose
 - Encapsulate all persistence concerns: schema, transactions, queries, snapshot storage and trash/purge operations. Repositories expose a DB-agnostic interface (`INoteRepository`) consumed by services.

Files and responsibilities
 - `INoteRepository.h`: Abstract persistence contract used by services. Defines CRUD, search, snapshot and trash operations. The interface documents transaction semantics and expected invariants (e.g., save returns the canonical `Note` with updated timestamps).
 - `SqliteNoteRepository.h` / `SqliteNoteRepository.cpp`: SQLite implementation of `INoteRepository`. Responsibilities include schema initialization and migrations, parameterized queries, encryption hooks (via `EncryptionService`), snapshot storage and pruning, and trashed-state management.

Design and interaction
 - All SQL logic and transaction handling must remain inside `SqliteNoteRepository`. Services call the repository methods and expect that operations are atomic where documented.
 - The repository is the single source of truth for note content. When a service needs to create a snapshot or change trashed state it calls the repository which enforces referential invariants and updates timestamps.

Concurrency and testing
 - `SqliteNoteRepository` should use transactions for multi-step updates (save + snapshot) to keep state consistent under concurrent access.
 - For tests, instantiate with the special path `":memory:"` to use an in-memory SQLite DB and to avoid file I/O. Integration tests can optionally use temporary file-backed DBs to assert persistence across process restarts.

Schema and migrations
 - The repository maintains schema versioning and performs on-startup migration steps when necessary. Keep migration logic simple and additive; always retain backward compatibility where possible.

Cross references
 - See `src/service/README.md` for how services use repositories and `src/crypto/README.md` for details on how encrypted payloads are produced/consumed.
