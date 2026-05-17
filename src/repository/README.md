Repository layer

Files:
- `INoteRepository.h`: Abstract persistence contract used by services. Defines CRUD, search, snapshot and trash operations (DB-agnostic).
- `SqliteNoteRepository.h` / `SqliteNoteRepository.cpp`: SQLite implementation of `INoteRepository`. Handles schema initialization, encryption hooks (via `EncryptionService`), snapshots storage/pruning, and trashed-state management.

Notes:
- All SQL logic and transaction handling must remain inside `SqliteNoteRepository`.
- For testing, the repository can be instantiated with the special path `":memory:"` to use an in-memory SQLite DB.
