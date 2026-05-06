# AstraNotes: Requirement Traceability Matrix

**Date**: April 27, 2026  
**Purpose**: Validate alignment between INITIAL_REQUIREMENTS.md (Revised specifications) and UML diagram evidence  
**Scope**: 9 key requirements (6 functional, 2 non-functional, 1 security)  
**UML Sources**: Class Diagram, Object Diagram, Use Case Diagram, Activity Diagram, Deployment Diagram  
**Note**: All requirements use the **REVISED** detailed specifications from INITIAL_REQUIREMENTS.md, not the original summary statements.

---

## Traceability Matrix Summary

| Req ID | Requirement | Structural Evidence (Class & Object) | Behavioral Evidence (Use Case & Activity) | Deployment Evidence | Status | Gap Note |
|--------|-------------|--------------------------------------|-------------------------------------------|---------------------|--------|----------|
| **FR1** | Create/Edit notes + manual save + 500ms auto-save | Class: `Note`, `NoteService`; Object: `note_1042` instance with unsaved state | Use case: Create Note, Edit Note, Manual Save, Auto Save (500ms). Activity: Editor action note shows separate flows for manual save and autosave using the debounced timer | UI editor -> NoteController -> NoteService -> SQLiteNoteRepository (transactional, WAL); error logging | **Partially Traced** | UTF-8 and explicit size constraint are not shown in the class diagram. Logging and non-blocking error paths are not shown in the activity diagram. |
| **FR2** | Plugin-based note formats (load/validate plugins, select enabled formats, plugin-owned save/load serialization, graceful missing-plugin fallback) | Class: `PluginManager`, `IPlugin`, `TextPlugin`, `SecurePlugin`, `Note.typeId`; Object: `pluginMap` for plugin resolution | Use case: Create Note -> Choose Note Type. Activity: startup plugin lifecycle + user format selection from loaded plugins | Plugin runtime + plugin directory loaded at startup; UI format selector bound to enabled plugin types | **Partially Traced** | Diagrams show plugin loading/selection, but do not explicitly show validation flow, explicit plugin refresh, save/load ownership for serialize/deserialize, or missing-plugin behavior (log + notify + read-only unsupported state). |
| **FR5** | Durable SQLite persistence with WAL, save completion after durable write, crash recovery from journal, and no session data loss on app/system failure | Class: `INoteRepository`, `SQLiteNoteRepository` with transactional save/get operations; Object: repository persists `note_1042` and snapshots | Use case: Create/Open/Save flows imply persistence. Activity: save commits, restore flow, graceful shutdown path | SQLite DB node + repository persistence layer; notes/snapshots tables shown | **Partially Traced** | Persistence architecture is shown, but WAL mode is not explicitly depicted in the deployment diagram; "flush before save completion" is not explicitly modeled; crash-recovery from journal and "no data loss" guarantee are not explicitly shown as UML behavior/constraints. |
| **FR6** | Encrypt private notes with AES-256-GCM + Argon2id; transparent encrypt on save, decrypt on load after password prompt; error handling logs, notifies, preserves encrypted state | Class: `EncryptionService` (encrypt, decrypt, deriveKey), `SecurePlugin`, `Note.typeId` field; Object: `encryptionService` transforms `note_1042` to secure.note state | Use case: Encrypt or Decrypt Note. Activity: `NoteService` delegates to `EncryptionService`; password prompt on open for encrypted notes; type state updated after encryption | EncryptionService in security layer; encrypted content persisted to SQLite; key derivation at service layer | **Partially Traced** | Error handling (log + notify + preserve encrypted state + continue running) is not explicitly shown in activity diagram; Argon2id parameters (time cost 2, memory 65536 KiB, parallelism 1) and IV generation/storage not depicted in UML; "transparent" encrypt-on-save and decrypt-on-load flows are implied but not explicitly labeled. |
| **FR8** | Auto-snapshot on save (max 2 per note, FIFO purge oldest); view/revert/delete snapshots; revert creates safety snapshot first; snapshots respect encryption | Class: `NoteSnapshot` (snapshotId, noteId, serializedPayload), `INoteRepository.createSnapshot()`, `SQLiteNoteRepository`; Object: snapshot_9001, snapshot_9002 linked to `note_1042` with max 2 enforced | Use case: Review Version History, Restore Previous Version shown. Activity: Revert path shows creating safety snapshot before applying selected; restore flow explicit | Repository layer manages snapshot lifecycle and storage in SQLite; snapshots persisted locally and accessible on demand | **Partially Traced** | Activity diagram shows snapshot creation only during the revert/restore flow (safety snapshot before applying), but does not explicitly show automatic snapshot creation on every manual/auto-save operation or at note close; "on save" trigger points are ambiguous; max 2 constraint and FIFO purge are implicit in repository design, not explicitly diagrammed. |
| **FR9** | Single/bulk deletion to Trash with 14-day retention, auto-purge, restore capability, transactional operations | Class: `NoteService.deleteToTrash()`, `NoteService.restore()` methods; repository delete/trash operations | Use case: Delete Note to Trash, Restore Note from Trash. Activity: Delete branch -> Mark note as trashed (hidden) -> Persist state -> Return to list | Trash flag persisted in SQLite; scheduled purge task runs cleanup after 14 days; UI layer shows separate Trash view; all operations transactional | **Partially Traced** | Activity diagram shows delete/restore/purge flows and 14-day retention, but does not explicitly distinguish single vs. bulk deletion operations; auto-purge scheduled task is shown but implementation details (timing, transaction handling) are not modeled; transactional guarantees for delete/restore/purge are implicit in repository design, not explicitly diagrammed. |
| **NFR1** | Meet performance targets: save <50ms, load <100ms, search (10K notes) <200ms, pagination <500ms | Class: `CacheManager` (LRU cache); `INoteRepository.search()` with FTS5 indexing; Object: `cacheManager` instance for hot notes | Use case: Search All Notes by Title expected to complete quickly. Activity: Search execution shown; cache hit/miss paths implicit through service layer | CacheManager in service layer; SQLite FTS5 indexes on title/metadata; lazy loading of content on demand | **Fully Traced** | Architecture supports performance targets through caching, indexing, and lazy loading; performance is a verification/testing concern, not a structural design artifact. |
| **NFR3** | Crash recovery via SQLite WAL with graceful shutdown on normal exit | Class: transactional repository saves; explicit service error handling semantics; Object: None | Use case/activity: graceful shutdown is explicit (on exit: Save, Discard, or Cancel). WAL-based consistency recovery is implied at startup | SQLite with WAL; transactional persistence and clean shutdown responsibilities in service/repository layers | **Weakly Traced** | UML explicitly models normal user-initiated exit handling, but does not explicitly model restart recovery behavior for abnormal termination cases. Gap: requirement expects recovery flow after random application crash and after system power loss (restore consistent DB state and prompt to recover/discard unsaved edits), and these two abnormal-restart paths are not explicitly shown in the activity/use-case diagrams. |
| **SR1** | Private-note encryption with AES-256-GCM, 256-bit key from Argon2id (t=2, m=65536 KiB, p=1), random IV per encryption stored with ciphertext, and decryption only with correct password | Class: `EncryptionService` (encrypt/decrypt/deriveKey), `SecurePlugin`, `Note` secure type state; Object: encryption flow shown for secure note instances | Use case: Encrypt or Decrypt Note; Open Note prompts for password before decrypt. Activity: encrypt/decrypt branch with password-gated decryption and secure-state transitions | EncryptionService in security layer; encrypted payload persisted in SQLite; key derivation handled in service layer | **Partially Traced** | UML supports the high-level encryption/decryption flow and password gate, but does not explicitly encode Argon2id parameter values (t=2, m=65536 KiB, p=1), IV generation/storage semantics, or an explicit “decryption succeeds only with correct password” constraint annotation. |

