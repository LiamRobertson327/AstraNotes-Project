## **Functional Requirements**
- The system must allow the user to create and edit notes with a manual save and auto-save feature, with all notes containing a title and body.
- The system must allow the user to create notes in different formats according to which plugins are currently installed that support those special formats.
- The system must allow the user to search the content in the note they currently have open.
- The system must allow the user to search the content or title for all the notes they have saved.
- The system must implement persistent storage so the user’s notes are not lost.
- The system must offer encryption of notes that the user marked as private.
- The system must attach metadata to each note, such as creation date and last modified date.
- The system must implement versioning, so the user can undo changes they made and optionally change a note to match a previously saved version if they want to revert multiple changes they made.  This system shall create a snapshot of a note when it is saved or edited to implement this.
- The system must allow the user to delete one or more notes at a time.

## **Non-Functional Requirements**
- The system must achieve fast performance, regardless of how many notes the user has.
- The system must be scalable and support at least 10,000 notes per user.
- The system must be reliable by implementing crash recovery and graceful shutdowns, so user notes are not lost.

## **Security, Privacy, Reliability, and Governance Requirements**
- Notes that the user marked as private shall be encrypted with AES-256-GCM.
- The system shall have immutable audit logs to track system operations and ensure compliance.
- The database shall implement methods to prevent SQL injection attacks.
- Errors when loading, saving, or creating notes shall be logged and reported to the user instead of the application crashing.

## **Functional Requirements**
- Original: The system must allow the user to create and edit notes with a manual save and auto-save feature, with all notes containing a title and body.
- Revised: The system shall allow users to create and edit notes identified by a unique ID, with optional UTF-8 title (0-500 characters, configurable) and optional UTF-8 body (0-10 MB, configurable); provide manual save via a Save button and optional auto-save via a 500 ms debounced QTimer that resets on each keystroke, show a persistent unsaved-changes indicator when auto-save is disabled and edits exist, persist all note/plugin formats to SQLite using transactional writes, and on save failure log the error, notify the user non-blockingly, preserve in-memory edits, and continue running without crashing

- Original: The system must allow the user to create notes in different formats according to which plugins are currently installed that support those special formats.
- Revised: The system shall allow users to create notes in any format provided by currently loaded and validated plugins. At application startup (and on explicit plugin refresh), the system discovers plugins from the configured plugin directory using the plugin interface contract, and each plugin must declare a unique format identifier, display name, and serialize/deserialize handlers. The note creation UI shall list only enabled formats from successfully loaded plugins. When saving or loading a note, the system shall invoke the owning plugin for format-specific serialization/deserialization and persist the result to SQLite. If a plugin is missing or fails, the system shall not crash, shall log the error, shall notify the user, and shall keep the note as unsupported/read-only until a compatible plugin is available

- Original: The system must allow the user to search the content in the note they currently have open.
- Revised: The system shall provide in-note search for the currently open note body (and title, optional), performing case-insensitive substring matching by default. Results shall update in real time as the user types, highlight all matches, display total match count and current match index, and support next/previous navigation. The search operation shall complete within 50 ms for notes up to 10 MB on supported target hardware. If the query is empty, all highlights are cleared; if no matches are found, the UI displays “No matches.” The feature shall not block editing or cause application crashes.

- Original: The system must allow the user to search the content or title for all the notes they have saved.
- Revised: The system shall allow users to search across all saved notes by title only, without loading full note bodies into memory. Search shall operate on indexed title fields in SQLite, return matching notes with their titles and metadata, and complete within the defined performance target for large note sets.

- Original: The system must implement persistent storage so the user’s notes are not lost.
- Revised: The system shall store all notes durably in SQLite using write-ahead logging (WAL) mode, ensuring that on-disk writes are flushed before the save operation completes to the user. On application crash or unexpected shutdown, the database shall recover uncommitted transactions from the journal and restore a consistent state on next startup. No note data created or edited in the session shall be lost due to application or system failure.

- Original: The system must offer encryption of notes that the user marked as private.
- Revised: The system shall encrypt all notes marked as private using AES-256-GCM with Argon2id key derivation. Encryption shall occur transparently during save; decryption shall occur transparently during load, but only after the user enters their password. If encryption or decryption fails, the system shall log the error, notify the user with a non-blocking message, preserve the note in its encrypted state, and continue running without crashing.
- Original: The system must attach metadata to each note, such as creation date and last modified date.
- Revised: The system shall attach the following metadata to each note: creation date (ISO 8601 timestamp, set at note creation and never modified), last modified date (ISO 8601 timestamp, updated on each save), and format identifier (string indicating the plugin format). Metadata shall be stored in the database alongside the note record and displayed in the UI when requested.

- Original: The system must implement versioning, so the user can undo changes they made and optionally change a note to match a previously saved version if they want to revert multiple changes they made.  This system shall create a snapshot of a note when it is saved or edited to implement this.
- Revised: The system shall automatically create a snapshot of every note on save (not on every edit keystroke). The system shall retain a maximum of two snapshots per note; when a third snapshot would be created, the oldest snapshot is deleted. Users can view the snapshot list, revert to any snapshot (which creates a new snapshot of the current state before reverting), and permanently delete snapshots. Snapshots respect encryption; private-note snapshots remain encrypted until the user decrypts the current note.

- Original: The system must allow the user to delete one or more notes at a time.
- Revised: The system shall support single and bulk note deletion by moving deleted notes to Trash, where they are hidden from active note views and viewable only in the Trash view. Deleted notes (including associated snapshots) shall be retained for 14 days, after which they are automatically and permanently deleted by a scheduled purge task. Users may restore notes from Trash during the 14-day period. All delete, restore, and purge operations shall be transactional; on failure, no partial state shall be committed.

