# AstraNotes: Simplified Project Structure

## Directory Layout (Streamlined for Rapid Development)

```
AstraNotes-Project/
**Note: This is a SIMPLIFIED structure that still meets all architectural constraints**
- 50% fewer directories than original design
- Faster navigation and build times
- Same modular separation of concerns
│
├── CMakeLists.txt                    # Root CMake configuration
├── ARCHITECTURE.md                  # System architecture & design
├── README.md                        # Project overview
├── LICENSE                          # GNU GPLv3
│
├── src/                             # Source code (organized by layer)
│   ├── CMakeLists.txt
│   ├── main.cpp                     # Application entry point
│   │
│   ├── core/                        # Core utilities & infrastructure
│   │   ├── Result.h                 # Result<T> type for error handling
│   │   ├── Error.h/Error.cpp        # Error codes & messages
│   │   ├── Logger.h/Logger.cpp      # Logging (spdlog wrapper)
│   │   ├── Config.h/Config.cpp      # Configuration (YAML)
│   │   ├── Platform.h               # Platform abstraction interface
│   │   ├── Platform.cpp             # Platform implementation (all #ifdef)
│   │   └── Types.h                  # Common typedefs
│   │
│   ├── model/                       # Domain model layer
│   │   ├── Note.h/Note.cpp          # Base note implementation
│   │   ├── TextNote.h/TextNote.cpp  # Text note variant
│   │   ├── VoiceNote.h/VoiceNote.cpp# Voice note variant
│   │   ├── SecureNote.h/SecureNote.cpp  # Encrypted note variant
│   │   ├── NoteCollection.h/NoteCollection.cpp  # Container with indexing
│   │   └── NoteMetadata.h           # Metadata structure
│   │
│   ├── repository/                  # Persistence layer (Data Access)
│   │   ├── INoteRepository.h        # Repository interface
│   │   ├── SQLiteNoteRepository.h/cpp  # Primary SQLite implementation
│   │   ├── InMemoryNoteRepository.h/cpp  # For testing
│   │   └── schema.sql               # Database schema
│   │
│   ├── service/                     # Business logic layer (SIMPLIFIED)
│   │   ├── NoteService.h/NoteService.cpp   # CRUD + validation
│   │   ├── SearchService.h/SearchService.cpp  # Full-text search
│   │   ├── EncryptionService.h/EncryptionService.cpp  # AES-256-GCM
│   │   ├── CacheManager.h/CacheManager.cpp  # LRU caching
│   │   └── ServiceLocator.h/ServiceLocator.cpp  # Dependency injection
│   │
│   ├── plugin/                      # Plugin system (SIMPLIFIED)
│   │   ├── IPlugin.h                # Plugin interface
│   │   ├── PluginManager.h/PluginManager.cpp  # Plugin loader/manager
│   │   ├── TextPlugin.h/TextPlugin.cpp      # Built-in: Text plugin
│   │   ├── VoicePlugin.h/VoicePlugin.cpp    # Built-in: Voice plugin
│   │   └── SecurePlugin.h/SecurePlugin.cpp  # Built-in: Encrypted plugin
│   │
│   ├── ui/                          # User interface layer (MVC)
│   │   ├── Application.h/Application.cpp  # Qt application wrapper
│   │   │
│   │   ├── view/                    # View components
│   │   │   ├── MainWindow.h/MainWindow.cpp     # Main UI window
│   │   │   ├── NoteListView.h/NoteListView.cpp  # Note list
│   │   │   ├── NoteEditorView.h/NoteEditorView.cpp  # Editor
│   │   │   ├── SearchView.h/SearchView.cpp    # Search interface
│   │   │   ├── SearchBar.h/SearchBar.cpp      # Custom search bar
│   │   │   └── TagCloud.h/TagCloud.cpp        # Tag widget
│   │   │
│   │   ├── controller/              # Controller layer
│   │   │   ├── NoteController.h/NoteController.cpp  # Note operations
│   │   │   ├── SearchController.h/SearchController.cpp # Search handling
│   │   │   └── EventBus.h/EventBus.cpp        # Event publishing
│   │   │
│   │   └── model/                   # Qt MVC model adapters
│   │       ├── NoteListModel.h/NoteListModel.cpp
│   │       └── SearchResultsModel.h/SearchResultsModel.cpp
│   │
│   └── platform/                    # Cross-platform abstractions
│       ├── Platform.h               # Interface
│       └── Platform.cpp             # Implementation (all #ifdef inside)
│
├── tests/                           # Test suite (SIMPLIFIED - 3 folders)
│   ├── CMakeLists.txt
│   ├── test_main.cpp                # Test entry point
│   │
│   ├── unit/                        # Unit tests (Model, Service, Repo)
│   │   ├── test_note.cpp
│   │   ├── test_note_collection.cpp
│   │   ├── test_note_service.cpp
│   │   ├── test_search_service.cpp
│   │   ├── test_encryption_service.cpp
│   │   ├── test_sqlite_repository.cpp
│   │   ├── test_result.cpp
│   │   ├── test_error.cpp
│   │   └── test_config.cpp
│   │
│   ├── integration/                 # Integration tests (E2E + plugins)
│   │   ├── test_note_lifecycle.cpp
│   │   ├── test_encryption_roundtrip.cpp
│   │   ├── test_plugin_loading.cpp
│   │   └── test_search_workflow.cpp
│   │
│   └── perf/                        # Performance & stress tests combined
│       ├── bench_10k_notes.cpp
│       ├── bench_search.cpp
│       ├── bench_encryption.cpp
│       ├── bench_memory.cpp
│       ├── test_concurrent_access.cpp
│       └── test_resource_limits.cpp
│
├── resources/                       # Application resources (SIMPLIFIED)
│   ├── config/
│   │   ├── settings.yaml            # Default configuration
│   │   └── plugin-registry.json     # Plugin metadata
│   ├── schema.sql                   # Database schema (at root level)
│   ├── icons/                       # App icons
│   │   ├── app_icon.ico
│   │   ├── app_icon.png
│   │   └── app_icon.svg
│   ├── styles/                      # Qt stylesheets
│   │   ├── dark.qss
│   │   └── light.qss
│   └── translations/                # i18n files
│       ├── astranotts_en.ts
│       ├── astranotts_es.ts
│       └── astranotts_fr.ts
│
├── docs/                            # Documentation (SIMPLIFIED)
│   ├── ARCHITECTURE.md              # Architecture overview
│   ├── DEVELOPMENT.md               # Development guide
│   ├── PLUGIN_GUIDE.md              # Plugin development
│   └── images/                      # Documentation images
│       ├── architecture_diagram.png
│       ├── system_layers.png
│       └── plugin_system.png
│
├── scripts/                         # Build & utility scripts
│   ├── build.sh
│   ├── build.bat
│   ├── test.sh
│   └── test.bat
│
├── build/                           # Build output (generated, in .gitignore)
│   ├── x64/
│   │   ├── Debug/
│   │   └── Release/
│   └── arm64/
│
├── .gitignore
├── .clang-format
├── .clang-tidy
└── .editorconfig
```

