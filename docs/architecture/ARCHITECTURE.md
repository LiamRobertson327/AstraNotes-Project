# AstraNotes: Simplified Modular Architecture
## High-Performance C++23 Note-Taking Application

**Version**: 2.0 (Simplified)  
**Date**: April 2, 2026  
**Status**: Production-Ready Design

> **This is a SIMPLIFIED version of the architecture that maintains all constraints while eliminating unnecessary complexity. 50% fewer components, equal functionality.**

> **After Project Completion Note**: At a high level, this architecture was followed to create AstraNote, however not strictly.  Specific implementation or guidlines were changed in certain layers as development occured.  One example is that `std::expected<T, Error>` was not used for testing which was initially planned for in the Infrastructure layer.  Another example is the naming convention for file strcuture and certain features were also changed.

---

## 1. EXECUTIVE SUMMARY

This document defines a **streamlined, production-grade modular architecture** for AstraNotes—a cross-platform note-taking application supporting 10,000+ notes with extensible plugins, secure storage, and MVC-pattern UI. 

**What Changed from Original**: This is version 2.0, simplified for faster development and easier maintenance without sacrificing any core constraints:

| Aspect | Original | Simplified | Benefit |
|--------|----------|-----------|---------|
| Service classes | 8 | 4 | 50% less code |
| Plugin complexity | Separate build | Dynamic Qt plugin loading | Runtime extensibility |
| Test folders | 7 | 3 | Easier navigation |
| CMakeLists.txt | 10+ | 3-4 | Faster builds |
| Total directories | 50+ | 25 | 50% fewer files |

**Core principles remain unchanged:**
- **Modularity**: 7-layer architecture with clear separation
- **Performance**: Sub-100ms operations for 10,000+ notes
- **Security**: AES-256-GCM encryption, input validation
- **Reliability**: RAII, smart pointers, comprehensive error handling
- **Testability**: Dependency injection, 80%+ coverage

---

## 2. HIGH-LEVEL ARCHITECTURE (Simplified 7 Layers)

The architecture is organized into seven simplified functional layers:

- GUI Layer: Qt6-based MVC views and controllers for user interaction.
- Application Models: Qt model adapters and domain objects mapped to persistent storage.
- Plugin System Layer: runtime-discovered plugins managed by a plugin loader.
- Service Layer: NoteService (CRUD + validation + search), EncryptionService, and CacheManager.
- Persistence Layer: repository interfaces backed by an optimized SQLite implementation.
- Infrastructure Layer: `std::expected<T, Error>` for explicit error handling, logging, configuration, and Qt platform utilities.
- SQLite Database: a WAL-mode storage engine with full-text search and version snapshot support.

**Key Simplifications:**
- ✅ **3 services instead of 8** - Validation and search integrated into NoteService, Backup/Sync/Export removed (handled by plugins)
- ✅ **Dynamic Qt plugin loading** - Discover plugins at runtime with `QPluginLoader`
- ✅ **Qt platform abstractions** - Use `QStandardPaths` and `QSysInfo` instead of custom platform code
- ✅ **3 test folders** - Unit, Integration, Performance (combined with stress tests)
- ✅ **No separate public headers** - Use relative includes within src/

---

## 3. CORE MODULES & RESPONSIBILITIES

### 3.1 MODEL LAYER (Domain)

#### **3.1.1 Note Class Hierarchy**

The domain model includes a base `Note` abstraction with metadata, content, tags, format, and optional encryption metadata. Specialized note types such as text, voice, and secure notes extend the base model with format-specific behavior and storage details.

- `Note`: base implementation containing id, title, timestamps, tags, content, and format metadata.
- `TextNote`: adds support for markdown and rich-text content.
- `VoiceNote`: adds audio payloads, duration, and optional transcription.
- `SecureNote`: stores encrypted payload and access controls.

#### **3.1.2 Persistent Note Storage**

Notes are persisted in SQLite with FTS5 full-text search. The domain layer does not rely on an in-memory `NoteCollection` container for core storage. Instead, notes are loaded and mapped from the database on demand, while caching is managed separately by the service layer.

To optimize performance for large collections (10,000+ notes), the application implements lazy loading: only note IDs, titles, and metadata are loaded initially for listing and browsing. Full note content is loaded on demand when the user clicks on a specific note, reducing memory usage and improving responsiveness.

- The `note` table contains metadata and serialized content.
- FTS5 indexes provide full-text search across titles, body text, and tags.
- The repository retrieves and materializes notes as needed rather than maintaining a full in-memory collection.

