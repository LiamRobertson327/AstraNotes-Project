# AstraNotes UML Starter Diagrams

**Date**: April 20, 2026  
**Purpose**: Initial UML baseline aligned to `INITIAL_REQUIREMENTS.md`, `REQUIREMENTS_UPDATED.md`, `USER_STORIES.md`, and `ARCHITECTURE.md`.

---

## 1. Use Case Diagram (Core Scope)

```mermaid
flowchart LR
  U[User]
  S[System Scheduler]
  A[Admin/Diagnostic Role]

  UC1((Create Note))
  UC1A((Choose Note Type from Dropdown))
  UC1B((Mark or Unmark Private Note Checkbox))
  UC1C((Encrypt Note if Private))
  UC1D((Edit Note))
  UC2((Manual Save))
  UC3((Auto Save Debounced 500ms))
  UC4((Search In Open Note))
  UC5((Search All Notes by Title))
  UC7((Encrypt or Decrypt Private Note))
  UC8((Delete Note to Trash))
  UC9((Restore Note from Trash))
  UC10((Auto Purge after 14 Days))
  UC11((View Audit Logs))

  U --> UC1
  U --> UC1A
  U --> UC1B
  U --> UC1D
  U --> UC2
  U --> UC3
  U --> UC4
  U --> UC5
  U --> UC7
  U --> UC8
  U --> UC9

  A --> UC11
  S --> UC10
  UC1 --> UC1A
  UC1 --> UC1B
  UC1B --> UC1C
```

---

## 2. Class Diagram (Domain + Services + Persistence)

```mermaid
classDiagram
  class Note {
    -qint64 id
    -QString typeId
    -QString title
    -QString content
    -QDateTime lastModified
    +Note(QString noteTypeId, QString noteTitle)
    +QString getTitle() const
    +qint64 getId() const
    +QString getTypeId() const
    +void setId(qint64 newId)
    +void setTitle(const QString& newTitle)
    +void setContent(const QString& newContent)
    +void setTypeId(const QString& newTypeId)
    +void display() const
  }

  class NoteSnapshot {
    +qint64 snapshotId
    +qint64 noteId
    +Binary serializedPayload
    +QDateTime createdAt
    +QDateTime? expiresAt
  }

  class INoteRepository {
    +save(Note)
    +getById(qint64)
    +searchTitles(QString)
    +moveToTrash(qint64)
    +restoreFromTrash(qint64)
    +purgeExpiredTrash(QDateTime)
    +createSnapshot(qint64)
  }

  class SQLiteNoteRepository {
    +QSqlDatabase db
    +QSqlQuery preparedQueries
  }

  class NoteService {
    +createNote(...)
    +updateNote(...)
    +manualSave(qint64)
    +autoSave(qint64)
    +deleteToTrash(qint64)
    +restore(qint64)
    +searchInOpenNote(QString, Note)
    +searchAllByTitle(QString)
  }

  class EncryptionService {
    +encrypt(AES-256-GCM)
    +decrypt(AES-256-GCM)
    +deriveKey(Argon2id)
  }

  class PluginManager {
    +discoverPlugins()
    +loadPlugins()
    +getFormatHandlers()
  }

  class IPlugin {
    +formatId()
    +serialize(Note)
    +deserialize(Binary)
  }

  Note "1" --> "0..2" NoteSnapshot : has
  NoteService --> INoteRepository
  EncryptionService --> INoteRepository
  PluginManager --> IPlugin
  SQLiteNoteRepository ..|> INoteRepository
  NoteService --> EncryptionService
  NoteService --> PluginManager
```

---

## 3. Sequence Diagram (Edit + Debounced Autosave + Failure Handling)

