# AstraNotes Class Responsibilities and Plugin Integration Guide

## Why this guide exists
This document maps the current architecture to concrete responsibilities so the project stays aligned with the interface-first design that is actually implemented in the codebase.

> **After Project Completion Note**: This document was created near the end of the project completion and is an accurate representation of the final project form for the files discussed.

## Current architecture at a glance

- `MainWindow` is the composition root. It creates the concrete repository and service implementations and wires them into the UI.
- UI classes depend on service interfaces, not concrete implementations.
- Services depend on `INoteRepository`, not on `SqliteNoteRepository` directly.
- `SqliteNoteRepository` owns SQLite-specific persistence details.
- `IPlugin` exists as a public extension contract, but the current codebase snapshot does not include a runtime plugin manager or plugin registry implementation.
- Audit logging, snapshots, trash, and encryption are handled by dedicated classes or services, not by the UI.

## Core class responsibilities

### `Note`
- Holds note domain data such as ID, title, content, type, timestamps, encryption state, and trash state.
- Does not talk to the database, UI, or filesystem on its own.

### `INoteRepository`
- Abstract persistence contract used by the service layer.
- Exposes CRUD, search, snapshot, trash, and storage-helper operations.
- Exists so services can depend on one abstraction regardless of the database backend.

### `SqliteNoteRepository`
- Concrete SQLite implementation of `INoteRepository`.
- Owns SQL statements, transaction handling, schema setup, snapshot storage, and trash persistence.
- May have SQLite-specific helpers internally, but callers should prefer the interface whenever possible.

### `INoteService`
- Defines the main note lifecycle contract used by the UI.
- Covers load, save, create, search, counting, and trash-related note operations.

### `ISnapshotService`
- Defines version-history operations such as save snapshot, list snapshots, delete snapshots, enforce limits, and revert flow.

### `ITrashService`
- Defines trash, restore, purge, and trash-reporting operations.

### `NoteService`
- Implements `INoteService` and coordinates note workflows.
- Uses `INoteRepository` for persistence.
- Handles note loading, saving, creation, searching, and trash handoff.
- Does not own direct UI logic.

### `SnapshotService`
- Implements `ISnapshotService`.
- Coordinates snapshot creation, retrieval, pruning, and revert behavior.
- Uses `INoteRepository` for snapshot persistence and retrieval.

### `TrashService`
- Implements `ITrashService`.
- Coordinates soft delete, restore, purge, and trash queries through `INoteRepository`.

### `MainWindow`
- Acts as the composition root for the desktop app.
- Creates `SqliteNoteRepository`, then passes it into `NoteService`, `SnapshotService`, and `TrashService`.
- Passes service interfaces into controllers and UI helpers.

### `NoteListController`
- UI controller for the notes list and pagination behavior.
- Depends on `INoteService`.

### `AuditLogPanel`
- Read-only UI for displaying append-only audit log entries.
- Does not generate audit events itself; it displays them.

### `AuditLogger`
- Central audit logging utility that writes the immutable log stream.
- Keeps the audit format consistent across actions.

### `EncryptionService`
- Handles encryption and decryption responsibilities.
- Keeps cryptographic details out of the repository and UI layers.

### `IPlugin`
- Public plugin contract for extension authors.
- Defines metadata and serialization/deserialization behavior.
- In the current snapshot, it is an interface contract rather than a fully wired runtime plugin system.

## Plugin integration in the current project

### What the codebase currently supports
- The project exposes `IPlugin` as an extension contract.
- The data model includes note type information that can be used for future routing.
- The architecture keeps room for a plugin layer without forcing the UI or repository to know plugin internals.

## Encryption boundary

- Encryption belongs in `EncryptionService`.
- The service layer is the correct place to invoke encryption or decryption.
- Repository code should store and retrieve encrypted payloads, but not decide cryptographic policy.

## Snapshot and trash flow

- Snapshot creation and revert logic belong in `SnapshotService`.
- Trash, restore, and purge logic belong in `TrashService`.
- `SqliteNoteRepository` persists the resulting state changes.

## Dependency rule for this project

- UI classes depend on service interfaces.
- Services depend on `INoteRepository`.
- The concrete `SqliteNoteRepository` is only created at the composition root.
- This is the part of the architecture that matters most for testability and substitution.

## Updated design summary

- `MainWindow` wires the app together.
- `NoteService`, `SnapshotService`, and `TrashService` define the behavior the UI uses.
- `SqliteNoteRepository` owns SQLite persistence.
- `IPlugin` remains a future-facing contract, not a currently wired subsystem.
- `AuditLogger` and `AuditLogPanel` handle the audit trail UI and log display.
