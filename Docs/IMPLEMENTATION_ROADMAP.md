# AstraNotes: Simplified Implementation Roadmap

**Version**: 2.0 (Streamlined)  
**Duration**: 10-12 weeks (accelerated from 14)  
**Effort**: 30-40 days (vs 40-50 for original)  

> This roadmap reflects the SIMPLIFIED architecture. 50% fewer tasks, same end result.

---

## Overview

This guide provides a **step-by-step implementation plan** using the streamlined modular architecture.

**Key Differences from Original:**
- 4 services instead of 8 → Fewer files to implement
- 3 test folders instead of 7 → Simpler test organization
- Built-in plugins linked directly → No plugin loader complexity
- Single Platform file → Easier cross-platform support
- Fewer CMakeLists.txt files → Faster builds

---

## Phase 1: Foundation & Core (Weeks 1-2)

### Goal
Set up the build system, core utilities, and domain model foundation.

### Tasks

#### 1.1 Simplify CMake Build System
**File**: `CMakeLists.txt`

**Key changes:**
- Only 3-4 top-level `add_subdirectory()` calls (core, model, repository, service, plugin, ui, main)
- Remove separate cmake/ folder (use find_package directly)
- No per-file CMakeLists.txt (just per-layer)

**Status**: 🔄 In Progress

#### 1.2 Implement Core Utilities
**Files**:
- `src/core/Result.h` - Template for success/failure
- `src/core/Error.h/cpp` - Simple error codes
- `src/core/Logger.h/cpp` - spdlog wrapper
- `src/core/Config.h/cpp` - YAML parser
- `src/core/Types.h` - Common typedefs
- `src/core/Platform.h/cpp` - All platform code in ONE file with #ifdef

**Key Code**:
```cpp
// Result.h - Simple and effective
template<typename T>
class Result {
  std::variant<T, Error> value;
public:
  bool isOk() const { return std::holds_alternative<T>(value); }
  T& unwrap() { return std::get<T>(value); }
};

// Platform.cpp - Single file, multiple implementations
#ifdef _WIN32
  std::string Platform::getDataDir() { /* Windows */ }
#elif __APPLE__
  std::string Platform::getDataDir() { /* macOS */ }
#else
  std::string Platform::getDataDir() { /* Linux */ }
#endif
```

**Status**: 🔄 In Progress

#### 1.3 Expand Note Model (No Changes to Original)
**Files**:
- Extend `include/Note.h` with metadata structs
- Create `TextNote.h/cpp`, `VoiceNote.h/cpp`, `SecureNote.h/cpp`

**Status**: ✅ Partially Done

#### 1.4 Create NoteCollection
**File**: `src/model/NoteCollection.h/cpp`

```cpp
class NoteCollection {
  std::unordered_map<NoteID, std::shared_ptr<Note>> notes;
  std::unordered_map<std::string, std::vector<NoteID>> tagIndex;
  
  void add(std::shared_ptr<Note> note);
  void remove(const NoteID& id);
  std::shared_ptr<Note> get(const NoteID& id) const;
  size_t size() const { return notes.size(); }
};
```

**Status**: 🔄 In Progress

#### 1.5 Setup Test Infrastructure
**Files**:
- `tests/CMakeLists.txt` - Link Google Test
- `tests/test_main.cpp` - Test runner
- `tests/unit/test_note.cpp` - Basic note tests
- `tests/unit/test_result.cpp` - Result<T> tests

**3 folders instead of 7:**
```
tests/
├── unit/           (All unit tests)
├── integration/    (E2E workflows)
└── perf/          (Performance + stress combined)
```

**Status**: ❌ Not Started

### Deliverables (Phase 1)
- ✅ Modular CMake with 3-4 subdirs
- ✅ Core utilities (Result, Error, Logger, Config, Platform)
- ✅ Enhanced Note model
- ✅ NoteCollection with indices
- ✅ Basic unit test framework (20+ tests)

### Effort: 3-4 days (faster than original 5-7)

---

## Phase 2: Persistence Layer (Weeks 3-4)

