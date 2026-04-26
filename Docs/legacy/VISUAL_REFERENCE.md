# AstraNotes: Visual Architecture Reference

## Alignment Note (April 25, 2026)

This file is a conceptual visual reference and includes legacy alternatives.

- Authoritative diagrams for current implementation planning are in `UML_STARTER_DIAGRAMS.md`.
- Where this file conflicts with current docs (for example, extra services, typed note hierarchy variants, or repository variants), prefer `UML_STARTER_DIAGRAMS.md` and `ARCHITECTURE.md`.

## 1. COMPLETE SYSTEM DIAGRAM

┌────────────────────────────────────────────────────────────────────────────┐
│                      ASTRANOTESS APPLICATION                              │
├────────────────────────────────────────────────────────────────────────────┤
│                                                                            │
│  ┌─ USER INTERFACE LAYER (Qt6 - MVC) ──────────────────────────────────┐ │
│  │                                                                      │ │
│  │  ┌─────────────┐  ┌──────────────┐  ┌──────────────────────────┐   │ │
│  │  │   Views     │  │ Controllers  │  │ Models (Qt Adapters)     │   │ │
│  │  ├─────────────┤  ├──────────────┤  ├──────────────────────────┤   │ │
│  │  │MainWindow   │  │NoteCtrl      │  │NoteListModel             │   │ │
│  │  │NoteListView │  │SearchCtrl    │  │SearchResultsModel        │   │ │
│  │  │EditorView   │  │PluginCtrl    │  │NoteViewModel             │   │ │
│  │  │SearchView   │  │SettingsCtrl  │  │                          │   │ │
│  │  └─────────────┘  └──────────────┘  └──────────────────────────┘   │ │
│  │                                                                      │ │
│  └─ Signals/Slots (Event Bus) ─────────────────────────────────────────┘ │
│                           ↓                                               │
│  ┌─ APPLICATION LAYER (Manager Classes) ──────────────────────────────┐ │
│  │                                                                      │ │
│  │  ┌──────────────────┐  ┌─────────────────┐  ┌────────────────────┐ │ │
│  │  │ Application Mgr  │  │ Note Manager    │  │ Plugin Manager     │ │ │
│  │  │ (Lifecycle)      │  │ (CRUD + Logic)  │  │ (Load/Unload)      │ │ │
│  │  └──────────────────┘  └─────────────────┘  └────────────────────┘ │ │
│  │                                                                      │ │
│  └──────────────────────────────────────────────────────────────────────┘ │
│                           ↓                                               │
│  ┌─ DOMAIN MODEL LAYER (Business Objects) ───────────────────────────┐ │
│  │                                                                      │ │
│  │         ┌────────────────┐                                         │ │
│  │         │   INote        │ (Abstract)                              │ │
│  │         └────────────────┘                                         │ │
│  │              ↑                                                      │ │
│  │      ┌───────┼───────┐                                             │ │
│  │      │       │       │                                             │ │
│  │   ┌──▼──┐  ┌─▼──┐  ┌─▼────────┐                                    │ │
│  │   │Note │  │Text│  │Voice│Sec │                                    │ │
│  │   └─────┘  └────┘  └──────────┘                                    │ │
│  │                                                                      │ │
│  │      ┌─────────────────────────────┐                               │ │
│  │      │ SQLite-backed note persistence              │                               │ │
│  │      │ - Map<NoteID, Note*>        │                               │ │
│  │      │ - TagIndex                  │                               │ │
│  │      │ - Full-text search support  │                               │ │
│  │      └─────────────────────────────┘                               │ │
│  │                                                                      │ │
│  └──────────────────────────────────────────────────────────────────────┘ │
│                           ↓                                               │
│  ┌─ PLUGIN SYSTEM LAYER ─────────────────────────────────────────────┐ │
│  │                                                                      │ │
│  │  ┌────────────────────────────────────────────────────────────┐   │ │
│  │  │ IPlugin (Abstract Interface)                               │   │ │
│  │  │ - initialize()                                             │   │ │
│  │  │ - processNote()                                            │   │ │
│  │  │ - getSupportedTypes()                                      │   │ │
│  │  └────────────────────────────────────────────────────────────┘   │ │
│  │              ↑                                                      │ │
│  │   ┌──────────┼──────────┐                                          │ │
│  │   │          │          │                                          │ │
│  │┌──▼──┐  ┌───▼──┐  ┌────▼─────┐                                    │ │
│  ││Text ││ │Voice ││ │Secure   ││                                    │ │
│  │└─────┘  └──────┘  └─────────┘                                    │ │
│  │                                                                      │ │
│  │  PluginManager: Load/Unload/Execute plugins                        │ │
│  │                                                                      │ │
│  └──────────────────────────────────────────────────────────────────────┘ │
│                           ↓                                               │
│  ┌─ SERVICE LAYER (Business Logic) ──────────────────────────────────┐ │
│  │                                                                      │ │
│  │ ┌──────────────┐  ┌──────────────┐  ┌─────────────────┐            │ │
│  │ │NoteService   │  │SearchService │  │EncryptionSvc   │            │ │
│  │ │- CRUD Ops    │  │- Full-text   │  │- AES-256-GCM   │            │ │
│  │ │- Validation  │  │- Indexing    │  │- Key Derivation│            │ │
│  │ └──────────────┘  └──────────────┘  └─────────────────┘            │ │
│  │                                                                      │ │
│  │ ┌──────────────┐  ┌──────────────┐  ┌─────────────────┐            │ │
│  │ │ValidationSvc │  │BackupService │  │CacheManager     │            │ │
│  │ │- Input check │  │- Snapshots   │  │- LRU Cache      │            │ │
│  │ └──────────────┘  └──────────────┘  └─────────────────┘            │ │
│  │                                                                      │ │
│  └──────────────────────────────────────────────────────────────────────┘ │
│                           ↓                                               │
│  ┌─ PERSISTENCE LAYER (Data Access) ────────────────────────────────┐ │
│  │                                                                      │ │
│  │  ┌──────────────────────────────────────────────────────┐          │ │
│  │  │     INoteRepository (Interface)                      │          │ │
│  │  │  - get(id)                                           │          │ │
│  │  │  - save(note)                                        │          │ │
│  │  │  - search(query)                                     │          │ │
│  │  │  - delete(id)                                        │          │ │
│  │  └──────────────────────────────────────────────────────┘          │ │
│  │                  ↑                                                   │ │
│  │   ┌──────────────┼──────────────┐                                  │ │
│  │   │              │              │                                  │ │
│  ││SQLiteRepo  ││FileSystemRepo ││InMemoryRepo││                   │ │
│  │   (Primary)     (Backup)        (Testing)                          │ │
│  │                                                                      │ │
│  │  QueryBuilder: Advanced search DSL                                 │ │
│  │                                                                      │ │
│  └──────────────────────────────────────────────────────────────────────┘ │
│                           ↓                                               │
│  ┌─ INFRASTRUCTURE LAYER ────────────────────────────────────────────┐ │
│  │                                                                      │ │
│  │  ┌──────────┐  ┌────────────┐  ┌────────────┐  ┌──────────────┐  │ │
│  │  │ Logger   │  │ Config     │  │ std::expected<T, Error> │  │ Error       │  │ │
│  │  │(spdlog)  │  │(YAML)      │  │(Railway-  │  │(ErrorCode + │  │ │
│  │  │          │  │            │  │ oriented) │  │ message)    │  │ │
│  │  └──────────┘  └────────────┘  └────────────┘  └──────────────┘  │ │
│  │                                                                      │ │
│  │  ┌──────────────────────────────────────────────────────┐          │ │
│  │  │ Platform Abstraction (Windows/Mac/Linux)            │          │ │
│  │  │ - getDataDirectory()                                │          │ │
│  │  │ - getPluginPath()                                   │          │ │
│  │  │ - Thread, Mutex, File I/O                          │          │ │
│  │  └──────────────────────────────────────────────────────┘          │ │
│  │                                                                      │ │
│  └──────────────────────────────────────────────────────────────────────┘ │
│                           ↓                                               │
│  ┌─ DATA LAYER (Persistence) ────────────────────────────────────────┐ │
│  │                                                                      │ │
│  │  ┌─────────────────────────────────────────────────┐               │ │
│  │  │ SQLite Database                                 │               │ │
│  │  │ ├─ notes (id, title, content, type, created...)│               │ │
│  │  │ ├─ note_tags (note_id, tag)                     │               │ │
│  │  │ ├─ audit_log (note_id, action, timestamp)       │               │ │
│  │  │ └─ (Indexed for performance)                    │               │ │
│  │  └─────────────────────────────────────────────────┘               │ │
│  │                                                                      │ │
│  └──────────────────────────────────────────────────────────────────────┘ │
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘

