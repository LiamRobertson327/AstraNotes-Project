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

**Mapped to Phase**: Phase 2 (Persistence) & Phase 5 (UI & Integration) - Core editing functionality.
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

**Mapped to Phase**: Phase 3 (Services) - NoteService/repository-backed global search implementation.
**Acceptance Criteria**:
- Search uses SQLite indexes on note titles, header metadata, and tags rather than loading full note bodies into memory.
- Search supports partial matches and returns results with relevance ranking based on title and metadata.
- Results display note titles, snippets of header metadata, and other metadata (creation date, etc.).
- Search completes in <200ms for 10,000 notes.
- Errors are logged and reported (no crashes).
- Feature is modular, allowing plugins to extend search capabilities.
- Security: Search respects encryption (private notes not searchable in plain text).
- Tested via unit tests for global search logic and integration tests for database queries.

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
- The system shall allow for selecting one or more notes and deleting them to the trash using a soft delete mechanism.
- Deleted notes shall remain in the trash until automatically deleted by the system according to the retention policy or manually deleted by the user.
- Trashed notes shall not show up on the side bar that display the notes the user has.
- On failure, the system shall log and display a non-blocking warning to the user and the application continues to run.

### Story 9: Plugin Validation and Missing-Plugin Fallback
**As a** developer/system, **I want** plugins to be validated on load and missing plugins to be handled gracefully, **so that** an invalid or missing plugin does not crash the application or leave notes inaccessible.

**Mapped to Phase**: Phase 4 (Plugin System) - Plugin validation and error handling.
**Acceptance Criteria**:
- On plugin load, the PluginManager validates plugin metadata (format ID, name, serialization functions).
- Invalid plugins fail gracefully with a logged error and a user notification.
- If a note's plugin is missing or fails, the note is marked as "unsupported/read-only" until a compatible plugin becomes available.
- User is notified of missing plugins without blocking note access.
- Application does not crash if a plugin is missing or malformed.
- Feature is modular, integrated into plugin lifecycle management.
- Security: Plugin validation prevents execution of untrusted or malformed plugins.
- Performance: Validation completes during plugin discovery phase.
- Tested via unit tests for plugin validation and integration tests for missing-plugin scenarios.

### Story 10: In-Note Search with Match Count and Navigation
**As a** user, **I want** to search within a note and see the total match count, current match index, and navigate through matches with next/previous buttons, **so that** I can efficiently locate and review all instances of a search term.

**Mapped to Phase**: Phase 5 (UI & Integration) - Enhanced in-note search UI.
**Acceptance Criteria**:
- Search box displays the current match index and total match count (e.g., "3 of 15").
- Next and Previous navigation buttons highlight the next or previous match in sequence.
- Navigating past the last match wraps to the first match; navigating before the first wraps to the last.
- If no matches are found, the UI displays "No matches" and disables navigation buttons.
- Search is case-insensitive and updates results in real-time as user types.
- Performance: Search operations complete within 50ms for notes up to 10 MB.
- Feature integrates seamlessly with MVC UI layer.
- Tested via UI tests for search navigation and integration tests for match counting.

### Story 11: Crash Recovery with WAL and Durable Save
**As a** user, **I want** the application to recover my notes and preserve data consistency even if the application crashes unexpectedly, **so that** I never lose work due to application failure.

**Mapped to Phase**: Phase 2 (Persistence) - WAL mode and crash recovery.
**Acceptance Criteria**:
- SQLite database uses WAL mode to ensure write-ahead logging and crash recovery.
- On save, the application flushes pending writes to disk before confirming save completion to the user.
- On unexpected application crash or system power loss, the database automatically recovers uncommitted transactions from the WAL journal on next startup.
- No note data created or edited in the session is lost due to application or system failure.
- Recovery is automatic and transparent to the user; no manual recovery steps are required.
- Feature is modular, part of repository layer.
- Security: WAL mode ensures data consistency across power failures and crashes.
- Performance: Flush-before-confirm does not exceed <50ms per save operation.
- Tested via integration tests for crash scenarios and performance benchmarks.

