# AstraNotes: User Stories and Acceptance Criteria

**Version**: 1.0
**Date**: April 8, 2026
**Status**: Active Requirements

---

## Overview

This document contains 8 user stories derived from the initial requirement set, working agreements, and definition of done for AstraNotes. Each story includes acceptance criteria that align with the definition of done (e.g., security considerations, phase mapping, modularity) and working agreements (e.g., AI-assisted development with CoPilot and Gemini review).

**Alignment with Architecture Phases**: Stories are mapped to relevant phases from `ARCHITECTURE.md` to ensure logical progression and avoid project drift. Requirements are distributed across phases to maintain modularity and incremental development.

**Key Mappings**:
- **Phase 1 (Foundation)**: Basic setup and core structures.
- **Phase 2 (Persistence)**: Database, storage, and versioning.
- **Phase 3 (Services)**: Business logic, encryption, and search.
- **Phase 4 (Plugin System)**: Format support via plugins.
- **Phase 5 (UI & Integration)**: User interface and editing features.
- **Phase 6 (Platform Integration)**: Performance and security optimizations.

---

## User Stories and Acceptance Criteria

### Story 1: Create and Edit Notes with Save Options
**As a** user, **I want** to create and edit notes with manual save and auto-save features, including title and body, **so that** I can efficiently manage my notes without losing work.

**Mapped to Phase**: Phase 2 (Persistance) & Phase 5 (UI & Integration) - Core editing functionality.
**Acceptance Criteria**:
- Notes can be created with a title (max 500 chars, no null bytes) and body (max 10 MB).
- Manual save and auto-save options are available, with auto-save implemented as a debounced 500ms `QTimer` that resets on each keystroke.
- All persistent saves write note records through SQLite via `QSqlDatabase`/`QSqlQuery`, using transactions for atomic operations.
- Errors during save are logged and reported to the user (no crashes).
- Feature is modular, allowing plugins to extend save behavior.
- Security: No sensitive data exposure during save operations.
- Performance: Save operations complete in <50ms for typical notes.
- Tested via unit tests for save logic and integration tests for UI behavior.

### Story 2: Support Different Note Formats via Plugins
**As a** user, **I want** to create notes in different formats based on installed plugins, **so that** I can use specialized note types like markdown or voice notes.

**Mapped to Phase**: Phase 4 (Plugin System) - Plugin-based format support.
**Acceptance Criteria**:
- Plugins are discovered dynamically via `QPluginLoader` from a `plugins/` directory.
- Supported formats include plain text, markdown, and voice (via TextPlugin and VoicePlugin).
- Each plugin implements `IPlugin` interface with format-specific processing.
- Notes store format metadata and are serialized/deserialized by plugins, so the core app can persist and restore plugin-specific note content.
- Errors in plugin loading are logged and handled gracefully (no crashes).
- Feature is modular, allowing new plugins without recompiling the core app.
- Security: Plugin operations do not compromise data integrity.
- Performance: Plugin loading completes in <500ms.
- Tested via unit tests for plugin interfaces and integration tests for format handling.

### Story 3: Search Within Current Open Note
**As a** user, **I want** to search for content within the note I currently have open, **so that** I can quickly find specific information in long notes.

**Mapped to Phase**: Phase 5 (UI & Integration) - UI search functionality.
**Acceptance Criteria**:
- Search input field highlights matching text in the note body.
- Search is case-insensitive and supports basic text matching.
- Search results are displayed in real-time as the user types.
- No performance impact on note editing or saving.
- Feature integrates seamlessly with the MVC UI layer.
- Security: Search does not expose encrypted content inappropriately.
- Modularity: Search logic can be extended by plugins.
- Tested via UI tests for search behavior and integration tests for performance.

### Story 4: Search Across All Saved Notes
**As a** user, **I want** to search for header data or titles across all my saved notes, **so that** I can find relevant notes quickly from my collection.