```mermaid
sequenceDiagram
  actor User
  participant EditorView
  participant NoteController
  participant NoteService
  participant DebounceTimer as QTimer(500ms)
  participant PluginManager
  participant Repo as SQLiteNoteRepository
  participant Logger

  User->>EditorView: Type in note
  EditorView->>NoteController: onTextChanged(noteId)
  NoteController->>DebounceTimer: restart()

  DebounceTimer-->>NoteController: timeout
  NoteController->>NoteService: autoSave(noteId)
  NoteService->>PluginManager: serialize by formatId
  PluginManager-->>NoteService: serialized payload
  NoteService->>Repo: save(note) in transaction

  alt Save succeeds
    Repo-->>NoteService: commit OK
    NoteService-->>NoteController: saved
    NoteController-->>EditorView: clear unsaved indicator
  else Save fails
    Repo-->>NoteService: error
    NoteService->>Logger: log error (redacted)
    NoteService-->>NoteController: failure with recoverable state
    NoteController-->>EditorView: show non-blocking error + keep unsaved indicator
  end
```

---

## 4. State Diagram (Note Lifecycle + Trash Retention)

```mermaid
stateDiagram-v2
  [*] --> Draft
  Draft --> Active: First successful save
  Active --> Active: Edit + Save
  Active --> PrivateActive: Mark private + encrypt
  PrivateActive --> Active: Decrypt view (authorized)
  Active --> Trashed: Delete action
  PrivateActive --> Trashed: Delete action
  Trashed --> Active: Restore within 14 days
  Trashed --> Purged: Auto purge at 14-day retention boundary
  Purged --> [*]
```

---

## 5. Component Diagram (Layered Runtime)

```mermaid
flowchart TB
  subgraph UI[Qt6 UI Layer]
    MainWindow
    NoteEditor
    SearchPanel
    TrashView
  end

  subgraph App[Application Layer]
    NoteController
    SearchController
  end

  subgraph Services[Service Layer]
    NoteService
    EncryptionService
    CacheManager
  end

  subgraph Plugins[Plugin Layer]
    PluginManager
    IPlugin
  end

  subgraph Persistence[Persistence Layer]
    INoteRepository
    SQLiteNoteRepository
  end

  subgraph Infra[Infrastructure]
    AuditLogger
    Config
    ErrorHandling
  end

  subgraph Data[Data Layer]
    SQLiteDB[(SQLite: notes, snapshots, audit_log)]
  end

  MainWindow --> NoteController
  NoteEditor --> NoteController
  SearchPanel --> SearchController
  TrashView --> NoteController

  NoteController --> NoteService
  SearchController --> NoteService

  NoteService --> PluginManager
  NoteService --> EncryptionService
  NoteService --> INoteRepository

  INoteRepository --> SQLiteNoteRepository
  SQLiteNoteRepository --> SQLiteDB

  NoteService --> AuditLogger
  NoteService --> ErrorHandling
  PluginManager --> Config
```

---

## 6. Deployment Diagram (Desktop Runtime)

```mermaid
flowchart TB
  User[User]

  subgraph Device[User Workstation (Windows/macOS/Linux)]
    subgraph Proc[AstraNotes Process]
      EXE[AstraNotes Executable]
      QtUI[Qt6 UI Module]
      NoteSvc[NoteService]
      EncSvc[EncryptionService]
      PluginMgr[PluginManager]
      Repo[SQLiteNoteRepository]
    end

    subgraph FS[Local File System]
      DB[(notes.db\nSQLite WAL Mode)]
      WAL[(notes.db-wal)]
      SHM[(notes.db-shm)]
      PLUGINS[plugins/\nText, Voice, Secure, Custom]
      LOGS[logs/\naudit + app logs]
      CONFIG[config/settings.json]
    end
  end

  User --> QtUI
  QtUI --> NoteSvc
  NoteSvc --> EncSvc
  NoteSvc --> PluginMgr
  NoteSvc --> Repo

  PluginMgr --> PLUGINS
  Repo --> DB
  DB --> WAL
  DB --> SHM
  NoteSvc --> LOGS
  QtUI --> CONFIG
```

