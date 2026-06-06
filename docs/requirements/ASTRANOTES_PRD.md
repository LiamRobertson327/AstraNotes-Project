# AstraNotes Product Requirements Document (PRD)

**Product**: AstraNotes
**Version**: 2.0
**Date**: 2026-06-04
**Status**: Final

---

## 1. Product Summary

AstraNotes is a cross-platform C++23/Qt6 note-taking application designed for reliable local note storage, fast search, secure private-note encryption, extensible note formats, and safe recovery from crashes or unexpected shutdowns.

The product is intended to support a structured, traceable development process from requirements to release gates.

---

## 2. Problem Statement

Users need a desktop note-taking system that is:
- reliable enough to avoid data loss,
- fast enough to handle large note collections,
- secure enough for private notes,
- flexible enough to support multiple note formats,
- and traceable enough to support disciplined implementation and testing.

Many note apps cover one or two of these areas, but AstraNotes is designed to address all of them in one coherent local-first system.

---

## 3. Product Goals

### Primary goals
- Create, edit, save, and reopen notes reliably.
- Protect private notes with AES-256-GCM encryption.
- Support plugin-based note formats.
- Provide fast search and acceptable performance at 10,000+ notes.
- Preserve user work across saves, crashes, and shutdowns.

### Secondary goals
- Support note versioning and restore workflows.
- Provide a Trash workflow with recovery and retention.
- Keep the application architecture modular and easy to extend.
- Maintain strong traceability from requirements to release evidence.

---

## 4. Target Users

### Primary user
A student, researcher, or knowledge worker who needs a desktop note application for personal productivity and expects local reliability and privacy.

### Secondary user
A developer or maintainer who needs a clean architecture, explicit traceability, and testable behavior.

---

## 5. In Scope

The following capabilities are in scope for AstraNotes:
- Create and edit notes with manual save and autosave.
- Persistent SQLite storage with WAL mode.
- Search within a note and across all notes.
- Note metadata and versioning with snapshots.
- Private-note encryption with AES-256-GCM and Argon2id.
- Plugin-based note formats, including fallback handling.
- Trash workflow with retention and restore.
- Graceful shutdown and crash recovery.
- Audit logging and database hardening.
- Performance verification and release gate checks.

---

## 6. Out of Scope

The following are not currently primary product goals:
- Cloud synchronization
- Multi-user collaboration
- Real-time shared editing
- Mobile applications
- Browser-based editing
- Export/sync services not handled through plugins

---

## 7. Product Principles

- **Local-first**: User data is stored locally and recovered safely.
- **Traceable**: Every implementation step should map back to a requirement and user story.
- **Modular**: Core services stay separate and plugins extend behavior without destabilizing the app.
- **Secure**: Private notes and database access are protected by explicit policies.
- **Performant**: Search and browsing should remain responsive at scale.

---

## 8. Core Capabilities

### 8.1 Note Lifecycle
- Create notes.
- Edit notes.
- Save manually.
- Autosave after a short debounce interval.
- Recover unsaved work when appropriate.

### 8.2 Search
- Search inside the current note.
- Search across all saved notes by indexed fields.
- Keep search responsive for large note sets.

### 8.3 Versioning
- Create snapshots on save.
- Limit each note to two snapshots.
- Allow restore to previous versions.

### 8.4 Security
- Encrypt private notes.
- Use Argon2ID + salt to derive a key from a password.
- Maintain audit logs and secure storage practices.

### 8.5 Extensibility
- PluginManager singleton will dynamically load plugins at runtime to give user access to various note formats (Currently implemented with cpp/h files, but plugins are planned to move to dll loading).
- Route note behavior using type identifiers.
- Support alternate formats such as voice notes.

### 8.6 Reliability
- Use SQLite WAL mode.
- Recover after crashes and power loss.
- Support graceful shutdown without losing committed work.

---

## 9. Success Metrics

AstraNotes will be considered successful when the following are true:
- Save operations complete within 5-20ms.
- Note load operations complete within O(n) complexity with runtime of < 100ms for 10,000 notes | n = number of notes.
- Search across 10,000 notes remains responsive.
- Private-note encryption and decryption work correctly.
- Crash recovery preserves data consistency.
- Plugin loading and fallback behavior work without crashes.
- User acceptance sign-off is possible for each user story.

---

## 10. Requirements Reference

The authoritative detailed requirements live in:
- `docs/requirements/INITIAL_REQUIREMENTS_REVISED.md`

Supporting planning and traceability documents include:
- `docs/validation/REQUIREMENT_TRACEABILITY_MATRIX.md`
- `docs/requirements/User_stories.md`
- `docs/planning/BACKLOG.md`
- `docs/planning/SPRINTS_IMPLEMENTATION_PLAN.md`
- `docs/validation/TEST_PLAN.md`
- `docs/planning/RELEASE_GATES.md`
- `docs/requirements/CUSTOMER_ACCEPTANCE_AND_DOD.md`
- `docs/validation/TRACEABILITY_CHAIN.md`
- `docs/planning/DEVELOPMENT_CHECKPOINT.md`

---

## 11. Risk Areas

- Requirements are dense and combine multiple behaviors in a single statement.
- Crash recovery and shutdown behavior need careful implementation and testing.
- Plugin behavior must remain decoupled from core lifecycle logic.
- Performance targets must be validated with benchmarks, not assumptions.
- Security behavior must avoid exposing sensitive data in logs or UI messages.

---

## 12. Release Approach

The project should be released in stages:
1. Persistence foundation
2. Search and versioning
3. Security and encryption
4. Plugin system
5. Trash and retention
6. Performance hardening and final release gates

---

## 13. Definition of Done for the Product

The product is ready when:
- the core requirements are implemented,
- tests pass and evidence is recorded,
- release gates are satisfied,
- and the application can be demonstrated without blocking defects in the release scope.

---

## 14. Notes

This PRD is intentionally concise and aligned to the existing planning documents. It should be refined as implementation decisions become final.
