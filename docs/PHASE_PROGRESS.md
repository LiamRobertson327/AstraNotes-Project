# AstraNotes Phase Progress Tracker

**Purpose:** Track completed phases, in-progress phases, and user story checkoffs in one place.

**Legend:**
- [x] Completed
- [ ] Not started
- [~] In progress

---

## Phase Summary

| Phase | Area | Status | Notes |
|---|---|---:|---|
| Phase 1 | Note-Type Awareness | [x] | Completed and integrated |
| Phase 2 | Plugin System | [x] | Completed and integrated |
| Phase 3 | Formatting Toolbar | [x] | Completed and integrated |
| Phase 4 | SQLite Persistence | [x] | Completed and integrated |
| Phase 5 | UI Search, Shutdown, and Recovery | [x] | Completed and integrated |
| Phase 6 | Global Search, Metadata, and Version History | [x] | Completed and integrated |
| Phase 7 | Security, Encryption, and Auditability | [x] | Completed and integrated |
| Phase 8 | Trash, Retention, and Release Hardening | [ ] | Planned next |
| Phase 9 | Strict MVC Refactor | [ ] | Planned after Phase 8 is implemented |

---

## User Story Checkoffs

### Completed Stories

- [x] Story 1: Create and Edit Notes with Save Options
- [x] Story 2: Support Different Note Formats via Plugins
- [x] Story 3: Search Within Current Open Note
- [x] Story 4: Search Across All Saved Notes
- [x] Story 5: Persistent Storage of Notes
- [x] Story 6: Encrypt Private Notes
- [x] Story 7: Note Metadata and Versioning with Snapshots
- [x] Story 9: Plugin Validation and Missing-Plugin Fallback
- [x] Story 10: In-Note Search with Match Count and Navigation
- [x] Story 11: Crash Recovery with WAL and Durable Save
- [x] Story 12: Encryption with Argon2id Key Derivation and IV Management
- [x] Story 14: Automatic Snapshot Creation on Save
- [x] Story 15: Graceful Shutdown with Unsaved-Edit Recovery
- [x] Story 17: Database Security and SQL Injection Prevention
- [x] Story 18: Consistent Error Handling and User Notification Contract

### In Progress / Planned Stories

- [ ] Story 8: Delete One or More Notes
- [ ] Story 13: Trash with 14-Day Retention and Automatic Purge
- [ ] Story 16: Immutable Audit Log for Note Operations

---

## Completion Notes

### Phase 1
- Note types now control editor modes.
- Plaintext notes are locked to write mode.
- Markdown notes enable write, read, and split modes.

### Phase 2
- PluginManager and plugin contracts are in place.
- Plaintext and markdown plugin support is wired.
- Plugin validation and fallback behavior are established.

### Phase 3
- Formatting toolbar actions are populated dynamically from plugin capabilities.
- Editor focus determines where formatting actions are applied.

### Phase 4
- SQLite persistence is wired to the UI.
- WAL mode and prepared statements are in place.
- Save status feedback is visible in the main window.

### Phase 5
- In-note search with real-time highlighting and match navigation implemented.
- Search expanded to title bar and saved-notes list filtering.
- Unsaved-change detection on title and content edits.
- Save/Discard/Cancel prompts on note switch, new-note creation, and application close.
- Auto-save with 500ms debounced QTimer (FR1) implemented.
- Auto-save toggle checkbox in header.
- Crash recovery via SQLite WAL mode validated.

### Phase 6
- **Metadata Display (FR7):** Created and modified timestamps stored in SQLite (UTC CURRENT_TIMESTAMP); displayed in local time via `.toLocalTime()` conversion; format ID shown in UI.
- **Snapshots (FR8 Core):** Automatic snapshot creation on manual save, note-switch, new-note creation, and app-close events; auto-save (500ms debounce) does NOT create snapshots to conserve resources.
- **Snapshot Limits & Enforcement:** Maximum 2 snapshots per note enforced via `enforceMaxSnapshotLimit()`; oldest snapshot auto-deleted (FIFO) when limit exceeded.
- **Snapshot UI:** History button in header; modal dialog with sortable list (newest first); Revert and Delete buttons; snapshot timestamps converted to local time for display.
- **Snapshot Revert Logic:** Revert creates safety snapshot of current state BEFORE restoring target content (robust two-phase approach prevents "snapshot not found" race condition); unsafe snapshots are captured if user changes mind.
- **Pagination (NFR1):** Lazy-loading sidebar with 50 notes per page; scroll-triggered LIMIT/OFFSET queries; paging methods in repository: `searchByTitlePaged(query, limit, offset)` and `countTitleMatches(query)`.
- **Full-Text Search (FTS5):** Virtual table `notes_fts` indexes title and content for sub-200ms global searches on 10k+ notes; `searchByTitle()` uses FTS5 MATCH with LIKE fallback for compatibility.
- **Repository Enhancements:** New methods `getSnapshotById(snapshotId)` for robust single-snapshot fetches (prevents rotation race condition); `saveSnapshot()`, `getSnapshotsByNoteId()`, `deleteSnapshotById()`, `deleteOldestSnapshotForNote()` for complete snapshot lifecycle.
- **Bug Fixes:** Fixed snapshot waste (auto-save now skips snapshot creation), timestamp offset (all displays use `.toLocalTime()`), revert "not found" error (fetch target before safety snapshot).

