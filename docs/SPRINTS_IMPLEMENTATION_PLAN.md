# AstraNotes: Informal Sprint Plan

**Purpose**: Convert the user stories in `docs/User_stories.md` into a practical implementation order.

**Note**: This is an informal planning guide, not a fixed delivery contract. The sprint order is based on dependencies, risk, and the desire to get the core note lifecycle working first.

---

## Planning Rules

- Build the core note path before adding advanced features.
- Keep persistence and recovery in the earliest sprint so later features can rely on stable storage.
- Add security, plugin flexibility, and deletion workflows after the core model is proven.
- Treat performance, crash recovery, and error handling as cross-cutting concerns that appear in every sprint.

---

## Sprint 1: Core Note Lifecycle and Persistence Foundation

**Goal**: Get notes creating, loading, saving, and recovering reliably before adding advanced features.

**Primary user stories**:
- Story 1: Create and Edit Notes with Save Options
- Story 5: Persistent Storage of Notes
- Story 11: Crash Recovery with WAL and Durable Save
- Story 15: Graceful Shutdown with Unsaved-Edit Recovery
- Story 18: Consistent Error Handling and User Notification Contract

**What gets implemented**:
- Note model and basic edit/save flow
- SQLite repository layer with WAL mode
- Manual save and autosave behavior
- Graceful exit handling with Save, Discard, or Cancel
- Unexpected shutdown recovery path
- Centralized error handling contract

**Exit criteria**:
- A note can be created, edited, saved, reopened, and recovered after a crash.
- Graceful shutdown prompts work correctly.
- Persistence is transactional and stable enough for later features.

---

## Sprint 2: Search, Metadata, and Version History

**Goal**: Make notes easy to find and revert once the storage foundation is stable.

**Primary user stories**:
- Story 3: Search Within Current Open Note
- Story 4: Search Across All Saved Notes
- Story 7: Note Metadata and Versioning with Snapshots
- Story 10: In-Note Search with Match Count and Navigation
- Story 14: Automatic Snapshot Creation on Save

**What gets implemented**:
- In-note search with highlighting, match counts, and navigation
- Global search using indexed title/metadata fields
- Metadata fields such as created date and last modified date
- Snapshot creation on save and snapshot restore flow
- Cache invalidation after restore

**Exit criteria**:
- Users can search within a note and across notes.
- Metadata is stored and displayed correctly.
- Snapshots are created on save, limited to two per note, and restorable.

---

## Sprint 3: Security, Encryption, and Data Protection

**Goal**: Lock down private notes and database access before exposing more content types.

**Primary user stories**:
- Story 6: Encrypt Private Notes
- Story 12: Encryption with Argon2id Key Derivation and IV Management
- Story 16: Immutable Audit Log for Note Operations
- Story 17: Database Security and SQL Injection Prevention
- Story 18: Consistent Error Handling and User Notification Contract

**What gets implemented**:
- AES-256-GCM encryption for private notes
- Argon2id key derivation and IV storage
- Password-gated decryption on open
- Append-only audit logging
- Prepared statements and input validation hardening
- Secure, non-blocking error notifications

**Exit criteria**:
- Private notes encrypt and decrypt correctly.
- SQL injection protections are in place.
- Audit logging is visible in the architecture and working in code.

---

## Sprint 4: Plugin System and Alternate Note Formats

**Goal**: Make the application extensible after the core note and security flows are stable.

**Primary user stories**:
- Story 2: Support Different Note Formats via Plugins
- Story 9: Plugin Validation and Missing-Plugin Fallback

**What gets implemented**:
- Plugin discovery and load lifecycle
- `IPlugin` contract and `PluginManager`
- Format routing through `typeId`
- Graceful fallback for missing or invalid plugins
- Voice note support as a concrete non-text plugin example

**Exit criteria**:
- Plugins load dynamically and are validated.
- Notes can resolve to the correct plugin by type.
- Missing plugins do not crash the app and leave notes read-only when needed.

---

## Sprint 5: Trash, Retention, and Deletion Workflow

**Goal**: Finish the note lifecycle with safe deletion and retention rules.

**Primary user stories**:
- Story 8: Delete One or More Notes
- Story 13: Trash with 14-Day Retention and Automatic Purge

**What gets implemented**:
- Single and bulk delete actions
- Trash state tracking in SQLite
- Trash view and restore flow
- 14-day retention rule
- Scheduled purge behavior

**Exit criteria**:
- Deleted notes move to Trash instead of disappearing immediately.
- Notes can be restored during the retention window.
- Purge runs automatically and transactionally.

---

## Sprint 6: Performance, Integration, and Release Hardening

**Goal**: Close out the remaining quality gates and stabilize the full system.