### Goal
Implement database access layer optimized for 10K+ notes.

### Tasks

#### 2.1 Design Database Schema
**File**: `src/repository/schema/schema.sql`

```sql
CREATE TABLE IF NOT EXISTS notes (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  title TEXT NOT NULL,
  content TEXT NOT NULL,
  type TEXT NOT NULL CHECK(type IN ('text', 'voice', 'secure')),
  format TEXT DEFAULT 'plain',
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  is_encrypted BOOLEAN DEFAULT FALSE,
  encryption_key_hash TEXT,
  metadata JSON
);

CREATE INDEX idx_created ON notes(created_at);
CREATE INDEX idx_type ON notes(type);
CREATE INDEX idx_updated ON notes(updated_at);

CREATE TABLE IF NOT EXISTS note_tags (
  note_id INTEGER NOT NULL,
  tag TEXT NOT NULL,
  PRIMARY KEY(note_id, tag),
  FOREIGN KEY(note_id) REFERENCES notes(id) ON DELETE CASCADE
);

CREATE INDEX idx_tag ON note_tags(tag);

CREATE TABLE IF NOT EXISTS audit_log (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  note_id INTEGER,
  action TEXT,
  timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY(note_id) REFERENCES notes(id)
);
```

**Status**: ✅ Designed

#### 2.2 Implement Repository Interface
**File**: `src/repository/INoteRepository.h`

```cpp
template<typename T>
class IRepository {
public:
  virtual ~IRepository() = default;
  virtual Result<std::shared_ptr<T>> get(const ID&) = 0;
  virtual Result<std::vector<std::shared_ptr<T>>> getAll() = 0;
  virtual Result<void> save(std::shared_ptr<T>) = 0;
  virtual Result<void> update(std::shared_ptr<T>) = 0;
  virtual Result<void> delete_item(const ID&) = 0;
  virtual Result<size_t> count() = 0;
};

using INoteRepository = IRepository<Note>;
```

**Status**: 🔄 In Progress

#### 2.3 Implement SQLiteNoteRepository
**Files**:
- `src/repository/SQLiteNoteRepository.h`
- `src/repository/SQLiteNoteRepository.cpp`

**Key Features**:
- Connection pooling (multi-threaded access)
- Prepared statements (prevent SQL injection)
- Batch operations (grouping in transactions)
- Paging support (for large result sets)

**Code Snippet**:
```cpp
class SQLiteNoteRepository : public INoteRepository {
  std::unique_ptr<sqlite3, decltype(&sqlite3_close)> db;
  std::unordered_map<std::string, sqlite3_stmt*> stmtCache;
  
  Result<std::shared_ptr<Note>> get(const NoteID& id) override {
    const char* sql = "SELECT * FROM notes WHERE id = ?";
    auto stmt = getOrPrepare(sql);
    sqlite3_bind_int64(stmt, 1, id);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      return Result<std::shared_ptr<Note>>(
        std::make_shared<Note>(/* from row */)
      );
    }
    return Result<std::shared_ptr<Note>>(
      Error{ErrorCode::NotFound, "Note not found"}
    );
  }
};
```

**Performance Notes**:
- Creates indices on frequently queried columns
- Uses WAL mode for concurrent read access
- Batches writes in transactions

**Status**: 🔄 In Progress

#### 2.4 Test SQLite Layer
**File**: `tests/integration/test_sqlite_repository.cpp`

**Tests**:
- ✅ Single note CRUD
- ✅ Batch inserts (1000 notes)
- ✅ Tag indexing
- ✅ Query performance (< 100ms for 10K notes)
- ✅ Concurrent access (thread safety)

**Status**: ❌ Not Started

### Deliverables
- ✅ SQLite schema with optimal indices
- ✅ Generic Repository interface
- ✅ SQLiteNoteRepository implementation
- ✅ Query builder for complex searches
- ✅ Integration tests (30+ tests)

### Performance Targets
- Create note: < 50ms
- Fetch 1000 notes: < 200ms
- Full-text search: < 300ms

### Estimated Effort: 5-7 days

---

## Phase 3: Service Layer (Weeks 5-6)