### Phase 7
- **Encryption Stability Fix (Architectural):** Secured notes now use an atomic save path so the note write and snapshot write do not encrypt the same content twice. The encrypted ciphertext, salt, IV, and tag from the primary note save are reused for history snapshots instead of generating a second ciphertext.
- **Auto-Save / Manual Save Race:** Auto-save is stopped at the start of `saveCurrentNote()` and the debounce window was increased to better absorb Argon2id work during manual saves, preventing overlapping writes while the password dialog is open.
- **Argon2id / OpenSSL Diagnostics:** `EncryptionService::deriveKey()` now keeps provider/module loading relative to the application binary and collects OpenSSL error strings so failures can be distinguished from incorrect passwords or missing modules.
- **Base64 / Tag Integrity:** Encryption and decryption paths now preserve Base64 payloads and validate the 128-bit authentication tag before decryption finalization.

- Phase 7 security work is complete and the encryption flow is now stable for secured note save/load and snapshot persistence.

### Phase 9
- Planned only after Phase 8 is completed and verified.
- Goal: move toward a stricter MVC split by extracting remaining note orchestration out of `MainWindow` and into controller/service boundaries.
- Current `MainWindow` responsibilities to evaluate for extraction include save/open flow, snapshot orchestration, and other non-visual note lifecycle logic.

#### Bug Fixes
- Fixed the double-encryption loop between `save()` and `createSnapshotForCurrentNote()` by reusing the note's encrypted ciphertext, salt, IV, and tag for snapshot persistence.
- Fixed the manual-save versus auto-save race by stopping the auto-save timer before starting a save and extending the debounce window for Argon2id work.
- Improved `deriveKey()` diagnostics so OpenSSL provider or KDF failures can be distinguished from wrong-password failures.
- Tightened Base64 and authentication-tag handling to avoid decryption failures caused by malformed or truncated encrypted payloads.

---

## Revised Functional Requirements (From INITIAL_REQUIREMENTS_REVISED.md)

### Functional Requirements (FR)

- [x] **FR1:** Create and edit notes with unique ID; manual save button; auto-save via 500ms debounced QTimer; unsaved-changes indicator; transactional SQLite writes; graceful error handling
- [x] **FR2:** Plugin-based note creation; plugin discovery at startup; format selection UI; plugin serialization/deserialization; missing-plugin fallback (Phases 1-2)
- [x] **FR3:** In-note search with case-insensitive matching; real-time highlighting; match count and navigation; title search; 50ms performance target
- [x] **FR4:** Global search across all notes by title; indexed SQLite fields; no full-body loading; performance target met
- [x] **FR5:** SQLite WAL mode for durable writes; crash recovery with journal rollback; consistent state on restart
- [x] **FR6:** Encrypt private notes with AES-256-GCM; transparent save/load encryption; password-based decryption; error handling
- [x] **FR7:** Note metadata (creation date ISO 8601, last modified date, format ID); database storage; UI display
- [~] **FR8:** Automatic snapshot creation on save; max 2 snapshots per note; snapshot list view; revert capability (creates safety snapshot); snapshot deletion; encryption respect (now reuses primary-note encryption metadata)
- [ ] **FR9:** Bulk note deletion to Trash; 14-day retention; auto-purge task; restore capability; transactional operations

### Non-Functional Requirements (NFR)

- [~] **NFR1:** Performance targets: save <50ms, load <100ms, title search <200ms for 10k notes, list pagination <500ms
  - [x] Save performance meets <50ms
  - [x] Load performance meets <100ms
  - [x] Title search optimization for 10k notes (FTS5 virtual table)
  - [x] Pagination implementation (lazy-load 50/page)
- [ ] **NFR2:** Support 10,000+ notes; indexed global search; on-demand full-body loading; no full-collection memory load
- [ ] **NFR3:** WAL mode crash recovery; graceful shutdown with in-flight save completion; unsaved-edit recovery prompt on restart

### Security, Privacy, Reliability, and Governance Requirements (SR)

- [~] **SR1:** Encrypt private notes with AES-256-GCM and Argon2id (time=2, memory=65536 KiB, parallelism=1); random IV per encryption; correct-password-only decryption
- [ ] **SR2:** Immutable append-only audit log; records create/update/delete/encryption ops; timestamps; redacted content (no body, no private data, no passwords); admin-only access
- [x] **SR3:** Prepared statements with QSqlQuery; no dynamic SQL concatenation; null-byte validation; restricted database permissions
- [ ] **SR4:** Error handling: log errors with stack trace; non-blocking user notifications; preserve app state; allow retry; no crashes/hangs/corruption

---

## How to Use This Tracker

1. Mark a story as completed only when code, validation, and evidence are all present.
1. Mark a requirement as completed only when code, validation, and evidence are all present.
2. Keep phase status aligned with the actual implementation state.
3. Use this file as the checklist for submission readiness and demo planning.
4. Link execution evidence and tests to the stories they validate.
4. Link execution evidence and tests to the requirements they validate.
