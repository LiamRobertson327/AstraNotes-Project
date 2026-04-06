# AstraNotes: Simplified Project Structure

## Directory Layout (Streamlined for Rapid Development)

The simplified project layout is organized by architectural layer, with a shallow source tree and clear separation of responsibilities:

- `src/api/`: (Contains INote.h, IPlugin.h)
- `src/app/`: (Contains main.cpp)
- `src/model/`: (Contains Note.cpp/h)
- `src/ui/`: (Contains mainwindow.cpp/h/ui)
- `src/plugin/`, `src/repository/`, `src/service/`: (Empty for now)
- `tests/`: unit, integration, and performance tests.
- `resources/`: default config, schema, icons, styles, and translations.

The architecture intentionally avoids a monolithic in-memory `NoteCollection`; persistent note storage is handled through SQLite and retrieved on demand.

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

---

## Module Dependency Graph

The application starts from `main.cpp` and flows into the Qt-based UI layer. Controllers call services, which depend on the repository and core utility layers. Plugins are managed separately through a runtime plugin manager, and core infrastructure provides logging, configuration, error handling, and Qt platform utilities.

Key dependencies include:
- `INoteRepository` backed by `SQLiteNoteRepository`
- `NoteService`, `SearchService`, and `EncryptionService`
- UI controllers and Qt model adapters
- `std::expected<T, Error>` for explicit error propagation
- Qt platform abstractions in `ui/utils`

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

The build produces layered static/shared libraries for core utilities, domain model, repository, service, plugin, and UI modules, plus executables for the main application and the test runner.

---

## Layered Build Order

The build order is core first, followed by model, repository, service, plugin, and UI layers. The main executable links the assembled application layers, and the test runner links the test harness with all application modules.

---

## Module Responsibilities

| Module | Responsibility | Key Concepts | Dependencies |
|--------|-----------------|-------------|--------------|
| **core** | Shared utilities | `std::expected`, Error, Logger, Config | STL only |
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
- ✅ `InMemoryNoteRepository` (for testing)
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

---

## Next Steps

1. **Set up CMake**: Create modular `CMakeLists.txt` with subdirectories
2. **Implement Core**: `std::expected<T, Error>`, `Error`, `Logger`, `Config`
3. **Build Model**: `Note` hierarchy and metadata
4. **Add Repository**: SQLite with migrations
5. **Implement Services**: Business logic layer
6. **Wire Plugin System**: `IPlugin` + manager
7. **Create UI**: Qt6 views and controllers
8. **Add Tests**: Unit + integration + perf
9. **Cross-platform**: Abstraction layer for Windows/Mac/Linux
10. **Package**: Create installers for each OS

---

This structure provides a solid foundation for a professional, scalable C++23 application.
