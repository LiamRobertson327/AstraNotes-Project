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
- Dynamic plugin discovery via QPluginLoader → Runtime extensibility without recompilation
- Qt platform abstractions (QStandardPaths, QSysInfo) → 100% platform-agnostic codebase
- Fewer CMakeLists.txt files → Faster builds

---

## Phase 1: Foundation (Weeks 1-2)

### Goal
Migrate existing code into the finalized directory structure and set up the top-level CMake file to link modules correctly using C++23.

### Tasks

#### 1.1 Migrate Existing Code to New Folders
**Migration Plan**:
- Move `INote.h` and `IPlugin.h` to `src/api/`
- Move `main.cpp` to `src/app/`
- Move `Note.cpp/h` to `src/model/`
- Move `mainwindow.cpp/h/ui` to `src/ui/`
- Leave `src/plugin/`, `src/repository/`, `src/service/` empty for future implementation

**Status**: 🔄 In Progress

#### 1.2 Set Up Top-Level CMake File
**File**: `CMakeLists.txt`

**Requirements**:
- Set C++23 standard
- Add subdirectories for `src/api/`, `src/app/`, `src/model/`, `src/ui/`
- Link modules correctly (api, model, ui depend on app for main)
- Ensure Qt6 integration
- Prepare for future additions to plugin/, repository/, service/

**Status**: ❌ Not Started

#### 1.3 Verify Build and Linking
**Tasks**:
- Build the migrated code
- Ensure all includes and dependencies resolve correctly
- Test basic application launch

**Status**: ❌ Not Started

### Deliverables (Phase 1)
- ✅ Code migrated to new directory structure
- ✅ Top-level CMake file configured for C++23 and module linking
- ✅ Successful build and basic application functionality

### Effort: 2-3 days

---

## Phase 2: Persistence Layer (Weeks 3-4)

### Goal
Implement database access layer optimized for 10K+ notes.

### Tasks

#### 2.1 Design Database Schema
**File**: `src/repository/schema/schema.sql`

The schema is designed for note metadata, encrypted content, note tags, version snapshots, and audit logging. Full-text search uses SQLite FTS5 indexes for fast title, body, and tag queries. Audit logs capture secure note actions and user events.

**Status**: ✅ Designed

#### 2.2 Implement Repository Interface
**File**: `src/repository/INoteRepository.h`

The repository interface exposes generic persistence operations for retrieval, listing, saves, updates, deletion, and counts. All persistence APIs are designed to return `std::expected` for explicit error handling.

**Status**: 🔄 In Progress

#### 2.3 Implement SQLiteNoteRepository
**Files**:
- `src/repository/SQLiteNoteRepository.h`
- `src/repository/SQLiteNoteRepository.cpp`

**Key Features**:
- Multi-threaded access and connection pooling
- Prepared statements to avoid SQL injection
- Batch operations for transaction grouping
- Paging support for large result sets

The SQLite repository implements the note persistence layer with prepared statements and explicit error propagation through `std::expected`. It is optimized for the note schema, indexing, and snapshots.

**Performance Notes**:
- Creates indices on frequently queried columns
- Uses WAL mode for concurrent reads
- Batches writes in transactions

**Status**: 🔄 In Progress

#### 2.4 Implement Two-Snapshot Rotation Logic
**File**: `src/repository/VersionHistoryManager.h/cpp`

Implement the version history snapshot rotation system that maintains up to two historical snapshots per note in the version_history table. When a third snapshot is created, the oldest snapshot for that note is automatically deleted.

**Requirements**:
- Store snapshots as blobs linked by note_id and ordered by timestamp
- Enforce maximum two snapshots per note
- Automatic cleanup of oldest snapshot when limit exceeded
- Snapshot creation only when note is opened for editing or manually by user

**Status**: ❌ Not Started

#### 2.5 Test SQLite Layer
**File**: `tests/integration/test_sqlite_repository.cpp`

**Tests**:
- ✅ Single note CRUD
- ✅ Batch inserts (1000 notes)
- ✅ Tag indexing
- ✅ Query performance (< 100ms for 10K notes)
- ✅ Concurrent access (thread safety)
- ✅ Two-snapshot rotation behavior

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

NoteService coordinates repository access, encryption, and business logic. It should use `std::expected<T, Error>` for all public operations.

**Sub-Task 3.1a: Integrate Inline Validation Logic**
Validation logic is integrated directly into NoteService rather than implemented as a separate `ValidationService`. This keeps the service layer simpler while still enforcing title, content, and tag rules.

Validations to enforce:
- Title: 1-500 chars, no null bytes
- Content: max 10 MB
- Tags: alphanumeric + spaces

**Status**: 🔄 In Progress

#### 3.2 Implement EncryptionService
**File**: `src/service/EncryptionService.h/cpp`

The encryption service provides AES-256-GCM encryption and decryption, along with password-based key derivation for secure notes. All encryption APIs return `std::expected` to capture errors explicitly.

**Status**: ❌ Not Started

#### 3.3 Implement SearchService
**File**: `src/service/SearchService.h/cpp`

The SearchService coordinates full-text note search through SQLite FTS5 and filters. It exposes search operations that accept query criteria and return results using `std::expected` on error.

**Status**: 🔄 In Progress

#### 3.4 Write Service Tests
**Files**: `tests/unit/service/*.cpp`

**Coverage**:
- ✅ CRUD operations (happy path + errors)
- ✅ Encryption roundtrips
- ✅ Search relevance
- ✅ Input validation
- ✅ Error handling

**Status**: ❌ Not Started

### Deliverables
- ✅ NoteService with business logic and inline validation
- ✅ EncryptionService (AES-256-GCM)
- ✅ SearchService with full-text indexing
- ✅ Comprehensive service tests (50+ tests)

