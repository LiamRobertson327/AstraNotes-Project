# AstraNotes: Simplified Project Structure

> **After Project Completion Note**: This file was created near the start of implementation.  The project does not stricly follow the structure discussed in the beginning of the project, however it is mostly correct.  The complete, finalized project structure is given at the end of the file.

## Alignment Note (April 25, 2026)

This file contains target-state structure plus aspirational module details. For implementation decisions, prioritize:

1. `src/` actual tree in the repository
2. `REQUIREMENTS_UPDATED.md` + `INITIAL_REQUIREMENTS.md`
3. `UML_STARTER_DIAGRAMS.md`
4. `IMPLEMENTATION_ROADMAP.md`

Current implemented baseline is intentionally lean (for example, `Note` model + Qt app skeleton), while repository/service/plugin modules are still being staged.

## Directory Layout (Streamlined for Rapid Development)

The simplified project layout is organized by architectural layer, with a shallow source tree and clear separation of responsibilities:

- `src/api/`: (Contains INote.h, IPlugin.h)
- `src/app/`: (Contains main.cpp)
- `src/model/`: (Contains Note.cpp/h)
- `src/ui/`: (Contains mainwindow.cpp/h/ui)
- `src/plugin/`, `src/repository/`, `src/service/`: (Empty for now)
- `tests/`: unit, integration, and performance tests.

The architecture intentionally avoids a monolithic in-memory `NoteCollection`; persistent note storage is handled through SQLite and retrieved on demand.

## Module Dependency Graph

The application starts from `main.cpp` and flows into the Qt-based UI layer. Controllers call services, which depend on the repository and core utility layers. Plugins are managed separately through a runtime plugin manager, and core infrastructure provides logging, configuration, error handling, and Qt platform utilities.

Key dependencies include:
- `INoteRepository` backed by `SQLiteNoteRepository`
- `NoteService`, `SearchService`, and `EncryptionService`
- UI controllers and Qt model adapters
- `std::expected<T, Error>` for explicit error propagation
- Qt platform abstractions in `ui`

## Build Targets (Simplified)

The build produces layered static/shared libraries for core utilities, domain model, repository, service, plugin, and UI modules, plus executables for the main application and the test runner.

---

## Layered Build Order

The build order is core first, followed by model, repository, service, plugin, and UI layers. The main executable links the assembled application layers, and the test runner links the test harness with all application modules.

---

## Module Responsibilities

| Module | Responsibility | Key Concepts | Dependencies |
|--------|-----------------|-------------|--------------|
| **core** | Shared utilities | Error, Logger, Config | STL only |
| **model** | Domain objects | Note hierarchy and metadata | core |
| **repository** | Data persistence | `INoteRepository`, SQLite persistence | core, model |
| **service** | Business logic | NoteService, SearchService, EncryptionService, CacheManager | core, model, repository |
| **plugin** | Plugin infrastructure | `IPlugin`, PluginManager, built-in plugins | core, model, service |
| **ui** | User interface | MainWindow, Controllers, Views, Qt adapters | core, model, service, plugin, Qt6 |
| **platform** | OS abstraction | Qt platform utilities via `QStandardPaths` and `QSysInfo` | Qt6 |

---

## CMakeLists.txt Structure

The root CMake configuration sets the project to C++23, enables Qt auto tools, and adds subdirectories in dependency order for core, model, repository, service, plugin, UI, the main executable, and tests. Installation targets copy the application binary and resources into the deployment layout.

---

## Deliverables by Module

### Core Module
- ✅ `std::expected<T, Error>`-based error handling
- ✅ `Error` class with code and message
- ✅ `Logger` interface wrapping spdlog
- ✅ `Config` YAML parser wrapper
- ✅ `Types.h` with common typedefs

### Model Module
- ✅ `Note` base class
- ✅ `TextNote`, `VoiceNote`, and `SecureNote` derivatives
- ✅ `NoteMetadata` structure
- ✅ format handling and validation abstractions