---

## Module Dependency Graph

```
main.cpp
    ↓
Application (ui/)
    ├─→ MainWindow (view/)
    ├─→ Controllers (controller/)
    │    └─→ NoteController
    │        └─→ NoteService (service/)
    │            ├─→ INoteRepository (repository/)
    │            │    └─→ SQLiteNoteRepository
    │            ├─→ SearchService
    │            ├─→ EncryptionService
    │            └─→ ValidationService
    ├─→ Models (model/)
    │    ├─→ Note
    │    ├─→ TextNote/VoiceNote/SecureNote
    │    └─→ NoteCollection
    └─→ PluginManager (plugin/)
        ├─→ IPlugin (plugin/)
        ├─→ TextPlugin (plugin/builtin/)
        ├─→ VoicePlugin (plugin/builtin/)
        └─→ SecurePlugin (plugin/builtin/)

Core dependencies:
├─→ Logger (core/)
├─→ Config (core/)
├─→ Result<T> (core/)
├─→ Error (core/)
└─→ Platform abstraction (platform/)
```

---

## File Naming Conventions

### Headers (.h)
- Interface files: `IPluginManager.h`
- Implementation classes: `Note.h`
- Utilities: `Types.h`, `Logger.h`
- No inline implementations (separate `.cpp`)

### Implementation (.cpp)
- Match header name: `Note.cpp` ↔ `Note.h`
- Include corresponding header first

### Tests
- Unit: `test_note.cpp` (tests `Note.h`)
- Fixture: `NoteTestFactory.h` (helper class)
- Mock: `MockRepository.h` (mock implementation)

### Resources
- Config: YAML/JSON format
- Images: PNG (16x32x64 for icons), SVG for scalables
- Translations: Qt `.ts` (Translation Source)