### Estimated Effort: 5-7 days

---

## Phase 4: Plugin System (Weeks 7-8)

### Goal
Build extensible plugin architecture.

### Tasks

#### 4.1 Define Plugin Interface
**File**: `src/plugin/IPlugin.h`

Define the plugin interface with metadata, lifecycle hooks, supported note types, and operations for processing notes. Plugin APIs should return `std::expected` for error handling, and the plugin context provides logger and repository access.

**Status**: ❌ Not Started

#### 4.2 Implement PluginManager
**File**: `src/plugin/PluginManager.h/cpp`

The plugin manager discovers and loads plugins at runtime using Qt's plugin loader. It tracks loaded plugin instances, exposes them to application components, and normalizes plugin lifecycle operations.

**Status**: ❌ Not Started

#### 4.3 Implement Built-In Plugins (Via QPluginLoader)

Built-in plugins are discovered and loaded dynamically at runtime using QPluginLoader, keeping the core application decoupled from plugin implementations.

**TextPlugin** (`src/plugin/builtin/TextPlugin.h/cpp`)
- Plain text and markdown support
- Export to HTML/PDF
- Syntax highlighting

**VoicePlugin** (`src/plugin/builtin/VoicePlugin.h/cpp`)
- Audio recording and playback
- WAV/MP3 codec support
- Transcription integration (optional)

**SecurePlugin** (`src/plugin/builtin/SecurePlugin.h/cpp`)
- Encryption wrapper for secure notes
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

The main window wraps the Qt application and composes the note list, editor, and search views. It exposes menu, toolbar, and dock widget structures for note workflows.

**Features**:
- Menu bar with File, Edit, View, and Help sections
- Toolbar for new note, save, delete, and search actions
- Dock widgets for note list, tag navigation, and encryption controls
- Status bar for mode, sync state, and operation feedback

**Status**: ❌ Not Started

#### 5.3 Implement Views
**Files**:
- `src/ui/view/NoteListView.h/cpp` - note listing and selection
- `src/ui/view/NoteEditorView.h/cpp` - editor with markdown preview
- `src/ui/view/SearchView.h/cpp` - search controls and filters

**Status**: ❌ Not Started

#### 5.4 Implement Controllers
**Files**:
- `src/ui/controller/NoteController.h/cpp` - note operations
- `src/ui/controller/SearchController.h/cpp` - search orchestration
- `src/ui/controller/PluginController.h/cpp` - plugin UI integration

Controllers use Qt signals and slots to communicate user actions from the UI to services and plugins without embedding business logic directly in view classes.

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

## Phase 6: Platform Integration & Optimization (Weeks 12-13)

### Goal
Integrate Qt platform abstractions for 100% platform-agnostic codebase and optimize performance.

### Tasks

#### 6.1 Platform Integration via Qt Abstractions
**Integration Points**:
- Data directory locations via `QStandardPaths::AppDataLocation` and `QStandardPaths::AppLocalDataLocation`
- System temp directory via `QStandardPaths::TempLocation`
- Plugin search paths via `QStandardPaths::ApplicationsLocation` and dynamic plugin directory
- Runtime environment checks via `QSysInfo::productType()`, `QSysInfo::kernelVersion()`, `QSysInfo::buildCpuArchitecture()`
- File operations using `QSaveFile` for atomic writes and safe edits across all platforms

**Design Principle**:
No OS-specific code folders. All platform logic is unified through Qt abstractions, ensuring a 100% platform-agnostic codebase. Platform differences are handled transparently by Qt, not by custom conditional compilation.
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
- ✅ Unified platform integration via Qt abstractions (QStandardPaths, QSysInfo, QSaveFile)
- ✅ 100% platform-agnostic codebase with no OS-specific source folders
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
- [ ] Implement core utilities (std::expected, Error, Logger)
- [ ] Enhance Note model with metadata
- [ ] Create SQLite-backed persistence adapter
- [ ] Setup test infrastructure with Google Test

### Phase 2: Persistence ✅
- [ ] Design SQLite schema with indices
- [ ] Implement INoteRepository interface
- [ ] Implement SQLiteNoteRepository
- [ ] Create QueryBuilder DSL
- [ ] Implement two-snapshot rotation logic for version history
- [ ] Write repository integration tests

### Phase 3: Services ✅
- [ ] Implement NoteService with inline validation
- [ ] Implement EncryptionService (AES-256-GCM)
- [ ] Implement SearchService with full-text indexing
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

To run the current project, open the repository in your development environment, create a `build` directory, configure CMake, build the project, and launch the resulting executable.

---

## Next Immediate Actions

1. **Review Architecture Document** - Understand the overall design
2. **Read Project Structure** - Familiarize with file organization
3. **Start Phase 1**: 
   - Refactor CMakeLists.txt for modularity
   - Create `src/core/` directory
   - Design std::expected-based error handling
4. **Setup Git Branching**: `feature/core-utilities`
5. **Begin Unit Testing**: Setup Google Test integration

---

## Key Success Factors

✅ **Modular Design** - Each layer has single responsibility  
✅ **Interface-Driven** - Depend on abstractions, not implementations  
✅ **RAII Compliance** - Smart pointers + RAII for safety  
✅ **Comprehensive Testing** - 80%+ coverage at all levels  
✅ **Error Handling** - Railway-oriented programming with std::expected  
✅ **Performance** - Optimized for 10K+ notes  
✅ **Security** - Encryption, validation, access control  
✅ **Documentation** - Architecture, API, user guides  

---

**This roadmap provides a clear path from current foundation to production-grade application. Start with Phase 1 and progress methodically through each phase, ensuring quality and testing at every step.**