---

## 2. CLASS HIERARCHY - NOTE TYPES

     ┌─────────────────────────────────────────┐
     │          INote (Abstract)                │
     ├─────────────────────────────────────────┤
     │ # id: NoteID                            │
     │ # title: string                         │
     │ # content: string/binary                │
     │ # createdAt, updatedAt: timestamp       │
     │ # tags: vector<string>                  │
     │ + getTitle(): string                    │
     │ + getContent(): string                  │
     │ + getTags(): vector<string>             │
     │ + getMetadata(): NoteMetadata           │
     └─────────────────────────────────────────┘
        ↑           ↑           ↑
     ┌──┴─┐      ┌──┴──┐      ┌──┴────────┐
     │    │      │     │      │           │
   Text  Markdown Voice Secure Audio     (Custom)
   Note  Note     Note   Note   Note     Plugins
    │      │       │      │      │
    └─┬────┴───┬───┴──┬───┴──────┘
      │        │      │
   Markdown  Voice    Encrypted
   Support   Record   Storage

---

## 3. PLUGIN ARCHITECTURE

┌────────────────────────────────────────────────────────────────┐
│                     Plugin System                               │
├────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌─────────────────────────────────────────────────────┐       │
│  │  IPlugin (Abstract Interface)                       │       │
│  │  + getName(): string                                │       │
│  │  + getVersion(): string                             │       │
│  │  + initialize(IPluginContext*): bool                │       │
│  │  + shutdown(): bool                                 │       │
│  │  + getSupportedNoteTypes(): vector<string>          │       │
│  │  │ + processNote(const Note&): std::expected<std::string, Error>         │       │
│  │  │ + exportNote(const Note&, path): std::expected<void, Error>      │       │
│  │  │ + importNote(path): std::expected<std::unique_ptr<Note>, Error>       │       │
│  └─────────────────────────────────────────────────────┘       │
│                          ↑                                       │
│         ┌────────────────┼────────────────┐                    │
│         │                │                │                    │
│    ┌────▼─────┐    ┌─────▼─┐      ┌─────▼──────┐              │
│    │TextPlugin│    │Voice ││      │SecurePlugin │              │
│    │           │    │Plugin│      │            │              │
│    │-Markdown  │    │      │      │-AES-256    │              │
│    │-Plain     │    │-Record│      │-Key Mgmt   │              │
│    │-RichText  │    │-Play  │      │-ACL        │              │
│    │-Export    │    │-Trans │      │-Audit Log  │              │
│    └────┬─────┘    └──┬───┘      └──────┬─────┘              │
│         │             │                  │                    │
│         └─────────────┼──────────────────┘                    │
│                       │                                        │
│  ┌────────────────────▼─────────────────────────────────┐    │
│  │  PluginManager                                       │    │
│  │  - loadPlugin(path): std::expected<void, Error>                   │    │
│  │  - unloadPlugin(name): std::expected<void, Error>                 │    │
│  │  - getPlugin(name): std::expected<IPlugin*, Error>                │    │
│  │  - listLoadedPlugins(): vector<string>              │    │
│  │  - executePlugin(name, note): std::expected<std::string, Error>        │    │
│  └────────────────────┬─────────────────────────────────┘    │
│                       │                                        │
│  ┌────────────────────▼─────────────────────────────────┐    │
│  │  IPluginContext (Provides access to services)       │    │
│  │  - getLogger()                                       │    │
│  │  - getRepository()                                   │    │
│  │  - getEncryption()                                   │    │
│  │  - getConfig()                                       │    │
│  └────────────────────────────────────────────────────────┘  │
│                                                                  │
└────────────────────────────────────────────────────────────────┘