### Repository Module
- ✅ `INoteRepository` interface
- ✅ `SQLiteNoteRepository` (optimized for 10K+)
- ✅ `QueryBuilder` with WHERE/ORDER BY/LIMIT
- ✅ Database migrations framework

### Service Module
- ✅ `NoteService` (CRUD, validation, and business rules)
- ✅ `SearchService` (full-text search)
- ✅ `EncryptionService` (AES-256-GCM)
- ✅ `CacheManager` for bounded in-memory caching

### Plugin Module
- ✅ `IPlugin` interface
- ✅ `PluginManager` (load/unload/list)
- ✅ `TextPlugin` (built-in)
- ✅ `VoicePlugin` skeleton
- ✅ `SecurePlugin` with encryption

### UI Module
- ✅ `MainWindow` with menu bar, toolbars
- ✅ `NoteListView` (table widget)
- ✅ `NoteEditorView` (rich text editor)
- ✅ `SearchView` with filters
- ✅ `NoteController`, `SearchController`
- ✅ Qt models for MVC adapters


This structure provides a solid foundation for a professional, scalable C++23 application.
## 🗂️ Repository Structure
```
AstraNotes-Project
├── src/
│   ├── api/
│   │   ├── INote.h
│   │   ├── IPlugin.h
│   │   └── ISnapshot.h
│   ├── app/
│   │   └── main.cpp
│   ├── model/
│   │   ├── Note.h
│   │   ├── Note.cpp
│   │   ├── Snapshot.h
│   │   └── Snapshot.cpp
│   ├── plugins/
│   │   ├── IFormattingAction.h
│   │   ├── MarkdownFormattingPlugin.h
│   │   ├── MarkdownPlugin.h
│   │   ├── MarkdownPlugin.cpp
│   │   ├── PlaintextPlugin.h
│   │   ├── PlaintextPlugin.cpp
│   │   ├── PluginManager.h
│   │   └── PluginManager.cpp
│   ├── repository/
│   │   ├── INoteRepository.h
│   │   └── SqliteNoteRepository.h/.cpp
│   ├── service/
│   │   ├── interfaces/
│   │   │   ├── INoteService.h
│   │   │   ├── ISnapshotService.h
│   │   │   └── ITrashService.h
│   │   └── impl/
│   │       ├── NoteService.h/.cpp
│   │       ├── SnapshotService.h/.cpp
│   │       └── TrashService.h/.cpp
│   ├── crypto/
│   │   ├── EncryptionService.h
│   │   └── EncryptionService.cpp
│   ├── logging/
│   │   ├── AuditLogger.h
│   │   └── AuditLogger.cpp
│   └── ui/
│       ├── mainwindow.ui
│       ├── mainwindow.h
│       ├── mainwindow.cpp
│       ├── NoteListController.h/.cpp
│       ├── AuditLogPanel.h/.cpp
│       ├── SettingsDialog.h/.cpp
│       └── TrashDialog.h/.cpp
├── tests/
│   ├── integration/
│   │   ├── note_trash_integration_tests.cpp
│   │   └── repoistory_roundtrip_integration_test.cpp
│   ├── performance/
│   │   └── nfr_performance_tests.cpp
│   ├── smoke/
│   │   ├── mainwindow_quick_tests.cpp
│   │   ├── SmokeTests.cpp
│   │   └── trash_feature_smoke.cpp
│   └── unit/
│       ├── concurrency_tests.cpp
│       ├── encryption_failure_tests.cpp
│       ├── note_deletion_lifecycle_tests.cpp
│       ├── note_service_password_unit_tests.cpp
│       ├── note_service_trash_unit_tests.cpp
│       ├── repository_failure_tests.cpp
│       ├── search_edgecases_tests.cpp
│       ├── snapshot_service_tests.cpp
│       └── trash_service_tests.cpp
├── docs/
│   ├── architecture/
│   ├── legacy/
│   ├── operations/
│   ├── planning/
│   ├── requirements/
│   ├── UML diagrams/
│   └── validaiton/
├── CMakeLists.txt
├── .gitignore
├── LICENSE
└── README.md
```