**Mapped to Phase**: Phase 3 (Services) - SearchService implementation.
**Acceptance Criteria**:
- Search uses SQLite indexes on note titles, header metadata, and tags rather than loading full note bodies into memory.
- Search supports partial matches and returns results with relevance ranking based on title and metadata.
- Results display note titles, snippets of header metadata, and other metadata (creation date, etc.).
- Search completes in <200ms for 10,000 notes.
- Errors are logged and reported (no crashes).
- Feature is modular, allowing plugins to extend search capabilities.
- Security: Search respects encryption (private notes not searchable in plain text).
- Tested via unit tests for SearchService and integration tests for database queries.

### Story 5: Persistent Storage of Notes
**As a** user, **I want** my notes to be persistently stored so they are not lost, **so that** I can access them across application sessions.

**Mapped to Phase**: Phase 2 (Persistence) - SQLite repository implementation.
**Acceptance Criteria**:
- Notes are stored in SQLite with WAL mode for concurrent access.
- Database schema includes tables for notes, metadata, and tags.
- Prepared statements prevent SQL injection.
- Crash recovery ensures notes are not lost on unexpected shutdowns.
- Feature supports at least 10,000 notes with <100ms listing time.
- Modularity: Repository interface allows different storage backends.
- Security: Database files are stored in user-writable locations via `QStandardPaths`.
- Tested via integration tests for CRUD operations and performance benchmarks.

### Story 6: Encrypt Private Notes
**As a** user, **I want** to encrypt notes marked as private using AES-256-GCM, **so that** my sensitive information remains secure.

**Mapped to Phase**: Phase 3 (Services) - EncryptionService implementation.
**Acceptance Criteria**:
- Private notes are encrypted with AES-256-GCM using Argon2id key derivation.
- Encryption/decryption happens transparently during save/load.
- Users are required to enter their password before a private note is decrypted and displayed.
- Key management uses password-based derivation (no separate key service).
- Errors in encryption are logged and reported (no crashes).
- Feature is modular, integrated into NoteService.
- Security: Immutable audit logs track encryption operations.
- Performance: Encryption completes in <100ms per note.
- Tested via unit tests for encryption roundtrips and integration tests for secure notes.

### Story 7: Note Metadata and Versioning with Snapshots
**As a** user, **I want** notes to include metadata (creation date, last modified) and versioning with snapshots, **so that** I can track changes and revert to previous versions.

**Mapped to Phase**: Phase 2 (Persistence) - Version history implementation.
**Acceptance Criteria**:
- Each note includes metadata: creation date, last modified date, tags.
- Versioning stores up to two snapshots per note in `version_history` table.
- Snapshots are created on save or edit; oldest deleted when limit exceeded.
- Users can revert to snapshots, invalidating the cache.
- Errors in versioning are logged and reported (no crashes).
- Feature is modular, part of repository layer.
- Security: Snapshots respect encryption for private notes.
- Performance: Version operations do not impact normal note performance.
- Tested via integration tests for snapshot creation/retrieval and cache invalidation.

### Story 8: Delete One or More Notes
**As a** user, **I want** to delete one or more notes at a time, **so that** I can manage my note collection efficiently.

**Mapped to Phase**: Phase 3 (Services) - NoteService deletion logic.
**Acceptance Criteria**:
- Single or bulk deletion supported via NoteService.
- Deletions are confirmed and reversible via versioning (if snapshots exist).
- Snapshots for deleted notes are retained only for a limited lifetime (for example, 14 days) before being purged.
- Database operations use transactions for reliability.
- Errors are logged and reported (no crashes).
- Feature is modular, integrated with UI for bulk operations.
- Security: Deletions are audited in immutable logs.
- Performance: Bulk deletions complete efficiently for large selections.
- Tested via unit tests for deletion logic and integration tests for UI workflows.

---

## Summary

These 8 user stories cover all functional requirements while incorporating non-functional (performance, scalability), security (encryption, audit logs), and governance (error handling, modularity) aspects. Each story maps to a specific architecture phase, ensuring incremental, modular development aligned with the working agreements (AI-assisted with CoPilot/Gemini review) and definition of done (security, phase mapping, ease of integration).

**Next Steps**: Prioritize stories by phase and begin implementation in Phase 1, using the working agreements for AI collaboration and documentation.