---

## 4. DATABASE SCHEMA

┌─────────────────────────────────────────────────┐
│ notes                                            │
├─────────────────────────────────────────────────┤
│ id (PK, AUTO_INCREMENT)      ┌───────────────┐  │
│ title (NOT NULL)             │ Indices:      │  │
│ content (NOT NULL)           │ - id (PK)     │  │
│ type (TEXT/VOICE/SECURE)     │ - created_at  │  │
│ format (plain/markdown)      │ - type        │  │
│ created_at (TIMESTAMP)       │ - updated_at  │  │
│ updated_at (TIMESTAMP)       └───────────────┘  │
│ is_encrypted (BOOLEAN)                          │
│ encryption_key_hash (VARCHAR)                   │
│ metadata (JSON: {tags, author, ...})            │
└─────────────────────────────────────────────────┘
         │                                  ↓
         │              ┌──────────────────────────┐
         │              │ note_tags                │
         │              ├──────────────────────────┤
         ├─────────────▶│ note_id (FK)             │
         │              │ tag (TEXT)               │
         │              │ (Composite PK + Index)  │
         │              └──────────────────────────┘
         │
         └──────────────┐
                        │
                   ┌────▼────────────────┐
                   │ audit_log           │
                   ├─────────────────────┤
                   │ id (PK)             │
                   │ note_id (FK)        │
                   │ action (TEXT)       │
                   │ timestamp (auto)    │
                   │ user_id (optional)  │
                   └─────────────────────┘