**Primary user stories / requirements**:
- NFR1: Performance targets
- NFR3: Crash recovery and graceful shutdown behavior
- Cross-cutting testing from all user stories

**What gets implemented**:
- Performance tuning for save, load, search, and pagination
- End-to-end integration tests across persistence, security, plugins, and trash
- Recovery testing for crash and power-loss scenarios
- Final validation of error handling and cache invalidation

**Exit criteria**:
- Performance targets are met or have documented exceptions.
- Crash/recovery and shutdown behavior are verified.
- The app is stable enough for broader implementation work.

---

## Suggested Implementation Order Within the First Sprint

1. Note model and repository interface
2. SQLite persistence with WAL
3. Manual save and autosave
4. Crash recovery and graceful shutdown
5. Error handling and logging

That sequence gives the team a stable base before adding search, encryption, plugins, or deletion workflows.

---

## Summary

This sprint plan follows the dependency chain from the user stories:
- first make notes persist safely,
- then make them searchable and versioned,
- then secure them,
- then extend them with plugins,
- then finish with trash/retention,
- and finally harden performance and recovery.

---

## Phase 5: UI Search, Shutdown, and Recovery

**Goal**: Finish the user-facing editing workflow and recoverability features that depend on the stable note, repository, and plugin foundations.

**Primary user stories**:
- Story 3: Search Within Current Open Note
- Story 10: In-Note Search with Match Count and Navigation
- Story 15: Graceful Shutdown with Unsaved-Edit Recovery

**What gets implemented**:
- Real-time in-note search with highlighting, match counts, and next/previous navigation
- Search UI state for no matches, wraparound navigation, and keyboard-friendly controls
- Unsaved-change detection on exit with Save All, Discard All, and Cancel
- Recovery prompt and state restoration on restart after a crash or power loss

**Exit criteria**:
- Users can search within the current note and navigate matches reliably.
- The app prompts before exit when there are unsaved changes.
- Restart recovery behavior is documented and exercised in execution evidence.

---

## Phase 6: Global Search, Metadata, and Version History

**Goal**: Add repository-backed discovery and note history features once the core UI flow is stable.

**Primary user stories**:
- Story 4: Search Across All Saved Notes
- Story 7: Note Metadata and Versioning with Snapshots
- Story 14: Automatic Snapshot Creation on Save

**What gets implemented**:
- Global search over titles, metadata, and indexed fields
- Metadata display for created and last-modified timestamps
- Snapshot creation on save and snapshot limit enforcement
- Snapshot restore with cache invalidation and safe fallback behavior

**Exit criteria**:
- Users can search across saved notes without loading full note bodies into memory.
- Metadata and snapshot history are persisted and retrievable.
- Snapshot rotation stays capped at two entries per note.

---

## Phase 7: Security, Encryption, and Auditability

**Goal**: Protect private note content and make sensitive operations traceable.

**Primary user stories**:
- Story 6: Encrypt Private Notes
- Story 12: Encryption with Argon2id Key Derivation and IV Management
- Story 16: Immutable Audit Log for Note Operations
- Story 17: Database Security and SQL Injection Prevention

**What gets implemented**:
- Password-gated AES-256-GCM encryption for private notes
- Argon2id key derivation and IV handling
- Immutable append-only audit logging for note and encryption operations
- Input validation and prepared-statement hardening for all database access

**Exit criteria**:
- Private notes encrypt and decrypt correctly with the expected password flow.
- Audit logging records the security-sensitive operations.
- Database operations remain parameterized and resistant to injection.

---

## Phase 8: Trash, Retention, and Release Hardening

**Goal**: Complete the note lifecycle and finish the project with verification, retention, and release-quality checks.

**Primary user stories / requirements**:
- Story 8: Delete One or More Notes
- Story 13: Trash with 14-Day Retention and Automatic Purge
- NFR1: Performance targets
- NFR3: Crash recovery via SQLite WAL with graceful shutdown on normal exit

**What gets implemented**:
- Single and bulk delete workflows that move notes into Trash
- Restore and scheduled purge behavior for the retention window
- Performance validation for save, load, and search flows
- Final crash-recovery and shutdown verification across the full stack

**Exit criteria**:
- Deleted notes move to Trash instead of being removed immediately.
- Purge and restore are transactional and repeatable.
- Performance and recovery behavior are documented in execution evidence.

---

## Recommended Build Order for the Next Work Chunk

1. Implement Phase 5 search and shutdown behavior.
2. Add the tracker doc so completed work is visible immediately.
3. Use the tracker to mark completed stories as code lands.
4. Move into Phase 6 global search and snapshots.
5. Finish with Phase 7 security and Phase 8 trash/hardening.
