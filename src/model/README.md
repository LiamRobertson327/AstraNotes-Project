Domain models

Purpose
 - Models are the canonical in-memory representation of domain concepts (notes, snapshots). They are simple, serializable data holders without embedded persistence or UI logic.

Files and responsibilities
 - `Note.h` / `Note.cpp`: Represents a note and its metadata: `id`, `title`, `typeId` (used to lookup the appropriate plugin), content (plain text or HTML depending on note type), `createdAt`/`modifiedAt` timestamps and optional encryption metadata (algorithm version, flag indicating encrypted payload). `Note` should be copyable and cheap to move.
 - `Snapshot.h` / `Snapshot.cpp`: Represents a point-in-time snapshot of a note's content and metadata used by the snapshot/versioning subsystem. Snapshots carry timestamp, source `noteId`, and optionally encryption metadata.

Design and interaction
 - Models are passed between layers: UI receives a `Note` from `INoteService::load()` and renders `Note.content` in an editor. Services orchestrate saves and snapshots using `Note` and `Snapshot` value objects.
 - `typeId` is the connection point between model and plugin architecture: it tells `PluginManager` which plugin can render or provide formatting actions for this note.

Ownership and lifetime
 - `Note` and `Snapshot` are value-like data models, not owning UI objects. They should be passed by reference or copied as needed.
 - Legacy repository APIs may still return raw `Note*` / `Snapshot*` pointers; callers own those objects and must delete them after use.
 - The RAII migration should eventually replace those legacy raw-pointer returns with smart pointers or other explicit ownership types.

Serialization and invariants
 - Models should not embed serialization logic tied to a specific DB (repository handles that). The repository is responsible for mapping `Note` fields to DB columns and handling encryption/decryption of content.

Testing
 - Tests should validate simple invariants (e.g., timestamp fields updated on save flows in integration tests) and that snapshots preserve content and metadata ordering.

Cross references
 - See `src/plugins/README.md` for `typeId` usage and `src/repository/README.md` for how models are mapped to storage.