## 5. MVC FLOW - DATA & CONTROL

User Interaction               View Layer
     │                            │
     │ (Click "New Note")         │
     │──────────────────────────▶ │ NoteListView::onNewNoteClicked()
     │                            │
     │                            ▼
     │                      NoteController
     │                       │
     │                       │ noteCreated signal
     │                       │
     │                       ▼
     │                  NoteService
     │                  - Validate input
     │                  - Create Note
     │                  - Encrypt (if needed)
     │                  - Call Repository
     │                       │
     │                       ▼
     │                  SQLiteRepository
     │                  - Insert into DB
     │                  - Return created Note
     │                       │
     │                       ▼
     │                  NoteService (return)
     │                       │
     │                       ▼
     │                  NoteController (emit signal)
     │                       │
     │                       ▼
     │              NoteListModel::addNote()
     │              Update Qt Model
     │                       │
     │◀──────────────────────┘
     │
     │ View automatically updates (Qt signals/slots)
     │
     ▼
  Display new note in list

---

## 6. DEPENDENCY INJECTION FLOW

Application Entry Point (main.cpp)
     │
     ▼
┌─────────────────────────────────────┐
│ ServiceLocator (Singleton)          │
│                                     │
│ Registers all singletons:           │
│ - Logger instance                   │
│ - Config instance                   │
│ - Database connection               │
│ - Repository instance               │
│ - Encryption service                │
│ - Search service                    │
│ - Note service                      │
│ - Plugin manager                    │
│ - Cache manager                     │
│                                     │
│ Method: getInstance<T>(name)        │
│ Returns: T*                         │
└─────────────────────────────────────┘
     │
     ├─ Controllers
     │  │ Fetch: NoteService via ServiceLocator::getInstance<NoteService>()
     │  │ Use: service->createNote(request)
     │
     ├─ Services
     │  │ Fetch: Repository via ServiceLocator::getInstance<INoteRepository>()
     │  │ Use: repo->save(note)
     │
     └─ Plugins
        │ Fetch: IPluginContext (provided by manager)
        │ Use: context->getRepository()->getNote(id)

---

## 7. ERROR HANDLING FLOW

Operation Request
     │
     ▼
ValidationService::validate(input)
     │
     ├─ Valid? → Continue
     │
     └─ Invalid? → Return std::expected<void, Error>
                       Error{InvalidInput, "Title too long"}
                       │
                       ▼
                  Check isErr()
                       │
                       ├─ isErr()? → Handle error
                       │             Display to user
                       │
                       └─ isOk()? → Continue to service
                                    │
                                    ▼
                             NoteService::createNote()
                                    │
                                    ├─ Logic OK? → Repository::save()
                                    │                    │
                                    │                    ├─ DB OK? → Return std::expected<Note, Error>
                                    │                    │
                                    │                    └─ DB Error? → Error{StorageError, "..."}
                                    │
                                    └─ Logic Error? → Return std::expected<Note, Error>
                                                      Error{...}
                                                           │
                                                           ▼
                                                    Return to controller
                                                           │
                                                           ├─ isOk()? → Update UI
                                                           │
                                                           └─ isErr()? → Show dialog

---

## 8. MEMORY MANAGEMENT - SMART POINTER USAGE

┌─────────────────────────────────────────────────────────────┐
│              Smart Pointer Strategy                          │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  Use Case                │  Type                           │
│  ──────────────────────────────────────────────────────     │
│  File handles            │  unique_ptr (RAII cleanup)      │
│  DB connections          │  unique_ptr + custom deleter    │
│  Services (singletons)   │  shared_ptr + ServiceLocator    │
│  Domain objects          │  shared_ptr (passed around)     │
│  Plugin instances        │  shared_ptr (lifecycle mgmt)    │
│  Cache entries           │  weak_ptr (avoid cycles)        │
│  Test mocks              │  shared_ptr + mock framework    │
│                                                              │
└─────────────────────────────────────────────────────────────┘

