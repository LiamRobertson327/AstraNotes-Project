# AstraNotes Class Responsibilities and Plugin Integration Guide

## Why this guide exists
The requirements are dense and each one spans multiple behaviors. This guide maps the key classes to concrete responsibilities and answers how plugin integration and encryption should work before implementation starts.

## Core class responsibilities

### `Note`
- Holds domain data: `id`, `title`, `content` or payload reference, timestamps, `typeId`, and state flags.
- `typeId` identifies which plugin owns serialization and format-specific behavior.
- Does not perform persistence, plugin loading, or encryption orchestration.

### `NoteService`
- Main application orchestrator for create/open/save/delete/search workflows.
- Validates inputs (where integrated per architecture).
- Resolves plugin from `typeId` through `PluginManager`.
- Calls repository methods for transactional persistence.
- Calls `EncryptionService` when note type/policy requires encryption.
- Owns workflow decisions such as manual save, autosave, restore, and trash operations.

### `INoteRepository`
- Interface for note persistence operations (`save`, `getById`, `search`, snapshot and trash operations, etc.).
- Defines transaction boundaries and persistence contract independent of SQLite.
- Should be the source of truth for database write/read semantics.

### `SQLiteNoteRepository`
- Concrete `INoteRepository` implementation.
- Uses SQLite transactions, WAL mode, prepared statements, indexes/FTS.
- Persists note records, snapshots, trash flags/retention metadata.
- Should not contain UI logic or plugin discovery logic.

### `PluginManager`
- Discovers and loads plugins from configured directory at startup and refresh.
- Keeps a registry of enabled plugins keyed by plugin id and supported note type ids.
- Resolves plugin ownership by `typeId`.

### `IPlugin`
- Contract for plugin metadata and format conversion behavior.
- Should expose at least: format identity, serialize, deserialize, capability metadata.
- Can include optional hooks, but core open/save orchestration should remain in `NoteService`.

### `EncryptionService`
- Performs AES-256-GCM encryption/decryption and Argon2id key derivation.
- Generates IV per encryption and packages crypto metadata with ciphertext payload.
- Returns explicit success/error results for `NoteService` to handle.

### `CacheManager`
- LRU cache for recently accessed notes.
- Reduces repeated DB reads and improves load performance.
- Must be invalidated on updates, restores, and trash/restore transitions.

### `NoteSnapshot`
- Immutable version payload captured on snapshot events.
- Linked to `noteId`, timestamped, and managed by snapshot retention rules.

## How plugins integrate with `Note`

## 1) Ownership model
- A `Note` is bound to a plugin by `typeId`.
- `typeId` maps to one plugin that owns format-specific serialization/deserialization.

## 2) Save flow (recommended)
1. `NoteService.saveNote(note)` receives domain object.
2. `NoteService` resolves plugin by `note.typeId`.
3. If secure/private policy applies, `EncryptionService` encrypts payload first.
4. Plugin serializes note payload to repository format.
5. `SQLiteNoteRepository.save(...)` commits transaction.

## 3) Open flow (recommended)
1. `NoteService.openNote(id)` reads persisted row via repository.
2. `NoteService` resolves plugin from stored `typeId`.
3. Plugin deserializes payload into domain object.
4. If payload is encrypted, `NoteService` requests password and calls `EncryptionService.decrypt(...)`.
5. Returned note is cached and sent to UI.

## Voice plugin integration
A voice plugin should not replace the core note lifecycle. It should implement format-specific handlers while `NoteService` remains the orchestrator.

### Voice plugin should provide
- Serializer/deserializer for audio payload metadata and storage references.
- Optional capabilities such as waveform metadata, duration, transcription fields.
- Validation for voice-specific constraints.

### Voice plugin should not own
- Database transactions.
- Global save/open control flow.
- Security policy decisions (except declaring secure capability support).

## Where encryption should be called

### Recommended boundary
- Encryption/decryption is performed by `EncryptionService`, invoked by `NoteService`.
- Plugins should not implement cryptography directly unless there is a hard requirement for plugin-specific crypto.

### Why
- Centralizes cryptography policy and key-derivation correctness.
- Avoids inconsistent encryption logic across plugins.
- Keeps auditability and error handling consistent.

## Do plugins need `openNote()`?
Short answer: not required for every plugin.

Recommended interface pattern:
- Required plugin methods: `serialize(note)`, `deserialize(payload)`, capability metadata.
- Optional hooks: `onBeforeSave(note)`, `onAfterLoad(note)` for plugin-specific processing.

`NoteService` should keep a single `openNote()` workflow and delegate only plugin-specific conversion steps, selected by `typeId`.

## How the app decides which plugin function to call
- Store `typeId` with each note record.
- On open/save, `NoteService` resolves plugin by `typeId` through `PluginManager`.
- If plugin missing/fails:
  - return explicit error,
  - log and notify user,
  - keep note in unsupported/read-only state until compatible plugin is available.

## Suggested implementation checklist
- Define/confirm `IPlugin` required methods: identity + serialize/deserialize.
- Define optional plugin hooks (if needed) without moving orchestration out of `NoteService`.
- Keep encryption in `EncryptionService`; call from `NoteService` before/after plugin conversion as required by format pipeline.
- Persist `typeId`, encryption metadata, and trash/snapshot metadata in repository schema.
- Add explicit sequence diagrams for:
  - save with plugin + encryption,
  - open with plugin + password-gated decryption,
  - missing-plugin fallback path.

## Design decision summary
- `NoteService` orchestrates.
- `IPlugin` converts format payloads.
- `EncryptionService` handles cryptography.
- `SQLiteNoteRepository` persists transactionally.
- `typeId` is the routing key between note and plugin behavior.