#### **3.1.3 NoteFormat and Format Handling**

Notes support multiple content formats such as plain text, markdown, rich text, JSON, and YAML. Format handling is abstracted by an interface that provides serialization, deserialization, and validation for each supported format.

---

### 3.2 PLUGIN SYSTEM

#### **3.2.1 Plugin Interface (Core)**

The Core Application manages the lifeime of all Data Objects created by plugins.  Plugins must provide serialization logic so that the Core can persist data even after a plugin is unloaded.  The plugin interface is defined by an `IPlugin` abstraction with metadata, lifecycle management, supported note and mime types, and operations for processing, importing, and exporting notes. Plugins are discovered at runtime with Qt plugin loading and receive application context through a plugin context interface.

#### **3.2.2 Plugin Loading**
Plugins are discovered at runtime from a configured plugin directory and loaded via `QPluginLoader`. This keeps the core application decoupled from plugin implementations and allows new plugins to be added without recompilation.

#### **3.2.3 Plugin Manager**

The plugin manager is responsible for discovering and loading plugins from a configured directory using `QPluginLoader`, tracking active plugin instances, and exposing loaded plugins to application components. It keeps the core application decoupled from plugin implementations while supporting runtime extensibility.

**Simplified Approach**:
- ✅ Plugins discovered and loaded dynamically via `QPluginLoader`
- ✅ `Q_DECLARE_INTERFACE` enables Qt plugin casting
- ✅ Core application remains platform-agnostic and extensible

#### **3.3.0 Versioning Support**
The repository layer includes a `version_history` table that stores up to two historical snapshots per note. Each snapshot is a blob of the note state, linked by `note_id` and ordered by `timestamp`. When a third snapshot is created, the oldest snapshot for that note is deleted, simplifying rollback logic.  A snapshot is created only when a
note is opened for editing, or manually by the user.


#### **3.3.1 Repository Interface (Generic)**

The repository layer exposes generic persistence operations for data retrieval, search, save, update, and deletion. All repository operations use `std::expected` for explicit error handling.

Specializations such as `INoteRepository` expose note-specific persistence behavior.

#### **3.3.2 SQLite Implementation (Primary)**

The primary persistence implementation uses SQLite with a schema optimized for note metadata, attachments, tags, and secure note audit logging. Performance is achieved through prepared statements, batched transactions, and indices on frequently queried columns.

#### **3.3.3 Query & Filter DSL**

The persistence layer supports composable query objects with predicates, sort order, paging, and filtering. Repositories interpret these query objects into SQLite statements for searches and retrievals.

---

### 3.4 SERVICE LAYER (Simplified to 3 Core Services)

#### **3.4.1 NoteService (Combines CRUD + Validation + Search)**

The NoteService performs note creation, retrieval, update, deletion, and full-text search through the repository interface. It validates titles, content, and tags inline rather than using separate validation or search services.

**Removed**: Separate ValidationService and SearchService (integrated here)

**Removed**: QueryBuilder DSL (simple WHERE clauses only)

#### **3.4.2 EncryptionService (AES-256-GCM Only)**

The EncryptionService provides authenticated AES-256-GCM encryption and decryption for secure note payloads. Key derivation is handled through a simple password-based scheme, with no separate key management service.

**Simplified**: No key management system (simple derivation only)

#### **3.4.3 CacheManager (LRU Caching Only)**

The CacheManager provides an in-memory LRU cache for recently accessed notes. It is designed to reduce repeated database reads while ensuring the repository remains the source of truth. All cache operations use `std::expected` where errors may occur.  The Cache must be invalidated when a Snapshot is restored to prevent showing old data.

**Removed Services:**
- ❌ ExportService (handled by plugins)
- ❌ SyncService (future feature)
- ❌ BackupService (repository responsibility)
- ❌ ValidationService (integrated into NoteService)
- ❌ SearchService (integrated into NoteService)

---

### 3.5 USER INTERFACE LAYER (MVC: View + Controller)

#### **3.5.1 MVC Pattern**

The UI layer follows the MVC paradigm, with view components for windows and dialogs, controllers for user interaction, and Qt model adapters for application state. The model layer is backed by lightweight note summaries (containing ID, title, and metadata) rather than full persisted note objects, enabling lazy loading of content to maintain performance for large note collections.

- Autosave is implemented using a debounced `QTimer` set to 500ms. Each keystroke resets the timer, and persistence is committed only after 500ms of inactivity to avoid excessive disk I/O.