### Story 12: Encryption with Argon2id Key Derivation and IV Management
**As a** developer, **I want** encryption to use Argon2id key derivation with specific parameters and securely generate and store initialization vectors (IVs), **so that** private notes are protected with industry-standard key derivation and each encryption is uniquely randomized.

**Mapped to Phase**: Phase 3 (Services) - EncryptionService implementation details.
**Acceptance Criteria**:
- Encryption uses AES-256-GCM with a 256-bit key derived via Argon2id with parameters: time cost 2, memory cost 65536 KiB, parallelism 1.
- A random IV is generated for each encryption operation and stored alongside the ciphertext.
- Decryption extracts the IV from storage and uses it to decrypt the payload; decryption succeeds only if the user provides the correct password.
- Key derivation and IV generation are handled exclusively by EncryptionService.
- Feature is modular and integrated into NoteService.
- Security: Key derivation parameters are correct for modern security standards; IVs are cryptographically random and unique per encryption.
- Performance: Encryption with key derivation completes in <100ms per note.
- Tested via unit tests for encryption roundtrips with IV validation and integration tests for password-gated decryption.

### Story 13: Trash with 14-Day Retention and Automatic Purge
**As a** user, **I want** deleted notes to be moved to a Trash view where they are hidden from active notes and automatically purged after 14 days, **so that** I can recover accidentally deleted notes while eventually freeing up storage.

**Mapped to Phase**: Phase 3 (Services) - Soft-delete and trash management.
**Acceptance Criteria**:
- Single or bulk deleted notes are moved to Trash (marked with is_trashed flag) and hidden from active note list.
- Trash view displays all trashed notes separately with their deletion timestamps.
- Users can restore notes from Trash during the 14-day retention period by selecting "Restore from Trash."
- After 14 days, trashed notes and their associated snapshots are automatically and permanently deleted by a scheduled purge task.
- All delete, restore, and purge operations are transactional; on failure, no partial state is committed.
- Errors are logged and reported (no crashes).
- Feature is modular, part of NoteService and repository layer.
- Security: Deletions are audited in immutable logs.
- Performance: Trash operations complete efficiently; purge task runs at scheduled intervals.
- Tested via integration tests for trash workflows, retention validation, and transaction rollback scenarios.

### Story 14: Automatic Snapshot Creation on Save
**As a** developer, **I want** snapshots to be created automatically when a note is saved (not on every keystroke), and old snapshots to be purged automatically when the limit is exceeded, **so that** the system efficiently manages version history without excessive storage.

**Mapped to Phase**: Phase 2 (Persistence) - Snapshot lifecycle management.
**Acceptance Criteria**:
- Snapshots are created automatically only on manual or auto-save operations, not on every keystroke.
- The system retains a maximum of two snapshots per note; when a third snapshot would be created, the oldest snapshot is deleted (FIFO).
- When a user reverts to a snapshot, a safety snapshot of the current note state is created first before applying the selected snapshot.
- Snapshot creation and purge operations are transactional.
- Cache is invalidated on snapshot restoration to prevent stale data.
- Feature is modular, part of repository layer.
- Security: Snapshots respect encryption; private-note snapshots remain encrypted until the note is decrypted.
- Performance: Snapshot operations do not impact normal note save/load performance.
- Tested via integration tests for snapshot lifecycle, cache invalidation, and FIFO purge validation.

### Story 15: Graceful Shutdown with Unsaved-Edit Recovery
**As a** user, **I want** the application to prompt me on exit if I have unsaved changes, and on restart after an unexpected shutdown, **so that** I can choose to save, discard, or cancel exit, and recover unsaved edits if the application crashes.

