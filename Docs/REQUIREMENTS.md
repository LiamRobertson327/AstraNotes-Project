# AstraNotes: Requirements Specification

**Project**: AstraNotes - High-Performance Note-Taking Application  
**Version**: 1.0  
**Date**: April 2, 2026  
**Status**: Requirements Baseline  

---

## Executive Summary

This document specifies the initial functional, non-functional, and security/reliability/governance requirements for AstraNotes. The application is a local note-taking system that supports 10,000+ notes with versioning, encryption on demand, multiple format support via plugins, and fast searching. This is the baseline specification for Phase 1 MVP.

---

## 1. FUNCTIONAL REQUIREMENTS

### FR1: Note Creation and Storage

**Description**: The application shall allow users to create, edit, and store notes with persistent storage.

**Details**:
- Users can create new notes from the GUI (File → New Note or toolbar button)
- Each note must contain:
  - Unique ID (auto-generated)
  - Title (1-500 characters, required)
  - Content (up to 10 MB, required)
  - Content Type (Plain Text, Markdown, Rich Text)
  - Timestamps (created_at, updated_at, auto-managed)
  - Tags (0-unlimited, alphanumeric + spaces, max 50 chars each)
  - Encryption metadata (if encrypted)
- Notes must be immediately persisted to SQLite database
- Manual save and auto-save features (auto-save every 30 seconds if modified)
- Version history: snapshots created automatically on each save and edit operation
- Users can undo changes made within the current session
- Users can revert to any previously saved version
- Version list shows timestamp and change summary for each snapshot
- At least 20 undo/redo operations maintained in current session

**Acceptance Criteria**:
- ✅ Can create new note in < 50ms
- ✅ Note appears in list immediately after creation
- ✅ All metadata stored correctly in database
- ✅ Auto-save triggers without user intervention
- ✅ Snapshots created on every save/edit automatically
- ✅ User can view and revert to any previous version
- ✅ Undo/Redo work for title and content changes
- ✅ Version history persists after close/reopen

**Priority**: P0 - Critical  
**Effort**: 10 story points (increased from 8 due to versioning implementation)

---

### FR2: Note Retrieval, Update, and Deletion

**Description**: The application shall support full CRUD operations on existing notes, including single and bulk deletion.

**Details**:
- Users can retrieve a note by clicking in note list
- Retrieved note displays in editor with all metadata visible
- Users can modify title, content, tags, encryption status
- Modifications are saved with updated_at timestamp
- Users can delete notes (with confirmation dialog, undo available for 30 seconds)
- Soft delete support (flag as deleted, hard delete after 30 days)
- Bulk operations: delete one or more notes at once (multi-select and delete)

**Acceptance Criteria**:
- ✅ Open note in < 30ms
- ✅ Modifications persist correctly
- ✅ Delete confirmation prevents accidents
- ✅ Undo delete restores note completely
- ✅ Bulk delete works for 100+ notes

**Priority**: P0 - Critical  
**Effort**: 6 story points

---

### FR3: Note Search and Filtering

**Description**: The application shall provide full-text search and filtering capabilities for note discovery.

**Details**:
- Full-text search across note titles and content
- Search query returns ranked results (relevance-based, TF-IDF scoring)
- Filtering by:
  - Note type (Plain, Markdown, Rich Text, Voice)
  - Encryption status (encrypted/unencrypted)
  - Tags (single or multiple)
  - Date range (created/updated)
- Search results paginated (100 items per page)
- Search performance < 300ms for 10K notes
- Real-time search suggestions (as user types)
- Save search queries as smart collections

**Acceptance Criteria**:
- ✅ Search returns correct results in < 300ms
- ✅ Filters work independently and combined
- ✅ Search suggestions appear in < 200ms
- ✅ Can save and load search collections
- ✅ Pagination handles 10K+ results smoothly

**Priority**: P0 - Critical  
**Effort**: 8 story points

---

### FR4: Multiple Note Formats Support

**Description**: The application shall support multiple note content formats via the plugin system.

**Details**:
- **Plain Text Format**: Simple text, no formatting
  - Syntax highlighting for code blocks (optional)