---

## Summary Statistics

| Category | Count |
|----------|-------|
| Fully Traced | 1 of 9 (11%) |
| Partially Traced | 7 of 9 (78%) |
| Weakly Traced | 1 of 9 (11%) |
| Not Traced | 0 of 9 (0%) |

**Fully Traced Requirements**:
- ✅ NFR1: Meet performance targets

**Partially Traced Requirements**:
- ⚠️ FR1: Create/Edit notes + manual save + 500ms auto-save
- ⚠️ FR2: Plugin-based note formats
- ⚠️ FR5: Durable SQLite persistence with WAL
- ⚠️ FR6: Encrypt private notes with AES-256-GCM + Argon2id
- ⚠️ FR8: Auto-snapshot on save
- ⚠️ FR9: Single/bulk deletion to Trash
- ⚠️ SR1: Private-note encryption with Argon2id

**Weakly Traced Requirements**:
- ⚠️ NFR3: Crash recovery via SQLite WAL with graceful shutdown on normal exit

---

## Gap Analysis and Recommendations

### NFR1 - Performance Gap
**Current State**:
- Architecture includes LRU cache manager and FTS5 indexing
- Lazy loading of note content on demand

**Missing from UML**:
- Benchmark test scenarios

**Recommendation**:
The architecture already shows the mechanisms needed to meet the performance targets, but the targets themselves should still be verified through integration testing and benchmarking. Document baseline hardware and test results separately.

