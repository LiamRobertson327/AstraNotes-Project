# AstraNotes UML Reference Diagrams

**Date**: June 4, 2026
**Purpose**: Current UML reference aligned to `INITIAL_REQUIREMENTS_REVISED.md`, `USER_STORIES.md`, and `ARCHITECTURE.md`.

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

## 2. Class Diagram (Current Runtime Model)

```mermaid
classDiagram
  class INote {
    <<interface>>
    +noteId() qint64
    +typeId() QString
    +title() QString
    +content() QString
    +createdAt() QDateTime
    +lastModified() QDateTime
    +setNoteId(qint64)
    +setTypeId(QString)
    +setTitle(QString)
    +setContent(QString)
    +setCreatedAt(QDateTime)
    +setLastModified(QDateTime)
    +displayText() QString
  }

  class Note {
    +qint64 m_noteId
    +QString m_typeId
    +QString m_title
    +QString m_content
    +QDateTime m_createdAt
    +QDateTime m_lastModified
    +bool m_isSecured
    +QString m_encryptionSalt
    +QString m_encryptionIv
    +QString m_encryptionTag
  }

  class ISnapshot {
    <<interface>>
    +snapshotId() qint64
    +noteId() qint64
    +title() QString
    +content() QString
    +createdAt() QDateTime
    +setSnapshotId(qint64)
    +setNoteId(qint64)
    +setTitle(QString)
    +setContent(QString)
    +setCreatedAt(QDateTime)
  }

  class Snapshot {
    +qint64 m_snapshotId
    +qint64 m_noteId
    +QString m_title
    +QString m_content
    +QDateTime m_createdAt
    +bool m_isSecured
    +QString m_encryptionSalt
    +QString m_encryptionIv
    +QString m_encryptionTag
  }

  class INoteRepository {
    <<interface>>
    +save(Note)
    +getById(qint64)
    +getById(qint64, QString)
    +getAll()
    +deleteById(qint64)
    +searchByTitle(QString)
    +searchByContent(QString)
    +saveSnapshot(Snapshot)
    +getSnapshotsByNoteId(qint64)
    +getSnapshotById(qint64)
    +trashNote(qint64)
    +restoreNote(qint64)
    +purgeTrashedNotes(int)
    +searchByTitlePaged(QString, int, int)
  }

  class SqliteNoteRepository {
    +QSqlDatabase db
    +save(Note)
    +getAll()
    +searchByTitlePaged(QString, int, int)
    +trashNote(qint64)
    +restoreNote(qint64)
  }

  class INoteService {
    <<interface>>
    +loadNote(qint64, QString)
    +saveNote(Note, QString)
    +createNote(QString, QString)
    +searchByTitlePaged(QString, int, int)
    +trashNote(qint64)
    +loadNoteRobust(qint64, QString)
  }

  class NoteService {
    +kMaxTitleCharacters
    +kMaxContentBytes
    +loadNote(qint64, QString)
    +saveNote(Note, QString)
    +createNote(QString, QString)
    +searchByTitlePaged(QString, int, int)
    +trashNote(qint64)
  }

  class ISnapshotService {
    <<interface>>
    +saveSnapshot(Note, QString)
    +getSnapshotsByNoteId(qint64)
    +getSnapshotById(qint64, QString)
    +deleteSnapshot(qint64)
    +enforceSnapshotLimit(qint64, int)
    +revertToSnapshot(Note, qint64, QString)
  }

  class SnapshotService {
    +saveSnapshot(Note, QString)
    +getSnapshotsByNoteId(qint64)
    +getSnapshotById(qint64, QString)
    +revertToSnapshot(Note, qint64, QString)
  }

  class ITrashService {
    <<interface>>
    +trashNote(qint64)
    +restoreNote(qint64)
    +purgeNote(qint64)
    +purgeOldTrashedNotes(int)
    +getTrashedNotes(int, int)
    +countTrashedNotes()
    +isNoteTrashed(qint64)
  }

  class TrashService {
    +trashNote(qint64)
    +restoreNote(qint64)
    +purgeNote(qint64)
    +getTrashedNotes(int, int)
  }

  class IFormattingAction {
    <<interface>>
    +actionId() QString
    +actionName() QString
    +actionToolTip() QString
    +actionIcon() QIcon
    +execute(QTextEdit*)
  }

  class IPlugin {
    <<interface>>
    +formatId() QString
    +displayName() QString
    +serializeMetadata(INote)
    +serializePayload(INote)
    +deserialize(QVariantMap, QByteArray, INote)
    +supportsMarkdown() bool
    +supportsPlaintext() bool
    +getFormattingActions()
  }

  class PlaintextPlugin {
    +formatId() QString
    +displayName() QString
  }

  class MarkdownPlugin {
    +formatId() QString
    +displayName() QString
  }

  class PluginManager {
    +instance() PluginManager&
    +registerPlugin(IPlugin*)
    +getPlugin(QString) IPlugin*
    +availableFormats() QStringList
  }

  class EncryptionService {
    +encrypt(...)
    +decrypt(...)
    +deriveKey(...)
  }

  class AuditLogger {
    +instance() AuditLogger*
    +install()
    +logFilePath() QString
  }

  class NoteListController {
    +reload()
    +trashSelectedNotes()
    +noteSaved(Note*)
  }

  class AuditLogPanel
  class TrashDialog

  class MainWindow {
    +MainWindow(QWidget*)
    +~MainWindow()
    +saveCurrentNote(bool)
    +showSnapshotHistoryDialog()
    +showTrashDialog()
    +showSettingsDialog()
  }

  Note ..|> INote
  Snapshot ..|> ISnapshot
  SqliteNoteRepository ..|> INoteRepository
  NoteService ..|> INoteService
  SnapshotService ..|> ISnapshotService
  TrashService ..|> ITrashService
  PlaintextPlugin ..|> IPlugin
  MarkdownPlugin ..|> IPlugin

  INoteRepository --> INote
  INoteRepository --> ISnapshot
  INoteService --> INote
  ISnapshotService --> ISnapshot
  ITrashService --> INote
  IPlugin --> INote
  IPlugin --> IFormattingAction

  MainWindow --> INoteRepository
  MainWindow --> INoteService
  MainWindow --> ISnapshotService
  MainWindow --> ITrashService
  MainWindow --> NoteListController
  MainWindow --> AuditLogPanel
  MainWindow --> TrashDialog
  MainWindow --> PluginManager
  MainWindow --> EncryptionService
  MainWindow --> AuditLogger

  NoteListController --> INoteService
  TrashDialog --> ITrashService
  AuditLogPanel --> AuditLogger

  NoteService --> INoteRepository
  SnapshotService --> INoteRepository
  TrashService --> INoteRepository
  SqliteNoteRepository --> EncryptionService
  SqliteNoteRepository --> AuditLogger
  PluginManager --> IPlugin
```