- **Markdown Format**: 
  - Render markdown to HTML preview
  - Support headings, lists, links, images, code blocks
  - Live preview while editing
- **Rich Text Format**:
  - WYSIWYG editor with bold, italic, underline, colors
  - Support for embedded images
- **Voice Notes** (VoicePlugin):
  - Record audio directly or import WAV/MP3
  - Playback with basic controls (play, pause, seek)
  - Optional: Transcription to text (speech-to-text integration)
  - Duration metadata stored
- Format conversion: Convert between Plain ↔ Markdown ↔ RichText
  - Content preservation (best effort)
  - Metadata preserved always

**Acceptance Criteria**:
- ✅ Each format loads and displays correctly
- ✅ Markdown preview updates in real-time
- ✅ Voice notes record and play back
- ✅ Format conversion preserves most content
- ✅ Can switch format without data loss
- ✅ Plugin interface extensible for custom formats

**Priority**: P1 - Important  
**Effort**: 12 story points

---

### FR5: Secure (Encrypted) Notes

**Description**: The application shall support end-to-end encrypted notes using AES-256-GCM.

**Details**:
- Encrypt individual notes with user-provided password
- Encryption algorithm: AES-256-GCM (authenticated encryption)
- Key derivation: PBKDF2 with 100,000 iterations or Argon2id
- Random salt (16 bytes) generated per encrypted note
- Encrypted notes cannot be searched or indexed
- Display lock icon in note list for encrypted notes
- Decrypt on-demand when user opens encrypted note
- Optional auto-lock after 5 minutes of inactivity
- Support for access control lists (ACL) - optional future feature
- Cannot export encrypted notes in plaintext

**Acceptance Criteria**:
- ✅ Encrypt/decrypt cycle preserves content exactly
- ✅ Password validation > 8 characters, no storage
- ✅ Encryption uses authenticated mode (GCM)
- ✅ Encrypted notes show lock indicator
- ✅ Cannot access content without password
- ✅ Auto-lock works in background
- ✅ Cannot export as plaintext

**Priority**: P1 - Important  
**Effort**: 10 story points

---

### FR6: Plugin System and Extensibility

**Description**: The application shall provide a plugin architecture for content processing and format support.

**Details**:
- Plugin Interface (IPlugin):
  - getName(), getVersion(), getAuthor()
  - getSupportedNoteTypes(), getSupportedMimeTypes()
  - initialize(context), shutdown()
  - processNote(note) → std::expected<std::string, Error>
  - exportNote(note, path) → std::expected<void, Error>
  - importNote(path) → std::expected<Note, Error>
- Built-in plugins (3):
  - TextPlugin: Plain text + markdown support
  - VoicePlugin: Audio recording/playback
  - SecurePlugin: AES-256 encryption wrapper