### NFR3 - Reliability Gap
**Current State**:
- Graceful shutdown flow with unsaved-changes handling is explicit
- Transactional saves with error handling
- SQLite WAL mode for crash recovery

**Missing from UML**:
- Explicit crash-recovery scenario diagram
- WAL mode recovery flowchart
- Abnormal termination / power-loss recovery path

**Recommendation**:
SQLite WAL mode is a persistence implementation detail (confirmed in ARCHITECTURE.md). The activity diagram should be extended to show abnormal restart recovery after crashes and power loss so the requirement is fully represented.

---

## Conclusion

✅ **All 9 requirements are traced to UML diagram evidence.**

- **1 requirement (11%)** has full explicit traceability across class, object, use case, activity, and/or deployment diagrams
- **7 requirements (78%)** are partially traced because they rely on architecture implementation choices or omitted edge cases that are present in diagrams but not fully explicit in UML notation
- **1 requirement (11%)** is weakly traced because the normal-flow UML exists, but the abnormal-restart recovery behavior is not explicit
- **0 requirements** are untraced

**Overall Verdict**: Requirements alignment is **adequate**, but the matrix now shows more of the real edge-case gaps. The UML diagrams provide strong coverage for the main flows, while several refined requirements still need explicit edge-case modeling or test-based validation.

---

## Appendix: Requirement ID Assignment Mapping

These requirements should be labeled in INITIAL_REQUIREMENTS.md as follows:

| ID | Category | Requirement |
|----|----------|-------------|
| FR1 | Functional | Create/Edit notes + manual save + 500ms auto-save |
| FR2 | Functional | Plugin-based note formats |
| FR5 | Functional | Durable SQLite persistence with WAL |
| FR6 | Functional | Encrypt private notes with AES-256-GCM + Argon2id |
| FR8 | Functional | Auto-snapshot on save |
| FR9 | Functional | Single/bulk deletion to Trash |
| NFR1 | Non-Functional | Meet performance targets |
| NFR3 | Non-Functional | Crash recovery via SQLite WAL with graceful shutdown on normal exit |
| SR1 | Security/Governance | Private-note encryption with Argon2id |

---

**Document Control**

| Version | Date | Purpose |
|---------|------|---------|
| 1.0 | 2026-04-27 | Initial traceability matrix for 9 key requirements extracted from INITIAL_REQUIREMENTS.md |