## **Non-Functional Requirements**
- Original: The system must achieve fast performance, regardless of how many notes the user has.
- Revised: The system shall meet the following performance targets: note save operations complete in <50 ms, note load operations complete in <100 ms, search (by title) across all notes completes in <200 ms for a 10,000-note collection, and the note list displays with pagination loading in <500 ms for the first page. These targets assume typical single-user workloads on hardware meeting the project's specified minimum requirements.

- Original: The system must be scalable and support at least 10,000 notes per user.
- Revised: The system shall support at least 10,000 notes per user while meeting defined performance targets. Global search shall execute in SQLite using indexed fields (for example, title and metadata, with optional FTS) without loading all note bodies into application memory. Full note content shall be loaded on demand only when a note is opened.

- Original: The system must be reliable by implementing crash recovery and graceful shutdowns, so user notes are not lost.
- Revised: The system shall implement crash recovery by using SQLite WAL mode; on restart, any incomplete transactions are rolled back and the database is recovered to a consistent state. The system shall support graceful shutdown by completing in-flight save operations, flushing open transactions, and closing the database cleanly before exit. Unsaved edits in the current session that have not been auto-saved shall be preserved in an in-memory edit state; on restart, the user is prompted to recover unsaved changes or discard them.

## **Security, Privacy, Reliability, and Governance Requirements**
- Original: Notes that the user marked as private shall be encrypted with AES-256-GCM.
- Revised: All notes marked as private shall be encrypted using AES-256-GCM with a 256-bit key derived from the user's password using Argon2id (parameters: time cost 2, memory cost 65536 KiB, parallelism 1). The initialization vector (IV) shall be randomly generated per encryption and stored alongside the ciphertext. Decryption shall succeed only if the user provides the correct password.

- Original: The system shall have immutable audit logs to track system operations and ensure compliance.
- Revised: The system shall maintain an append-only immutable audit log recording all create, update, delete, and encryption operations on notes, along with timestamps and operation details. Log entries cannot be modified or deleted once written. Logs shall be stored in a dedicated database table or file and retained for the application lifetime. Access to logs is restricted to admin/diagnostic contexts. Log content shall be redacted to exclude sensitive data (note body content, decrypted private data, passwords).

- Original: The database shall implement methods to prevent SQL injection attacks.
- Revised: All database queries shall use prepared statements (parameterized queries) with bound parameters via QSqlQuery, never dynamic string concatenation or string templates. Input validation shall reject null bytes in string fields. The database shall be opened with restricted permissions and stored in a user-writable, application-owned directory.

- Original: Errors when loading, saving, or creating notes shall be logged and reported to the user instead of the application crashing.
- Revised: On any error during note load, save, create, delete, or encryption operations, the system shall: (1) log the error with full stack trace to the application log file, (2) display a non-blocking user notification describing the error in user-friendly language (without exposing raw exception text), (3) preserve the current application state and allow the user to retry or continue work. The application shall not crash, hang, or corrupt the database.

## **Governance and Ethics Risk Ratings (Current Assessment)**

**Rating Scale**: High = likely and high-impact if unaddressed; Medium = meaningful risk needing targeted controls; Low = mostly covered with minor residual risk.

- Risk: Sensitive data exposure in logs
	- Rating: High
	- Gap: Logging and audit requirements exist, but strict redaction boundaries can still be inconsistently implemented.
	- Mitigation Priority: Add a mandatory redaction policy and log schema tests.

- Risk: PII handling and classification gaps
	- Rating: High
	- Gap: Requirements protect private notes, but do not formally classify data categories and handling rules across all features.
	- Mitigation Priority: Add data classification tiers (public/internal/sensitive) and handling obligations per tier.

- Risk: Retention and purge inconsistency (notes, snapshots, backups)
	- Rating: Medium-High
	- Gap: Trash retention is defined, but backup retention and full deletion semantics are not fully specified.
	- Mitigation Priority: Define end-to-end deletion policy including backup lifecycle.

- Risk: Access control and least-privilege enforcement
	- Rating: Medium
	- Gap: Requirements focus on local storage and reliability, but explicit least-privilege constraints are limited.
	- Mitigation Priority: Add requirements for restricted filesystem/database permissions and privileged operation boundaries.

- Risk: Auditability vs privacy over-collection
	- Rating: Medium
	- Gap: Immutable logs improve governance, but can create privacy risk if operation payloads are over-collected.
	- Mitigation Priority: Restrict audit entries to metadata-only events and enforce retention limits.

- Risk: Encryption key lifecycle ambiguity
	- Rating: Medium
	- Gap: Algorithm and KDF are specified, but key rotation/reset behavior and secure handling lifecycle need more detail.
	- Mitigation Priority: Add key lifecycle requirements (derive, store, rotate, recover, fail-safe behavior).

- Risk: Concurrency and silent overwrite
	- Rating: Medium
	- Gap: Multi-window same-note editing conflict handling remains under-specified.
	- Mitigation Priority: Add conflict detection and user-visible resolution flow requirements.

- Risk: Performance fairness at scale
	- Rating: Low-Medium
	- Gap: Performance targets are present but benchmark assumptions can still vary by hardware/workload.
	- Mitigation Priority: Pin baseline hardware and dataset profile for performance verification.

- Risk: AI-use and external data leakage (future scope)
	- Rating: Low (current scope), High (if AI is introduced without controls)
	- Gap: AI integration is out of current implementation scope, so governance controls are not yet specified.
	- Mitigation Priority: Add placeholder AI guardrails for opt-in consent, PII redaction, data sharing limits, and provider retention constraints.