**Mapped to Phase**: Phase 5 (UI & Integration) - Graceful shutdown and restart recovery.
**Acceptance Criteria**:
- On user-initiated exit, if there are unsaved note edits, the application displays a dialog with options: Save All, Discard All, or Cancel Exit.
- On cancel, the application remains open and editing continues.
- On unexpected application crash or system power loss, the next startup detects the crash and preserves in-memory edit state.
- After restart, the user is presented with recovered unsaved edits and can choose to recover them or discard them.
- All pending transactions are flushed and the database is closed cleanly on graceful shutdown.
- Feature integrates into UI lifecycle and is modular with service layer.
- Security: No sensitive data is exposed during shutdown prompts or recovery.
- Performance: Shutdown and recovery operations complete quickly without blocking.
- Tested via integration tests for graceful shutdown flows and crash recovery scenarios.

### Story 16: Immutable Audit Log for Note Operations
**As a** developer/administrator, **I want** all note create, update, delete, and encryption operations to be recorded in an immutable append-only audit log with timestamps, **so that** all note operations are traceable and tamper-proof.

**Mapped to Phase**: Phase 3 (Services) & Phase 2 (Persistence) - Audit logging infrastructure.
**Acceptance Criteria**:
- An immutable audit log records all note create, update, delete, and encryption operations with timestamps and operation details.
- Log entries cannot be modified or deleted once written.
- Logs are stored in a dedicated database table and retained for the application lifetime.
- Access to logs is restricted to admin/diagnostic contexts and is not exposed in standard user interfaces.
- Log content is redacted to exclude sensitive data: note body content, decrypted private data, passwords.
- Feature is modular, part of service and repository layers.
- Security: Audit logs are immutable and tamper-proof; sensitive data is redacted.
- Performance: Audit log writes do not block note operations.
- Tested via integration tests for log creation, immutability validation, and redaction verification.

### Story 17: Database Security and SQL Injection Prevention
**As a** developer, **I want** all database queries to use prepared statements with parameterized queries and input validation to reject null bytes, **so that** the application is protected against SQL injection attacks.

**Mapped to Phase**: Phase 2 (Persistence) - Database security hardening.
**Acceptance Criteria**:
- All database queries use prepared statements (`QSqlQuery` with parameter binding) and never use dynamic string concatenation or string templates.
- Input validation rejects null bytes in string fields (title, body, tags).
- Database file is stored in user-writable, application-owned directories via `QStandardPaths`.
- Database is opened with restricted file permissions to prevent unauthorized access.
- Feature is modular, part of repository layer.
- Security: SQL injection attacks are prevented; database access is restricted to the application.
- Performance: Prepared statements maintain performance through query plan caching.
- Tested via unit tests for parameterized query construction and integration tests for injection attack scenarios.

### Story 18: Consistent Error Handling and User Notification Contract
**As a** developer, **I want** all errors during note operations (load, save, create, delete, encryption) to follow a consistent contract: log with full stack trace, notify the user non-blockingly, preserve application state, and allow retry, **so that** users are informed of errors and can recover without crashing.

**Mapped to Phase**: Phase 3 (Services) & Phase 5 (UI & Integration) - Error handling strategy.
**Acceptance Criteria**:
- On any error during note load, save, create, delete, or encryption, the system logs the error with full stack trace to the application log file.
- The application displays a non-blocking user notification describing the error in user-friendly language without exposing raw exception text.
- The current application state is preserved; the user can retry the operation or continue working without interruption.
- The application does not crash, hang, or corrupt the database on error.
- All service/repository operations return explicit error results via `std::expected<T, Error>`.
- Feature is modular, integrated across service and UI layers.
- Security: Error messages do not expose sensitive information; logs are stored securely.
- Performance: Error handling does not introduce performance penalties.
- Tested via unit tests for error propagation and integration tests for user-facing error scenarios.

---

These 8 user stories cover all functional requirements while incorporating non-functional (performance, scalability), security (encryption, audit logs), and governance (error handling, modularity) aspects. Each story maps to a specific architecture phase, ensuring incremental, modular development aligned with the working agreements (AI-assisted with CoPilot/Gemini review) and definition of done (security, phase mapping, ease of integration).

**Next Steps**: Prioritize stories by phase and begin implementation in Phase 1, using the working agreements for AI collaboration and documentation.