#### **3.5.2 UI Framework Choice**
- **Primary**: Qt 6 (cross-platform, mature, best for desktop)
- **Alternative**: wxWidgets (lighter weight)

Web-based UI using Qt WebEngine for embedded browser features is a future consideration. For now, we are implementing a native executable application.

#### **3.5.3 Events & Signals**

The controller layer uses Qt signals and slots to propagate note lifecycle events such as creation, updating, deletion, and search completion. Controllers expose event notifications to views and services without embedding business logic directly in UI components.

---

### 3.6 ERROR HANDLING STRATEGY

#### **3.6.1 `std::expected` Type (C++23)**

The application uses `std::expected<T, Error>` for all operation results, ensuring that success and failure cases are explicit and handled at each boundary.

#### **3.6.2 Error Hierarchy**

Errors are modeled as a structured hierarchy with codes and messages for data, storage, permission, and system failures. This enables consistent reporting and recovery across services.

#### **3.6.3 Exception Safety**
- **Strong Guarantee**: Operations either complete successfully or leave state unchanged.
- **Nothrow Guarantee**: Destructors and simple accessors remain noexcept.
- **Basic Guarantee**: Objects remain valid after failure.

---

### 3.7 MEMORY MANAGEMENT

#### **3.7.1 RAII Principles**

Resource management follows RAII across file I/O and database connections. Objects own their resources and release them automatically in destructors.

#### **3.7.2 Smart Pointer Policy**
| Use Case | Type | Rationale |
|----------|------|-----------|
| Unique ownership | `std::unique_ptr<T>` | Single owner, move semantics |
| Shared ownership | `std::shared_ptr<T>` | Plugins, services, domain objects |
| Cache/pool | Custom allocator | Pre-allocated, non-owning pointers |
| Event handlers | `std::function<>` | Callback type erasure |

#### **3.7.3 Memory Optimization for 10K+ Notes**

Memory optimization focuses on limiting resident note state, using a bounded LRU cache, and deferring full note materialization until required. This supports 10,000+ notes with minimal memory overhead.

---

### 3.8 TESTING ARCHITECTURE

#### **3.8.1 Test Layers**

The testing architecture includes:
- Unit tests for model, service, repository, and plugin behavior.
- Integration tests for end-to-end workflows, database interactions, plugin loading, and encryption roundtrips.
- Performance tests for large note operations, search performance, memory usage, and cache behavior.
- UI tests for controller behavior, signal/slot chains, and user workflows.

#### **3.8.2 Test Infrastructure**

Test infrastructure is built around fixtures and mockable repository interfaces, enabling both in-memory unit testing and integration scenarios with SQLite-backed persistence.

---

## 4. CROSS-PLATFORM STRATEGIES (Simplified)

### 4.1 Qt Abstractions for Platform-Agnostic Logic

Instead of custom OS-specific files, the application relies on Qt's platform abstraction APIs for data directories, plugin discovery paths, and runtime environment checks. This keeps core logic platform-agnostic and reduces OS-specific branching.

Persistent data is stored in SQLite via `QSqlDatabase` and `QSqlQuery`; the application writes note records into tables rather than filesystem files. `QSaveFile` is not required for core persistence, and is only needed if the application exports separate filesystem artifacts such as backups or user exports.

**Benefit**: Core logic remains platform-agnostic while preserving atomic persistence through SQLite transactions and Qt database APIs across Windows, macOS, and Linux.

### 4.2 Limited Abstractions

Only abstract what's genuinely different:
- Data directory locations via `QStandardPaths`
- Plugin file extensions (.dll, .dylib, .so)
- Runtime environment checks via `QSysInfo`

**Don't abstract**:
- Standard library (same on all platforms with C++23)
- Qt6 (already cross-platform)
- SQLite (already cross-platform)

---

## 5. PERFORMANCE TARGETS

| Operation | Target | Implementation |
|-----------|--------|-----------------|
| Create note | <50ms | Direct DB insert, no full-text index on create |
| List 10K notes | <100ms | Paging (100 items/page), DB query optimized |
| Full-text search | <200ms (10K notes) | Inverted index, TF-IDF ranking |
| Encrypt note | <100ms | Hardware AES (if available) |
| Plugin loading | <500ms | Lazy loading, parallel init |
| UI responsiveness | 60 FPS | Async DB operations, thread pool |

---

## 6. SECURITY ARCHITECTURE

### 6.1 Encryption
- **Algorithm**: AES-256-GCM
- **Key derivation**: Argon2id (for passwords)
- **Random**: `std::random_device` + `std::mt19937`