### Goal
Implement business logic, encryption, search, and validation.

### Tasks

#### 3.1 Implement NoteService
**File**: `src/service/NoteService.h/cpp`

```cpp
class NoteService {
  std::shared_ptr<INoteRepository> repo;
  std::shared_ptr<ValidationService> validator;
  std::shared_ptr<EncryptionService> encryptor;
  
public:
  Result<std::shared_ptr<Note>> createNote(const NoteCreateRequest&);
  Result<void> updateNote(const NoteUpdateRequest&);
  Result<void> deleteNote(const NoteID&);
  Result<std::shared_ptr<Note>> getNote(const NoteID&);
};
```

**Status**: 🔄 In Progress

#### 3.2 Implement EncryptionService
**File**: `src/service/EncryptionService.h/cpp`

```cpp
class EncryptionService {
  // AES-256-GCM via OpenSSL
  Result<std::string> encrypt(
    const std::string& plaintext,
    const EncryptionKey& key
  );
  
  Result<std::string> decrypt(
    const std::string& ciphertext,
    const EncryptionKey& key
  );
  
  // Key derivation (PBKDF2)
  EncryptionKey deriveKey(
    const std::string& password,
    const std::string& salt
  );
};
```

**Status**: ❌ Not Started

#### 3.3 Implement SearchService
**File**: `src/service/SearchService.h/cpp`

```cpp
class SearchService {
  std::shared_ptr<INoteRepository> repo;
  
  // Full-text search via inverted index
  std::vector<std::shared_ptr<Note>> search(
    const std::string& query
  );
  
  // Advanced query with filters
  std::vector<std::shared_ptr<Note>> advancedSearch(
    const SearchQuery& query
  );
};
```

**Status**: 🔄 In Progress

#### 3.4 Implement ValidationService
**File**: `src/service/ValidationService.h/cpp`

**Validations**:
- Title: 1-500 chars, no null bytes
- Content: max 10 MB
- Tags: alphanumeric + spaces

**Status**: ❌ Not Started

#### 3.5 Write Service Tests
**Files**: `tests/unit/service/*.cpp`

**Coverage**:
- ✅ CRUD operations (happy path + errors)
- ✅ Encryption roundtrips
- ✅ Search relevance
- ✅ Input validation
- ✅ Error handling

**Status**: ❌ Not Started

### Deliverables
- ✅ NoteService with business logic
- ✅ EncryptionService (AES-256-GCM)
- ✅ SearchService with full-text indexing
- ✅ ValidationService with input sanitization
- ✅ Comprehensive service tests (50+ tests)

### Estimated Effort: 5-7 days

---

## Phase 4: Plugin System (Weeks 7-8)

### Goal
Build extensible plugin architecture.

### Tasks

#### 4.1 Define Plugin Interface
**File**: `src/plugin/IPlugin.h`

```cpp
class IPlugin : public std::enable_shared_from_this<IPlugin> {
public:
  virtual std::string getName() const = 0;
  virtual std::string getVersion() const = 0;
  virtual bool initialize(const IPluginContext* ctx) = 0;
  virtual bool shutdown() = 0;
  virtual std::vector<std::string> getSupportedNoteTypes() const = 0;
  virtual Result<std::string> processNote(const Note&) = 0;
};

class IPluginContext {
public:
  virtual ILogger* getLogger() = 0;
  virtual IRepository* getRepository() = 0;
  virtual IEncryptionService* getEncryption() = 0;
};
```

**Status**: ❌ Not Started

#### 4.2 Implement PluginManager
**File**: `src/plugin/PluginManager.h/cpp`

```cpp
class PluginManager {
  std::unordered_map<std::string, std::shared_ptr<IPlugin>> plugins;
  
  Result<void> loadPlugin(const std::string& path);
  Result<void> unloadPlugin(const std::string& name);
  Result<std::shared_ptr<IPlugin>> getPlugin(const std::string& name);
  std::vector<std::string> listLoadedPlugins() const;
};
```

**Status**: ❌ Not Started

#### 4.3 Implement Built-In Plugins