- Plugin Manager:
  - Load plugins at startup (from /plugins directory)
  - List loaded plugins and capabilities
  - Execute plugin operations on demand
  - Error isolation (plugin crash doesn't crash app)
  - Plugin hot-reload support (future)
- Plugin Context provides access to:
  - Logger
  - Repository (note storage)
  - Configuration
  - Encryption service
- Custom plugins can be created by users/developers
- Plugin documentation and SDK provided

**Acceptance Criteria**:
- ✅ All 3 built-in plugins load and initialize
- ✅ Plugin interface is stable and documented
- ✅ Custom plugin creation is feasible
- ✅ Plugin context provides necessary services
- ✅ Plugin crash doesn't crash application
- ✅ Plugin error handling is graceful
- ✅ Can disable/enable plugins via settings

**Priority**: P1 - Important  
**Effort**: 10 story points

---

### FR7: Note Organization and Collections (Bonus)

**Description**: The application shall support organizing notes into collections and hierarchies.

**Details**:
- Folders/Collections for grouping notes
- Hierarchical structure (nested folders)
- Move notes between folders
- Drag-and-drop support
- Smart collections (saved searches)
- Pin favorite notes
- Color-coding for folders
- Note count per folder
- Recent notes list
- Favorites list

**Acceptance Criteria**:
- ✅ Can create folder hierarchy
- ✅ Drag-and-drop moves notes
- ✅ Smart collections reflect changes
- ✅ Pinned notes persist
- ✅ Color scheme applies correctly

**Priority**: P2 - Nice-to-have  
**Effort**: 6 story points

---

## 2. NON-FUNCTIONAL REQUIREMENTS

### NFR1: Performance

**Description**: The application shall meet strict performance requirements for responsive user experience with 10,000+ notes.

**Details**:
- **Response Times**:
  - Create note: < 50ms
  - Open note: < 30ms
  - Search 10K notes: < 300ms
  - List 1K notes: < 100ms (paginated)
  - Encrypt/decrypt 1MB: < 100ms
  - Plugin load: < 500ms per plugin
- **Throughput**:
  - Bulk create 1000 notes: < 30 seconds
  - Bulk search across 10K notes: < 500ms
- **UI Responsiveness**:
  - GUI remains responsive during searches (async operations)
  - No freezing during database operations
  - Maintain 60 FPS during scrolling/animation
- **Startup Time**:
  - Application launch: < 3 seconds
  - Load all plugins: < 2 seconds
  - Load database index: < 1 second
- **Memory Usage**:
  - 10K notes cached: < 200MB RAM
  - LRU cache evicts least-used notes
  - No memory leaks (verified with valgrind/Instruments)
- **Database** Performance:
  - Queries use indices
  - Prepared statements for all queries
  - Batch operations grouped in transactions
  - WAL mode for concurrent read access

**Acceptance Criteria**:
- ✅ All response times within targets (measured with profiler)
- ✅ No UI freezing during operations
- ✅ Memory stable over 1-hour usage session
- ✅ Database operations logged and profiled
- ✅ Can handle 10K+ notes seamlessly
- ✅ Search still responsive at high load

**Priority**: P0 - Critical  
**Effort**: 10 story points (performance engineering)

---

### NFR2: Scalability and Reliability

**Description**: The application shall scale to support 100,000+ notes and remain reliable under stress.

**Details**:
- **Scalability**:
  - Tested with 10K, 50K, 100K notes
  - Database indices optimize beyond 10K limit
  - Memory usage grows sub-linearly with note count
  - Cache strategy handles large datasets
  - No SQL queries without limits (pagination enforced)
- **Reliability**:
  - Application crash recovery: restore to pre-crash state
  - Database crash: automatic recovery on startup
  - Corrupted note: isolate from other notes
  - Transaction rollback on error
  - Backup creation before major operations
  - Graceful degradation (plugin failure doesn't break app)
- **Availability**:
  - 99% uptime target
  - 0-downtime updates for data files
  - Auto-repair of corrupted database
  - No single point of failure in architecture
- **Data Integrity**:
  - Foreign key constraints enforced
  - Audit log of all modifications
  - Checksums for data verification
  - Atomic operations (all-or-nothing)

**Acceptance Criteria**:
- ✅ Application handles 100K notes smoothly
- ✅ Crash recovery tested and verified
- ✅ Data integrity maintained after crash
- ✅ Stress tested with concurrent operations
- ✅ Graceful error handling, no data loss
- ✅ Audit log captures all changes

**Priority**: P0 - Critical  
**Effort**: 12 story points

---

## 3. SECURITY / PRIVACY / RELIABILITY / GOVERNANCE REQUIREMENTS

### SR1: Data Security and Encryption

**Description**: The application shall protect sensitive data through encryption and secure key management.

**Details**:
- **Encryption**:
  - Sensitive notes encrypted with AES-256-GCM
  - Key derivation using PBKDF2 (100K iterations) or Argon2id
  - 16-byte random salt per encrypted note
  - Authenticated encryption (cannot tamper with ciphertext)
  - Full-disk encryption support (leverage OS features)
- **Key Management**:
  - No plaintext storage of passwords
  - Password hashed with PBKDF2-SHA256
  - Master key derived from password + salt
  - No key material in memory longer than necessary
  - Secure key erasure (memset with random values)
  - Optional keychain/secure storage integration
- **Data at Rest**:
  - Database file encrypted (SQLite SEE or OS-level)
  - Config files do not contain secrets
  - Backup files encrypted matching original data
- **Data in Transit**:
  - SSL/TLS for future cloud sync features
  - No note data transmitted insecurely
- **Compliance**:
  - Follows OWASP encryption guidelines
  - No hardcoded secrets
  - Security audit log of encryption operations
  - GDPR-compliant data handling

**Acceptance Criteria**:
- ✅ Encrypted notes cannot be read without password
- ✅ No passwords stored in plaintext
- ✅ Encryption verified with penetration testing
- ✅ Key derivation uses strong parameters
- ✅ No timing side-channels in crypto code
- ✅ Backup files encrypted like original data
- ✅ Memory properly cleaned after use

**Priority**: P0 - Critical  
**Effort**: 12 story points

---

### SR2: Input Validation, Privacy, and Governance

**Description**: The application shall validate all inputs, respect user privacy, and meet governance/audit requirements.

**Details**:
- **Input Validation**:
  - Title: Max 500 chars, no null bytes, trimmed
  - Content: Max 10 MB (scalable), UTF-8 validated
  - Tags: Alphanumeric + spaces, max 50 chars each
  - Dates: RFC3339 format only
  - File paths: No directory traversal (/../../)
  - All user inputs sanitized before storage
  - SQL queries use prepared statements (no injection)
  - File uploads validated for size/type
- **Privacy**:
  - Local-only by default (no cloud sync without consent)
  - No telemetry or usage tracking
  - No analytics collection
  - No third-party cookies
  - Privacy policy (if distributed)
  - User can export all data in standard format
  - Delete all data feature (not just mark deleted)
  - Clear data on app uninstall (config folder cleanup)
- **Governance / Audit**:
  - Audit log table: who changed what, when
  - Audit includes note ID, action, timestamp, user
  - Audit log not editable by users
  - Version tracking for notes (future)
  - Change history accessible to user
  - Compliance with project requirements
  - Regular security reviews documented
  - Vulnerability disclosure policy

**Acceptance Criteria**:
- ✅ Invalid inputs rejected or sanitized
- ✅ No SQL injection possible
- ✅ File paths cannot escape intended directory
- ✅ No external network requests without consent
- ✅ User can delete all data permanently
- ✅ Audit log captures all changes
- ✅ Encryption keys never logged
- ✅ Privacy policy (if applicable) accurate

**Priority**: P0 - Critical  
**Effort**: 10 story points

---

### SR3: Error Handling, Resilience, and Disaster Recovery

**Description**: The application shall handle errors gracefully, recover from failures, and protect against data loss.

**Details**:
- **Error Handling**:
  - `std::expected<T, Error>` for all operations
  - No uncaught exceptions
  - Meaningful error messages (not technical jargon for users)
  - User-facing errors logged with context
  - Plugin errors isolated (don't crash app)
  - Network errors handled gracefully (future)
  - Out-of-memory conditions handled
- **Resilience**:
  - Database corruption detected and reported
  - Automatic repair of minor corruption
  - Partial note loss acceptable if database unrecoverable
  - Graceful degradation (continue with reduced features)
  - Retry logic for transient failures
  - Timeout handling for long operations
- **Disaster Recovery**:
  - Automatic daily backups (optional)
  - Manual backup export (File → Export Backup)
  - Restore from backup (File → Restore Backup)
  - Point-in-time recovery (note versioning)
  - Transaction logs for recovery
  - Backup verification before deletion
  - Document disaster recovery procedure
- **Testing**:
  - Chaos engineering: inject failures and verify recovery
  - Database corruption scenarios tested
  - Disk full scenarios tested
  - Memory pressure scenarios tested
  - Plugin crash isolation tested
  - Data consistency verified after crash

**Acceptance Criteria**:
- ✅ Application never crashes on bad input
- ✅ Database corruption detected early
- ✅ Auto-recovery restores most data
- ✅ Backup works correctly
- ✅ Restore from backup verified to work
- ✅ Plugin crashes don't crash app
- ✅ Out-of-memory handled gracefully

**Priority**: P0 - Critical  
**Effort**: 8 story points

---

## 4. FEATURE SUMMARY TABLE

| # | Requirement | Type | Priority | Epic | Story Points |
|---|-------------|------|----------|------|--------------|
| FR1 | Create/Edit/Store Notes with Versioning | Functional | P0 | Core | 10 |
| FR2 | CRUD Operations | Functional | P0 | Core | 6 |
| FR3 | Search and Filtering | Functional | P0 | Core | 8 |
| FR4 | Multiple Formats | Functional | P1 | Features | 12 |
| FR5 | Encryption (SecureNote) | Functional | P1 | Features | 10 |
| FR6 | Plugin System | Functional | P1 | Architecture | 10 |
| FR7 | Organization (Bonus) | Functional | P2 | Nice-to-have | 6 |
| **NFR1** | **Performance** | **Non-Functional** | **P0** | **Quality** | **10** |
| **NFR2** | **Scalability/Reliability** | **Non-Functional** | **P0** | **Quality** | **12** |
| **SR1** | **Security & Encryption** | **Security** | **P0** | **Safety** | **12** |
| **SR2** | **Input Validation & Privacy** | **Security** | **P0** | **Safety** | **10** |
| **SR3** | **Error Handling & Recovery** | **Reliability** | **P0** | **Safety** | **8** |

---

## 5. REQUIREMENTS TRACEABILITY MATRIX

### Core Epics vs Requirements

| Epic | FR1 | FR2 | FR3 | FR4 | FR5 | FR6 | NFR1 | NFR2 | SR1 | SR2 | SR3 |
|------|-----|-----|-----|-----|-----|-----|------|------|-----|-----|-----|
| **Core** | ✅ | ✅ | ✅ | - | - | - | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Features** | - | - | - | ✅ | ✅ | - | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Architecture** | - | - | - | - | - | ✅ | ✅ | ✅ | - | ✅ | ✅ |
| **Quality** | - | - | - | - | - | - | ✅ | ✅ | - | - | - |
| **Safety** | - | - | - | - | - | - | - | - | ✅ | ✅ | ✅ |

---

## 6. ACCEPTANCE CRITERIA SUMMARY

### Phase 1: MVP (Core Requirements Only)
- ✅ FR1: Create/edit notes
- ✅ FR2: CRUD operations  
- ✅ FR3: Search and filter
- ✅ NFR1: Performance targets met
- ✅ NFR2: Handles 10K notes reliably
- ✅ SR1: Encryption for sensitive notes
- ✅ SR2: Input validation complete
- ✅ SR3: Error handling and recovery

### Phase 2: Feature Release
- ✅ FR4: Multiple formats (Markdown, RichText, Voice)
- ✅ FR5: Enhanced encryption with ACL
- ✅ FR6: Plugin ecosystem
- ✅ Advanced search (saved collections)

### Phase 3: Polish
- ✅ FR7: Organization (folders, collections)
- ✅ Performance optimization (sub-100ms operations)
- ✅ Backup/restore workflow
- ✅ User documentation

---

## 7. CONSTRAINTS & ASSUMPTIONS

### Technical Constraints
- Must compile with C++23 standard
- Must work on Windows, macOS, Linux (same codebase)
- SQLite as primary database (no external DB server)
- Qt 6 for GUI framework
- No external cloud dependencies
- Offline-first (no required internet connection)

### Assumptions
- Users have administrator access to install application
- File system is POSIX-compliant or Windows NTFS
- Display resolution >= 1024x768
- At least 500MB free disk space
- Network optional (sync not in MVP)

### Out of Scope (Future)
- Cloud synchronization
- Shared notes / collaboration
- Mobile application
- Web interface
- Version control / full history
- Automatic updates
- Plugin marketplace

---

## 8. REQUIREMENT REVIEW SIGN-OFF

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Product Manager | [Your Name] | 2026-04-02 | _____ |
| Tech Lead | [Your Name] | 2026-04-02 | _____ |
| QA Lead | [Your Name] | 2026-04-02 | _____ |
| Security Review | [Your Name] | 2026-04-02 | _____ |

---

## Document Control

| Version | Date | Author | Change |
|---------|------|--------|--------|
| 1.0 | 2026-04-02 | System | Initial requirements baseline |

---

**End of Requirements Specification**