### 6.2 Input Validation
- Title: max 500 chars, no null bytes
- Content: max 10 MB (configurable)
- Tags: alphanumeric + spaces, max 50 chars each

### 6.3 Access Control

Access control is modeled with permission flags for read, write, delete, and share operations. The security layer evaluates user permissions against note access rules through a centralized access control service.

---

## 7. DEPLOYMENT STRUCTURE

The deployed application is organized into platform-specific executable and plugin directories, shared configuration and resource assets, optional system configuration, and a writable SQLite database.

- `bin/`: application executable
- `lib/plugins/`: runtime plugin modules
- `share/config/`: default settings and plugin registry
- `share/resources/`: icons, themes, and translations
- `etc/`: system-level configuration
- `db/`: user-writable SQLite database

Note: This exact structure was not strictly followed
---

## 8. DEPENDENCY MAP

External dependencies include Qt 6 for UI, SQLite for persistence, OpenSSL for encryption, Google Test for testing, spdlog for logging, nlohmann/json for config serialization, and fmt for string formatting.

Internal module dependencies include the core utilities, domain model, repository layer, service layer, plugin system, UI layer, and test harness.

Note: This exact structure was not strictly followed
---

## 9. BUILD & DEPLOYMENT

### 9.1 CMake Target Structure

The build uses a layered CMake structure with shared libraries for core, service, and UI modules, module plugins for runtime extensions, a main executable, and a test executable. The build configuration exposes unit tests through CTest.

### 9.2 CI/CD Pipeline

The CI/CD pipeline includes build, test, package, and deploy stages for Linux, macOS, and Windows targets. It verifies unit tests, integration tests, and performance benchmarks before packaging platform-specific installers.

---

## 10. IMPLEMENTATION ROADMAP

### Phase 1: Foundation (Weeks 1-2)
- [ ] Set up module structure & CMake
- [ ] Create Note domain model
- [ ] Establish core interfaces and utilities
- [ ] Basic repository abstractions

### Phase 2: Persistence (Weeks 3-4)
- [ ] Implement repository layer (SQLite)
- [ ] Database schema for notes, metadata, tags, versions
- [ ] WAL mode and crash recovery configuration
- [ ] Repository tests for CRUD and versioning

### Phase 3: Services (Weeks 5-6)
- [ ] NoteService with business logic
- [ ] Integrated validation and search in NoteService
- [ ] EncryptionService
- [ ] Unit tests (70% coverage)

### Phase 4: Plugin System (Weeks 7-8)
- [ ] IPlugin interface & manager
- [ ] TextPlugin implementation
- [ ] VoicePlugin skeleton
- [ ] SecurePlugin with encryption

### Phase 5: UI & Integration (Weeks 9-10)
- [ ] Qt GUI framework
- [ ] NoteController & MVC wiring
- [ ] Search & filtering UI
- [ ] Integration tests

### Phase 6: Platform Integration (Weeks 11-12)
- [ ] Performance profiling & optimization
- [ ] Security audit
- [ ] Cross-platform testing
- [ ] User documentation

---

## 11. CONVENTIONS & STANDARDS

### 11.1 Naming
- **Classes**: `PascalCase` (e.g., `NoteService`)
- **Methods**: `camelCase` (e.g., `createNote`)
- **Constants**: `UPPER_SNAKE_CASE`
- **Interfaces**: `I` prefix (e.g., `IRepository`)
- **Namespaces**: lowercase (e.g., `astranotesservice`)

### 11.2 Code Style
- Clang-format: `.clang-format` in repo (LLVM style)
- Line length: 100 chars (hard limit)
- Include guards: `#pragma once`
- Const correctness: enforced throughout

### 11.3 Documentation
- **API Docs**: Doxygen comments on public interfaces
- **Architecture**: Markdown in `/docs/`
- **Examples**: `/examples/` with sample plugins

---

## CONCLUSION

This architecture provides a **scalable, testable, and maintainable foundation** for AstraNotes:

✅ **Modularity**: Plugin system enables unlimited extensibility  
✅ **Performance**: Optimized for 10K+ notes with caching & indexing  
✅ **Security**: Industry-standard encryption + access control  
✅ **Quality**: Comprehensive testing at all levels  
✅ **Cross-Platform**: Abstractions for Windows, macOS, Linux  
✅ **Developer Experience**: Clear interfaces, dependency injection, error handling  

The modular design allows teams to work on plugins, services, and UI independently while maintaining integration points.

---

**For questions or clarifications, refer to the implementation phases and module specifications above.**