**TextPlugin** (`src/plugin/builtin/TextPlugin.h/cpp`)
- Plain text + markdown support
- Export to HTML/PDF
- Syntax highlighting

**VoicePlugin** (`src/plugin/builtin/VoicePlugin.h/cpp`)
- Audio recording/playback
- WAV/MP3 codec support
- Transcription integration (optional)

**SecurePlugin** (`src/plugin/builtin/SecurePlugin.h/cpp`)
- Encryption wrapper
- Access control lists
- Audit logging

**Status**: ❌ Not Started

#### 4.4 Plugin Tests
**File**: `tests/unit/plugin/test_plugin_manager.cpp`

**Tests**:
- ✅ Plugin loading/unloading
- ✅ Interface compliance
- ✅ Error handling
- ✅ Plugin context access

**Status**: ❌ Not Started

### Deliverables
- ✅ Plugin interface (stable API)
- ✅ PluginManager (load/unload/list)
- ✅ 3 built-in plugins (Text, Voice, Secure)
- ✅ Plugin tests (20+ tests)
- ✅ Plugin documentation

### Estimated Effort: 5-7 days

---

## Phase 5: User Interface (Weeks 9-11)

### Goal
Implement Qt6-based GUI following MVC pattern.

### Tasks

#### 5.1 Set Up Qt Project
**Files**:
- Update `CMakeLists.txt` to include Qt6 modules
- Create `src/ui/Application.h/cpp` (QApplication wrapper)
- Create `src/ui/CMakeLists.txt` (Qt configuration)

**Status**: 🔄 In Progress

#### 5.2 Implement Main Window
**File**: `src/ui/view/MainWindow.h/cpp`

```cpp
class MainWindow : public QMainWindow {
  Q_OBJECT
  
  std::unique_ptr<NoteListView> listView;
  std::unique_ptr<NoteEditorView> editorView;
  std::unique_ptr<SearchView> searchView;
  
  void createMenuBar();
  void createToolBars();
  void createDockWidgets();
};
```

**Features**:
- Menu bar (File, Edit, View, Help)
- Toolbar (New, Save, Delete, Search)
- Dock widgets (Note list, Tags, Encryption)
- Status bar (Mode, sync status, record duration)

**Status**: ❌ Not Started

#### 5.3 Implement Views
**Files**:
- `src/ui/view/NoteListView.h/cpp` - QTableWidget for notes
- `src/ui/view/NoteEditorView.h/cpp` - QPlainTextEdit + QTextEdit (markdown preview)
- `src/ui/view/SearchView.h/cpp` - Search bar + filters

**Status**: ❌ Not Started

#### 5.4 Implement Controllers
**Files**:
- `src/ui/controller/NoteController.h/cpp` - Handles note operations
- `src/ui/controller/SearchController.h/cpp` - Search logic
- `src/ui/controller/PluginController.h/cpp` - Plugin UI integration

```cpp
class NoteController : public QObject {
  Q_OBJECT
  
  Signal<void(std::shared_ptr<Note>)> noteCreated;
  Signal<void(std::shared_ptr<Note>)> noteUpdated;
  Signal<void(NoteID)> noteDeleted;
  
  void onCreateNoteClicked();
  void onDeleteNoteClicked(const NoteID& id);
  void onNoteDoubleClicked(const NoteID& id);
};
```

**Status**: ❌ Not Started

#### 5.5 Implement Qt Models
**Files**:
- `src/ui/model/NoteListModel.h/cpp` - QAbstractTableModel
- `src/ui/model/SearchResultsModel.h/cpp`

**Status**: ❌ Not Started

#### 5.6 UI Tests (Optional)
**File**: `tests/ui/test_main_window.cpp`

**Status**: ❌ Not Started

### Deliverables
- ✅ Qt6 application framework
- ✅ Main window with menus, toolbars, dock widgets
- ✅ Note list view (table)
- ✅ Note editor (text + markdown preview)
- ✅ Search interface with filters
- ✅ MVC-compliant controllers
- ✅ Basic UI tests (10+ tests)