Deployment notes:
- This diagram models the current offline-first architecture with all persistence on the user workstation.
- IDs are generated by SQLite (AUTOINCREMENT) and assigned back to model objects after insert.
- Plugins are loaded dynamically from `plugins/` at runtime.

---

## 7. Activity Diagrams (User Workflows)

### 7.1 Create Note (Type Selection + Optional Privacy)

```mermaid
flowchart TD
  A([Start]) --> B[User clicks Create Note]
  B --> C[System shows Note Type dropdown]
  C --> D[User selects typeId]
  D --> E[User enters optional title and content]
  E --> F{Private note checkbox selected?}
  F -- Yes --> G[Request password]
  G --> H[Encrypt content with AES-256-GCM]
  F -- No --> I[Keep content plain]
  H --> J[Persist note in SQLite transaction]
  I --> J
  J --> K[SQLite assigns AUTOINCREMENT id]
  K --> L[Update Note model with DB id]
  L --> M[Clear unsaved indicator]
  M --> N([End])
```

### 7.2 Edit Note + Debounced Autosave

```mermaid
flowchart TD
  A([Start]) --> B[User edits open note]
  B --> C[Mark note as unsaved]
  C --> D{Autosave enabled?}
  D -- Yes --> E[Restart 500ms QTimer debounce]
  E --> F[No input for 500ms]
  F --> G[NoteService autoSave]
  D -- No --> H[Wait for manual save button]
  H --> I[NoteService manualSave]
  G --> J[Persist note in SQLite transaction]
  I --> J
  J --> K{Save success?}
  K -- Yes --> L[Clear unsaved indicator]
  K -- No --> M[Log redacted error + show non-blocking notification]
  M --> N[Keep in-memory edits]
  L --> O([End])
  N --> O
```

### 7.3 Search Within Current Open Note

```mermaid
flowchart TD
  A([Start]) --> B[User types query in in-note search box]
  B --> C{Query empty?}
  C -- Yes --> D[Clear highlights and counters]
  C -- No --> E[Run case-insensitive match against open note buffer]
  E --> F{Matches found?}
  F -- Yes --> G[Highlight matches + update current/total counters]
  F -- No --> H[Show No matches]
  G --> I([End])
  H --> I
  D --> I
```

### 7.4 Search Across All Notes (Title-Only)

```mermaid
flowchart TD
  A([Start]) --> B[User enters global search query]
  B --> C[NoteService searchAllByTitle]
  C --> D[SQLite index lookup on title fields]
  D --> E[Return note ids, titles, metadata only]
  E --> F[Render results list]
  F --> G{User opens a result?}
  G -- Yes --> H[Load full note content on demand]
  G -- No --> I([End])
  H --> I
```

### 7.5 Delete to Trash, Restore, and Auto Purge

```mermaid
flowchart TD
  A([Start]) --> B[User selects one or more notes]
  B --> C[User confirms Delete]
  C --> D[Move notes to Trash]
  D --> E[Set deletedAt timestamp]
  E --> F[Hide from active note list]
  F --> G{Restore requested within 14 days?}
  G -- Yes --> H[Restore note and clear deletedAt]
  G -- No --> I[System scheduler checks retention window]
  I --> J{Older than 14 days?}
  J -- Yes --> K[Permanently purge note and snapshots]
  J -- No --> L[Keep in Trash]
  H --> M([End])
  K --> M
  L --> M
```

---

## 8. Modeling Notes and Next UML Iteration

- Current diagrams assume global search is title-only for scale and memory safety.
- Delete flow models trash retention with auto purge at 14 days.
- Governance controls included in behavior: redacted logging and non-blocking error notifications.
- AI integration is intentionally excluded from UML scope for now.

Next diagram candidates:
1. Detailed ER diagram for notes, tags, snapshots, audit logs
2. Plugin lifecycle activity/sequence diagram (discover, load, fail-safe fallback)
3. Concurrency sequence diagram (same note edited in two windows)
4. Packaging and installer deployment variants per OS