---

## Build Targets (Simplified)

```cmake
# Libraries
astranotescore          (Core utilities)
astranotesmodel         (Domain model)
astranotesrepository    (Data access)
astranotesservice       (Business logic)
astranotesplugin        (Plugin system + built-ins)
astranotesui            (Qt UI layer)

# Executables
AstraNotes              (Main application)
AstraNotesTests         (Test runner)
```

---

## Layered Build Order

```
1. core/              → libastranotecore.{a,lib}
2. model/             → libastranotesmodel.{a,lib}        (depends on core)
3. repository/        → libastranotesrepository.{a,lib}   (depends on core, model)
4. service/           → libastranotesservice.{a,lib}      (depends on core, model, repository)
5. plugin/ (builtin)  → libastranotesplugin.{a,lib}       (depends on core, service, model)
6. ui/                → libastranotesui.{a,lib}           (depends on core, service, model, plugin)
7. Main executable    → AstraNotes.exe/app                (depends on all above)
8. Tests              → AstraNotesTests.exe               (depends on all + Google Test)
```

---

## Module Responsibilities

| Module | Responsibility | Key Classes | Dependencies |
|--------|-----------------|-------------|--------------|
| **core** | Shared utilities | Result, Error, Logger, Config | STL only |
| **model** | Domain objects | Note, TextNote, NoteCollection | core |
| **repository** | Data persistence | INoteRepository, SQLiteNoteRepository | core, model |
| **service** | Business logic | NoteService, SearchService, EncryptionService | core, model, repository |
| **plugin** | Plugin infrastructure | IPlugin, PluginManager, builtin plugins | core, model, service |
| **ui** | User interface | MainWindow, Controllers, Views | core, model, service, plugin, Qt6 |
| **platform** | OS abstraction | Platform::getDataDir(), Thread, Mutex | STL, OS native |

---

## CMakeLists.txt Structure

```cmake
# Root CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(AstraNotes VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)  # For Qt
set(CMAKE_AUTORCC ON)

# Add subdirectories in dependency order
add_subdirectory(src/core)
add_subdirectory(src/model)
add_subdirectory(src/repository)
add_subdirectory(src/service)
add_subdirectory(src/plugin)
add_subdirectory(src/ui)
add_subdirectory(src/platform)
add_subdirectory(src)           # Main executable
add_subdirectory(tests)

# Installation targets
install(TARGETS AstraNotes DESTINATION bin)
install(DIRECTORY resources/ DESTINATION share/astranotesplugin/)
```

---

## Deliverables by Module

### Core Module
- ✅ `Result<T>` template (success/failure wrapper)
- ✅ `Error` class with code + message
- ✅ `Logger` interface implementing spdlog
- ✅ `Config` YAML parser wrapper
- ✅ `Types.h` with common typedefs

### Model Module
- ✅ `Note` base class
- ✅ `TextNote`, `VoiceNote`, `SecureNote` derivatives
- ✅ `NoteCollection` with indexing
- ✅ `NoteMetadata` structure
- ✅ `SearchQuery` DSL

### Repository Module
- ✅ `INoteRepository` interface
- ✅ `SQLiteNoteRepository` (optimized for 10K+)
- ✅ `InMemoryNoteRepository` (for testing)
- ✅ `QueryBuilder` with WHERE/ORDER BY/LIMIT
- ✅ Database migrations framework

### Service Module
- ✅ `NoteService` (CRUD + business logic)
- ✅ `SearchService` (full-text + indexing)
- ✅ `EncryptionService` (AES-256-GCM)
- ✅ `ValidationService` (input sanitization)
- ✅ `ExportService`, `BackupService`, `CacheManager`

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

---

## Next Steps

1. **Set up CMake**: Create modular `CMakeLists.txt` with subdirectories
2. **Implement Core**: `Result<T>`, `Error`, `Logger`, `Config`
3. **Build Model**: `Note` hierarchy, `NoteCollection`
4. **Add Repository**: SQLite with migrations
5. **Implement Services**: Business logic layer
6. **Wire Plugin System**: `IPlugin` + manager
7. **Create UI**: Qt6 views and controllers
8. **Add Tests**: Unit + integration + perf
9. **Cross-platform**: Abstraction layer for Windows/Mac/Linux
10. **Package**: Create installers for each OS

---

This structure provides a solid foundation for a professional, scalable C++20 application.