### Estimated Effort: 7-10 days

---

## Phase 6: Cross-Platform & Optimization (Weeks 12-13)

### Goal
Add platform abstractions, optimize performance, and prepare for release.

### Tasks

#### 6.1 Platform Abstraction Layer
**Files**:
- `src/platform/IPlatform.h` - Platform interface
- `src/platform/windows/WindowsPlatform.h/cpp`
- `src/platform/unix/UnixPlatform.h/cpp`
- `src/platform/macos/MacOSPlatform.h/cpp`

**Abstractions**:
- Data directory (config, database)
- System temp directory
- Plugin search paths
- Native notifications

**Status**: ❌ Not Started

#### 6.2 Performance Optimization
**Profiling Tasks**:
- Profile with valgrind/Instruments
- Identify hot paths
- Optimize database queries
- Reduce memory allocations

**Targets**:
- 10K note operations < 500ms total
- Search < 200ms
- Memory footprint < 200 MB

**Status**: ❌ Not Started

#### 6.3 Security Audit
**Checklist**:
- ✅ Input validation
- ✅ SQL injection prevention (prepared statements)
- ✅ Memory safety (no buffer overflows)
- ✅ Encryption correctness
- ✅ Error message disclosure

**Status**: ❌ Not Started

#### 6.4 Packaging
**Files**:
- `scripts/package.sh/bat` - Create installers
- `docker/Dockerfile` - Docker image
- `CMakeLists.txt` - Install targets

**Outputs**:
- Windows: `.msi` (NSIS)
- macOS: `.dmg` (create-dmg)
- Linux: `.AppImage` (linuxdeployqt)

**Status**: ❌ Not Started

### Deliverables
- ✅ Platform abstraction layer
- ✅ 20%+ performance improvement
- ✅ Security audit report
- ✅ Installation packages for all platforms

### Estimated Effort: 5-7 days

---

## Phase 7: Polish & Documentation (Week 14+)

### Goal
Finalize testing, documentation, and release preparation.

### Tasks

#### 7.1 Comprehensive Testing
**Coverage Targets**:
- Unit tests: 80%+
- Integration tests: 50%+
- Performance: All critical paths

**Additional Tests**:
- Stress tests (100K notes)
- Concurrent access (10 threads)
- Memory leaks (valgrind)
- Crash scenarios

**Status**: 🔄 In Progress

#### 7.2 Documentation
**Files to Create**:
- `docs/USER_GUIDE.md` - User manual
- `docs/PLUGIN_GUIDE.md` - Plugin development
- `docs/API_REFERENCE.md` - API docs (Doxygen)
- `docs/CONTRIBUTING.md` - Contribution guidelines

**Status**: 🔄 In Progress (See ARCHITECTURE.md, PROJECT_STRUCTURE.md)

#### 7.3 Release Preparation
**Checklist**:
- ✅ Version bump (1.0.0)
- ✅ Changelog generation
- ✅ Release notes
- ✅ GitHub/GitLab release page

**Status**: ❌ Not Started

### Deliverables
- ✅ 80%+ test coverage
- ✅ Complete user & developer documentation
- ✅ Release packages + installers
- ✅ Public release (v1.0.0)

---

## Implementation Checklist

### Phase 1: Foundation ✅
- [ ] Refactor CMake build system
- [ ] Implement core utilities (Result, Error, Logger)
- [ ] Enhance Note model with metadata
- [ ] Create NoteCollection container
- [ ] Setup test infrastructure with Google Test

### Phase 2: Persistence ✅
- [ ] Design SQLite schema with indices
- [ ] Implement INoteRepository interface
- [ ] Implement SQLiteNoteRepository
- [ ] Create QueryBuilder DSL
- [ ] Write repository integration tests

### Phase 3: Services ✅
- [ ] Implement NoteService
- [ ] Implement EncryptionService (AES-256-GCM)
- [ ] Implement SearchService with full-text indexing
- [ ] Implement ValidationService
- [ ] Write comprehensive service tests