Example:
┌──────────────────────────────────────────────────┐
│ auto fileGuard = std::make_unique<FileHandle>(   │
│   "notes.db",                                    │
│   std::default_delete<FileHandle>()              │
│ );  ← Automatically closes on scope exit         │
│                                                   │
│ auto note = std::make_shared<Note>("My Note");   │
│ cache[noteId] = note;                            │
│ // note still referenced; won't be deleted      │
│ // until both cache and all local copies gone   │
└──────────────────────────────────────────────────┘

---

## 9. PERFORMANCE OPTIMIZATION TARGETS

┌──────────────────────────────────────────────────────────┐
│  Operation           │  Target    │  Implementation     │
├──────────────────────────────────────────────────────────┤
│  Create note         │  <50ms     │  Direct DB insert   │
│  Fetch note          │  <10ms     │  Indexed lookup     │
│  List 1K notes       │  <100ms    │  Paging + select    │
│  Full-text search    │  <200ms    │  Inverted index     │
│  Encrypt 1MB         │  <100ms    │  AES hw accel       │
│  Plugin load         │  <500ms    │  Lazy loading       │
│  UI update           │  60 FPS    │  Async DB ops      │
│  Memory (10K notes)  │  <200MB    │  Compression +     │
│                      │            │  LRU cache         │
└──────────────────────────────────────────────────────────┘

---

## 10. TESTING PYRAMID

                      ▲
                     ╱ ╲
                    ╱   ╲
                   ╱  UI  ╲        10% - High-level tests
                  ╱Tests  ╲       (Controller, View)
                 ╱─────────╲
                ╱           ╲
               ╱ Integration ╲    30% - Workflow tests
              ╱    Tests      ╲   (E2E scenarios)
             ╱─────────────────╲
            ╱                   ╲
           ╱    Unit Tests       ╲  60% - Low-level tests
          ╱                       ╲ (Model, Service, Repo)
         ╱─────────────────────────╲
        ╱                           ╲
       ╱─────────────────────────────╲
      
Targets:
- Unit: 80%+ coverage (Model, Service, Repository)
- Integration: All critical workflows
- Performance: 10K+ note benchmarks
- UI: Critical user paths
- Stress: Concurrent access, memory limits

---

## 11. DEPLOYMENT TARGETS

AstraNotes is deployed as a portable folder structure for cross-platform compatibility. All platforms follow the same directory layout with platform-specific binaries and plugins.

Portable Folder Structure (All Platforms)

AstraNotes/
├─ AstraNotes (executable: .exe on Windows, .app on macOS, binary on Linux)
├─ plugins/
│  ├─ TextPlugin (dynamic library: .dll, .dylib, .so)
│  └─ VoicePlugin (dynamic library: .dll, .dylib, .so)
├─ resources/
│  ├─ config.yaml
│  ├─ icons/
│  ├─ styles/
│  └─ translations/
├─ db/
│  └─ notes.db (user SQLite database)
└─ README.txt (quick start guide)

Deployment Note:
This portable structure simplifies distribution across Windows, macOS, and Linux. Users extract the folder and run the AstraNotes executable. All plugins are discovered dynamically at runtime from the plugins/ directory using QPluginLoader, ensuring consistent behavior across all platforms.

---

## QUICK REFERENCE: WHERE TO FIND THINGS

| Feature | Location |
|---------|----------|
| **Domain Model** | `src/model/Note.h` |
| **Plugin Interface** | `src/api/IPlugin.h` |
| **Data Access** | `src/repository/INoteRepository.h`, `SQLiteNoteRepository.h` |
| **Business Logic** | `src/service/NoteService.h`, `EncryptionService.h`, `SearchService.h` |
| **Plugin Manager** | `src/plugin/PluginManager.h` |
| **UI** | `src/ui/MainWindow.h`, `src/ui/view/*.h`, `src/ui/controller/*.h` |
| **Core Utilities** | `src/api/`, `src/app/main.cpp` |
| **Tests** | `tests/unit/*`, `tests/integration/*`, `tests/perf/*` |
| **Database Schema** | `src/repository/schema/schema.sql` |
| **Documentation** | `docs/`, `ARCHITECTURE.md`, `PROJECT_STRUCTURE.md`, `IMPLEMENTATION_ROADMAP.md` |

---

This visual reference should help you navigate the architecture and understand the relationships between components. Refer back to ARCHITECTURE.md and PROJECT_STRUCTURE.md for detailed implementation specifics.