---

## 3. Object Diagram (Representative Runtime Snapshot)

```mermaid
flowchart LR
  mw["mainWindow: MainWindow"]
  nlc["noteListController: NoteListController"]
  auditPanel["auditLogPanel: AuditLogPanel"]
  trashDlg["trashDialog: TrashDialog"]

  noteSvc["noteService: NoteService"]
  snapshotSvc["snapshotService: SnapshotService"]
  trashSvc["trashService: TrashService"]
  repo["noteRepository: SqliteNoteRepository"]
  pluginMgr["pluginManager: PluginManager::instance()"]
  auditLogger["auditLogger: AuditLogger::instance()"]

  plaintext["plaintextPlugin: PlaintextPlugin"]
  markdown["markdownPlugin: MarkdownPlugin"]

  currentNote["currentNote: Note\n(typeId=markdown, secured=false)"]
  snapshotA["snapshotA: Snapshot\n(latest version)"]
  snapshotB["snapshotB: Snapshot\n(previous version)"]

  db[(notes.db / snapshots)]
  log[(audit.log)]

  mw --> nlc
  mw --> auditPanel
  mw --> trashDlg
  mw --> noteSvc
  mw --> snapshotSvc
  mw --> trashSvc
  mw --> pluginMgr
  mw --> auditLogger

  nlc --> noteSvc
  trashDlg --> trashSvc
  auditPanel --> auditLogger

  noteSvc --> repo
  snapshotSvc --> repo
  trashSvc --> repo
  repo --> db

  noteSvc --> currentNote
  snapshotSvc --> snapshotA
  snapshotSvc --> snapshotB
  currentNote --> snapshotA
  currentNote --> snapshotB

  pluginMgr --> plaintext
  pluginMgr --> markdown
  auditLogger --> log
```

---

## 4. Sequence Diagram (Edit + Debounced Autosave + Failure Handling)

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

## 5. State Diagram (Note Lifecycle + Trash Retention)

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

## 6. Component Diagram (Layered Runtime)

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
    SQLiteDB[(SQLite: notes, snapshots)]
    AuditLog[(audit.log in app data directory)]
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
  AuditLogger --> AuditLog
```

---

## 7. Deployment Diagram (Desktop Runtime)

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
      LOGS[(audit.log in app data directory)]
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
- The audit log is written to the Qt app data location, not a repository-local `logs/` folder.

---

## 8. Activity Diagrams (User Workflows)

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

## 9. Modeling Notes and Next UML Iteration

- Treat these diagrams as living documentation. Update them when storage locations, service boundaries, or test coverage shift.
- The component and deployment diagrams are the most likely to drift from implementation details.

- Current diagrams assume global search is title-only for scale and memory safety.
- Delete flow models trash retention with auto purge at 14 days.
- Governance controls included in behavior: redacted logging and non-blocking error notifications.
- AI integration is intentionally excluded from UML scope for now.

Next diagram candidates:
1. Detailed ER diagram for notes, tags, snapshots, audit logs
2. Plugin lifecycle activity/sequence diagram (discover, load, fail-safe fallback)
3. Concurrency sequence diagram (same note edited in two windows)
4. Packaging and installer deployment variants per OS
