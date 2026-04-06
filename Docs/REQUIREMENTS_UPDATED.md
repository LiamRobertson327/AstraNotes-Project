# AstraNotes: Requirements Specification

**Project**: AstraNotes - High-Performance Note-Taking Application  
**Version**: 2.0 (Updated)  
**Date**: April 2, 2026  
**Status**: Initial Requirements Baseline  

---

## Executive Summary

This document specifies the initial functional, non-functional, and security/reliability/governance requirements for AstraNotes. The application is a local note-taking system that supports 10,000+ notes with versioning, encryption on demand, multiple format support via plugins, and fast searching. This is the baseline specification for Phase 1 MVP.

---

## 1. FUNCTIONAL REQUIREMENTS (9 total)

### FR1: Create and Edit Notes with Save Features
The system must allow the user to create and edit notes with a manual save and auto-save feature, with all notes containing a title and body.

### FR2: Multiple Note Formats via Plugins
The system must allow the user to create notes in different formats according to which plugins are currently installed that support those special formats.

### FR3: Search Within Current Note
The system must allow the user to search the content in the note they currently have open.

### FR4: Search All Notes
The system must allow the user to search the content or title for all the notes they have saved.

### FR5: Persistent Storage
The system must implement persistent storage so the user's notes are not lost.

### FR6: Encryption of Private Notes
The system must offer encryption of notes that the user marked as private.

### FR7: Note Metadata
The system must attach metadata to each note, such as creation date and last modified date.

### FR8: Versioning and Snapshots
The system must implement versioning, so the user can undo changes they made and optionally change a note to match a previously saved version if they want to revert multiple changes they made. This system shall create a snapshot of a note when it is saved or edited to implement this.

### FR9: Delete Notes
The system must allow the user to delete one or more notes at a time.

---

## 2. NON-FUNCTIONAL REQUIREMENTS (3 total)

### NFR1: Fast Performance
The system must achieve fast performance, regardless of how many notes the user has.

### NFR2: Scalability
The system must be scalable and support at least 10,000 notes per user.

### NFR3: Reliability and Recovery
The system must be reliable by implementing crash recovery and graceful shutdowns, so user notes are not lost.

---

## 3. SECURITY, PRIVACY, RELIABILITY, AND GOVERNANCE REQUIREMENTS (4 total)

### SR1: Private Note Encryption
Notes that the user marked as private shall be encrypted with AES-256-GCM.

### SR2: Immutable Audit Logs
The system shall have immutable audit logs to track system operations and ensure compliance.

### SR3: SQL Injection Prevention
The database shall implement methods to prevent SQL injection attacks.

### SR4: Error Logging and Reporting
Errors when loading, saving, or creating notes shall be logged and reported to the user instead of the application crashing.

---

## 4. REQUIREMENTS SUMMARY TABLE

| # | Requirement | Type | Category |
|---|-------------|------|----------|
| FR1 | Create and edit notes with manual/auto-save | Functional | Core |
| FR2 | Multiple formats via plugins | Functional | Core |
| FR3 | Search within current note | Functional | Core |
| FR4 | Search all notes (title/content) | Functional | Core |
| FR5 | Persistent storage | Functional | Core |
| FR6 | Encryption of private notes | Functional | Core |
| FR7 | Note metadata (created/modified dates) | Functional | Core |
| FR8 | Versioning with snapshots | Functional | Core |
| FR9 | Delete one or more notes | Functional | Core |
| NFR1 | Fast performance | Non-Functional | Quality |
| NFR2 | Scalability to 10K+ notes | Non-Functional | Quality |
| NFR3 | Reliability and crash recovery | Non-Functional | Quality |
| SR1 | AES-256-GCM encryption for private notes | Security | Safety |
| SR2 | Immutable audit logs | Security | Safety |
| SR3 | SQL injection prevention | Security | Safety |
| SR4 | Error logging and user reporting | Reliability | Safety |

---

## 5. TRACEABILITY NOTES

### Functional Requirements Map to Architecture Components

| Requirement | Architecture Component | Design Document |
|---|---|---|
| FR1 (Save/Auto-save) | NoteService, Timer | ARCHITECTURE.md (Service Layer) |
| FR2 (Plugins) | PluginManager, IPlugin interface | ARCHITECTURE.md (Plugin System) |
| FR3 (Search in note) | Editor search functionality | UI Layer |
| FR4 (Search all notes) | SearchService, full-text index | ARCHITECTURE.md (Service Layer) |
| FR5 (Persistent storage) | SQLiteNoteRepository, database schema | ARCHITECTURE.md (Repository Pattern) |
| FR6 (Encryption) | EncryptionService, SecurePlugin | ARCHITECTURE.md (Security) |
| FR7 (Metadata) | Note model, database schema | PROJECT_STRUCTURE.md (Model Layer) |
| FR8 (Versioning) | Version history table, snapshot mechanism | DATABASE_SCHEMA.md (to be created) |
| FR9 (Delete) | NoteService.delete(), cascade logic | ARCHITECTURE.md (Service Layer) |

---

## 6. PHASE 1 MVP SCOPE

**Included in MVP**:
- ✅ All 9 functional requirements
- ✅ All 3 non-functional requirements
- ✅ All 4 security/reliability requirements

**Out of Scope (Future)**:
- ❌ Cloud synchronization
- ❌ Shared notes / collaboration
- ❌ Mobile application
- ❌ Web interface
- ❌ Advanced note organization (folders/collections)
- ❌ Plugin marketplace
- ❌ Automatic updates

---

## 7. CONSTRAINTS & ASSUMPTIONS

### Technical Constraints
- Must compile with C++20 standard
- Must work on Windows, macOS, Linux (same codebase)
- SQLite as primary database (no external DB server)
- Qt 6 for GUI framework
- Offline-first (no required internet connection)

### Assumptions
- Users have adequate disk space (500MB minimum free)
- File system is POSIX-compliant or Windows NTFS
- Display resolution >= 1024x768

---

## 8. NEXT STEPS

To convert these requirements into actionable work items, the following artifacts need to be created:

1. **User Stories** - Convert each FR to user story format (As a user, I want...)
2. **Data Model Definition** - Document Note structure, Version structure, metadata fields
3. **Database Schema** - SQL DDL for tables and relationships
4. **API Specifications** - Method signatures for services and plugin interfaces
5. **Test Case Matrix** - Specific test cases for each requirement
6. **UI/UX Flows** - Wireframes and user interaction workflows

---

## Document Control

| Version | Date | Author | Change |
|---------|------|--------|--------|
| 1.0 | 2026-04-02 | System | Initial requirements baseline (extended) |
| 2.0 | 2026-04-02 | System | Updated with user-provided requirements (simplified, focused) |

---

**End of Requirements Specification**