### Phase 4: Plugins ✅
- [ ] Define IPlugin interface
- [ ] Implement PluginManager
- [ ] Create TextPlugin
- [ ] Create VoicePlugin
- [ ] Create SecurePlugin
- [ ] Write plugin tests

### Phase 5: UI ✅
- [ ] Setup Qt6 project
- [ ] Implement MainWindow
- [ ] Create NoteListView
- [ ] Create NoteEditorView
- [ ] Implement NoteController and other controllers
- [ ] Create Qt models for MVC
- [ ] Write UI tests

### Phase 6: Optimization ✅
- [ ] Platform abstraction layer
- [ ] Performance profiling
- [ ] Security audit
- [ ] Create installers

### Phase 7: Polish ✅
- [ ] Comprehensive testing
- [ ] User & developer documentation
- [ ] Release preparation

---

## File Modification Summary

### New Files to Create
- `src/core/*` (utilities)
- `src/model/*` (domain)
- `src/repository/*` (persistence)
- `src/service/*` (business logic)
- `src/plugin/*` (plugins)
- `src/ui/*` (user interface)
- `src/platform/*` (cross-platform)
- `tests/**/*` (test suite)

### Existing Files to Modify
- `CMakeLists.txt` - Modularize build
- `include/Note.h` - Enhanced model
- `src/Note.cpp` - Implement new methods
- `src/main.cpp` - Wire up application

### Files to Create (Documentation)
- `ARCHITECTURE.md` ✅
- `PROJECT_STRUCTURE.md` ✅
- `docs/DEVELOPMENT.md`
- `docs/CONTRIBUTING.md`
- `docs/PLUGIN_GUIDE.md`

---

## Timeline Summary

| Phase | Duration | Start | End | Effort |
|-------|----------|-------|-----|--------|
| 1: Foundation | 2 weeks | Week 1 | Week 2 | 3-4 days |
| 2: Persistence | 2 weeks | Week 3 | Week 4 | 3-4 days |
| 3: Services | 2 weeks | Week 5 | Week 6 | 3-4 days |
| 4: Plugins | 1.5 weeks | Week 7 | Week 8 | 2-3 days |
| 5: UI | 2 weeks | Week 9 | Week 10 | 5-6 days |
| 6: Optimization | 1.5 weeks | Week 11 | Week 12 | 2-3 days |
| 7: Polish | 1 week | Week 13+ | - | 1-2 days |
| **Total** | **12 weeks** | - | - | **22-30 days** |
| **vs Original** | **14 weeks** | - | - | **40-50 days** |
| **Speedup** | **-2 weeks** | - | - | **-50% effort** |

---

## Quick Start: Run Existing Code

```bash
# Navigate to project
cd d:\Liams\ Files\SANTA\ CLARA\ UNIVERSITY\Graduate\SPRING\ 2026\CSEN\ 296B\AstraNotes-Project

# Create build directory
mkdir build && cd build

# Configure (MSVC on Windows)
cmake ..

# Build
cmake --build . --config Debug

# Run
bin/Debug/AstraNotes.exe
```

---

## Next Immediate Actions

1. **Review Architecture Document** - Understand the overall design
2. **Read Project Structure** - Familiarize with file organization
3. **Start Phase 1**: 
   - Refactor CMakeLists.txt for modularity
   - Create `src/core/` directory
   - Implement `Result<T>` template
4. **Setup Git Branching**: `feature/core-utilities`
5. **Begin Unit Testing**: Setup Google Test integration

---

## Key Success Factors

✅ **Modular Design** - Each layer has single responsibility  
✅ **Interface-Driven** - Depend on abstractions, not implementations  
✅ **RAII Compliance** - Smart pointers + RAII for safety  
✅ **Comprehensive Testing** - 80%+ coverage at all levels  
✅ **Error Handling** - Railway-oriented programming with Result<T>  
✅ **Performance** - Optimized for 10K+ notes  
✅ **Security** - Encryption, validation, access control  
✅ **Documentation** - Architecture, API, user guides  

---

**This roadmap provides a clear path from current foundation to production-grade application. Start with Phase 1 and progress methodically through each phase, ensuring quality and testing at every step.**
