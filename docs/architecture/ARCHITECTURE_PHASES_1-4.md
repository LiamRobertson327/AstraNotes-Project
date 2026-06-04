# AstraNotes Architecture Diagram - Phases 1-4

## Component Interaction Flowchart

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                            MAINWINDOW (UI LAYER)                            │
│                                                                             │
│  ┌────────────────────────────────────────────────────────────────────┐     │
│  │ Phase 1: Note-Type Awareness                                       │     │
│  │ ┌──────────────────┐  setNoteType()  ┌──────────────────────┐      │     │
│  │ │ currentNote      │  ───────────→   │ Hide/Show Mode Buttons│     │     │
│  │ │ currentTypeId    │                 │ (Write/Read/Split)   │      │     │
│  │ └──────────────────┘                 └──────────────────────┘      │     │
│  │                                             │                      │     │
│  │                                             ↓                      │     │
│  │                                  populateFormattingToolbar()       │     │
│  └────────────────────────────────────────────────────────────────────┘     │
│                                           │                                 │
│  ┌────────────────────────────────────────┼─────────────────────────┐       │
│  │                                        ↓                         │       │
│  │  ┌──────────────────────────────────────────────────────────┐    │       │
│  │  │ Phase 3: Formatting Toolbar                              │    │       │
│  │  │                                                          │    │       │
│  │  │  formattingToolbar: QToolBar                             │    │       │
│  │  │  ├── Gets actions from PluginManager                     │    │       │
│  │  │  ├── Connects to IFormattingAction::execute()            │    │       │
│  │  │  └── Focus-aware: targets writeEditor or splitEditor     │    │       │
│  │  └──────────────────────────────────────────────────────────┘    │       │
│  │                                                                  │       │
│  │  Editors:                                                        │       │
│  │  ├── writeEditor: QTextEdit (Write Mode)                         │       │
│  │  ├── readViewer: QTextBrowser (Read Mode)                        │       │
│  │  └── splitEditor/splitPreview: Split Mode                        │       │
│  │                                                                  │       │
│  │  Control: QButtonGroup (exclusive mode selection)                │       │
│  │  ├── btnWrite → viewStack->setCurrentIndex(0)                    │       │
│  │  ├── btnRead  → render markdown, setCurrentIndex(1)              │       │
│  │  └── btnSplit → sync editors, setCurrentIndex(2)                 │       │
│  └───────────────────────────────────┬──────────────────────────────┘       │
│                                      ↓                                      │
│  ┌────────────────────────────────────────────────────────────────────┐     │
│  │ Phase 4: Persistence Layer                                         │     │
│  │                                                                    │     │
│  │  Save Button Clicked                                               │     │
│  │         │                                                          │     │
│  │         ├─→ Create/Update currentNote object                       │     │
│  │         ├─→ Populate title & content from editors                  │     │
│  │         └─→ noteRepository->save(currentNote)                      │     │
│  │                     │                                              │     │
│  │                     └─→ Save Indicator Status Update               │     │
│  │                         (Green=Saved, Red=Error, Pink=Unsaved)     │     │
│  │                         └─→ Reset after 2 seconds                  │     │
│  └────────────────────────────────────────────────────────────────────┘     │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
                                     ↓
          ┌──────────────────────────────────────────────────────────┐
          │           PLUGIN MANAGER (SINGLETON)                     │
          │                                                          │
          │  PluginManager::instance()                               │
          │  ├── plugins: QMap<QString, IPlugin*>                    │
          │  ├── registerPlugin(IPlugin *plugin)                     │
          │  ├── getPlugin(const QString &formatId)                  │
          │  └── availableFormats()                                  │
          │                                                          │
          │  Registered Plugins:                                     │
          │  ├── "plaintext" → PlaintextPlugin                       │
          │  └── "markdown"  → MarkdownPlugin                        │
          │                                                          │
          └───────────────────┬──────────────────────────────────────┘
                              ↓
          ┌──────────────────────────────────────────────────────────┐
          │              PLUGIN LAYER (Phase 2)                      │
          │                                                          │
          │  ┌─────────────────────────────────────────────────────┐ │
          │  │ IPlugin Interface                                   │ │
          │  │ ├── formatId(): QString                             │ │
          │  │ ├── displayName(): QString                          │ │
          │  │ ├── serialize(content): QString                     │ │
          │  │ ├── deserialize(stored): QString                    │ │
          │  │ ├── render(content): QString                        │ │
          │  │ ├── supportsMarkdown(): bool                        │ │
          │  │ ├── supportsPlaintext(): bool                       │ │
          │  │ └── getFormattingActions(): QList<IFormattingAction*> │
          │  └─────────────────────────────────────────────────────┘ │
          │                                                          │
          │  ┌──────────────────────┐  ┌──────────────────────────┐  │
          │  │ PlaintextPlugin      │  │ MarkdownPlugin           │  │
          │  │ ├── formatId: "pt"   │  │ ├── formatId: "md"       │  │
          │  │ ├── No actions       │  │ ├── 8 Formatting Actions │  │
          │  │ └── Text-only        │  │ └── Full markdown        │  │
          │  └──────────────────────┘  └────────┬─────────────────┘  │
          │                                       ↓                  │
          │                        ┌──────────────────────────────┐  │
          │                        │ Formatting Actions (Phase 3) │  │
          │                        │ ├── BoldAction               │  │
          │                        │ ├── ItalicAction             │  │
          │                        │ ├── BulletListAction         │  │
          │                        │ ├── NumberedListAction       │  │
          │                        │ ├── HeadingAction (x3)       │  │
          │                        │ ├── CodeBlockAction          │  │
          │                        │ └── LinkAction               │  │
          │                        │                              │  │
          │                        │ Each implements:             │  │
          │                        │ ├── actionId()               │  │
          │                        │ ├── actionName()             │  │
          │                        │ ├── actionToolTip()          │  │
          │                        │ ├── actionIcon()             │  │
          │                        │ └── execute(QTextEdit*)      │  │
          │                        └──────────────────────────────┘  │
          │                                                          │
          └──────────────────────────────────────────────────────────┘
                                     ↓
          ┌──────────────────────────────────────────────────────────┐
          │            REPOSITORY LAYER (Phase 4)                    │
          │                                                          │
          │  ┌──────────────────────────────────────────────────┐    │
          │  │ INoteRepository (Interface)                      │    │
          │  │ ├── save(Note &note): bool                       │    │
          │  │ ├── getById(qint64 id): Note*                    │    │
          │  │ ├── getAll(): QVector<Note*>                     │    │
          │  │ ├── deleteById(qint64 id): bool                  │    │
          │  │ ├── update(const Note &note): bool               │    │
          │  │ ├── searchByTitle(query): QVector<Note*>         │    │
          │  │ └── searchByContent(query): QVector<Note*>       │    │
          │  └──────────────────────────────────────────────────┘    │
          │                              ↓                           │
          │  ┌──────────────────────────────────────────────────┐    │
          │  │ SqliteNoteRepository                             │    │
          │  │ ├── Constructor: Initialize WAL mode             │    │
          │  │ ├── createTablesIfNeeded()                       │    │
          │  │ ├── Prepared statements (SQL injection safe)     │    │
          │  │ ├── AUTOINCREMENT ids                            │    │
          │  │ ├── Modified timestamp tracking                  │    │
          │  │ └── Index on title for search performance        │    │
          │  └──────────────────────────────────────────────────┘    │
          │                                                          │
          └──────────────────────────────────────────────────────────┘
                                     ↓
          ┌──────────────────────────────────────────────────────────┐
          │              DATABASE LAYER (SQLite)                     │
          │                                                          │
          │  notes.db (WAL Mode)                                     │
          │  │                                                       │
          │  └── Table: notes                                        │
          │      ├── id: INTEGER PRIMARY KEY AUTOINCREMENT           │
          │      ├── typeId: TEXT (plaintext/markdown)               │
          │      ├── title: TEXT                                     │
          │      ├── content: TEXT                                   │
          │      ├── created_at: DATETIME                            │
          │      ├── modified_at: DATETIME                           │
          │      │                                                   │
          │      └── Index: idx_title (ON title)                     │
          │                                                          │
          │  Features:                                               │
          │  ├── WAL mode: Crash recovery guarantee                  │
          │  ├── Prepared statements: No SQL injection               │
          │  ├── Concurrent access: Readers don't block writers      │
          │  └── Auto-incrementing: Unique IDs per note              │
          │                                                          │
          └──────────────────────────────────────────────────────────┘
```

## Data Flow Sequence

### New Note Creation → Save

```
User Types Note
    ↓
Edit in writeEditor (QTextEdit)
    ↓
Click Save Button
    ↓
MainWindow Save Handler:
    ├── Validate title not empty
    ├── Create Note object with currentTypeId
    ├── Set title from titleBar
    ├── Set content from writeEditor
    ├── Call noteRepository->save(note)
    │   ├── Query: INSERT INTO notes (typeId, title, content, modified_at)
    │   ├── Get lastInsertId()
    │   └── Update note.id with auto-generated ID
    ├── Update saveIndicator (green)
    └── Reset to pink after 2 seconds
```

### Load Existing Note (Future Phase 5)

```
noteRepository->getAll()
    ↓
For each row in database:
    ├── Create Note object
    ├── Set id, typeId, title, content
    └── Return QVector<Note*>
    ↓
Populate noteList (QListWidget)
    ↓
User clicks note item
    ↓
Call setNoteType(note->getTypeId())
    ├── Hide/show mode buttons based on type
    ├── Populate formatting toolbar
    └── Adjust UI for note type
    ↓
Load content into writeEditor
    ↓
Display note for editing
```

### Formatting Action Execution

```
User clicks Bold button on toolbar
    ↓
QAction::triggered signal
    ↓
Lambda captures BoldAction instance
    ↓
Determine focused editor:
    ├── writeEditor->hasFocus() ?
    └── splitEditor->hasFocus() ?
    ↓
Call BoldAction->execute(editor)
    ├── Get cursor position
    ├── Get selected text (if any)
    ├── Insert **selected** or ****
    └── Update editor display
    ↓
splitEditor::textChanged signal (if split mode)
    ├── Sync preview with markdown
    ├── Mirror content to writeEditor
    ├── Update display
    └── Ready for next action
```

## Module Dependencies

```
MainWindow (UI)
    ├── depends on: Note (model)
    ├── depends on: PluginManager (plugin registry)
    ├── depends on: PlaintextPlugin
    ├── depends on: MarkdownPlugin
    ├── depends on: SqliteNoteRepository (persistence)
    ├── depends on: INoteRepository (interface)
    └── Qt6 modules: Core, Gui, Widgets, Sql

PluginManager (Singleton)
    └── depends on: IPlugin (interface)

Plugins
    ├── PlaintextPlugin: implements IPlugin
    └── MarkdownPlugin: implements IPlugin
        └── depends on: IFormattingAction (interface)

Formatting Actions
    ├── BoldAction: implements IFormattingAction
    ├── ItalicAction: implements IFormattingAction
    ├── BulletListAction: implements IFormattingAction
    ├── NumberedListAction: implements IFormattingAction
    ├── HeadingAction: implements IFormattingAction
    ├── CodeBlockAction: implements IFormattingAction
    └── LinkAction: implements IFormattingAction

Repository
    ├── SqliteNoteRepository: implements INoteRepository
    ├── depends on: Note (model)
    ├── depends on: Qt6::Sql
    └── depends on: SQLite3 C API
```

## Thread Safety Considerations

- **PluginManager**: Singleton, thread-safe after initialization
- **MainWindow**: UI thread only (Qt event loop)
- **Repository**: Current implementation is single-threaded
  - Future enhancement: Consider QMutex for concurrent access
  - WAL mode provides some protection at database level

## Memory Management

```
MainWindow Constructor:
├── Create plugins → new PlaintextPlugin(this)
│                 → new MarkdownPlugin(this)
├── Create repository → new SqliteNoteRepository()
├── Create toolbar → new QToolBar()
└── Create formatting actions via getFormattingActions()

MainWindow Destructor:
├── Delete repository
├── Delete currentNote
└── Qt parent-child hierarchy handles UI widget cleanup
   (btnWrite, btnRead, btnSplit, editors, toolbar, etc.)

Note: Plugins are owned by PluginManager (no cleanup needed)
      Formatting actions are temporarily allocated in populateFormattingToolbar()
      (could optimize with unique_ptr or shared_ptr in production)
```

---

## Performance Characteristics

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| Save note | O(1) | INSERT/UPDATE with prepared statement |
| Load by ID | O(1) | PRIMARY KEY indexed |
| Get all notes | O(n) | n = number of notes |
| Search by title | O(n*m) | n = notes, m = avg title length (LIKE-based) |
| Switch editor modes | O(1) | Stack widget index change |
| Populate toolbar | O(a) | a = number of formatting actions (8 for markdown) |
| Markdown render | O(n) | n = content length (setMarkdown rendering) |

---

## Extensibility Points

1. **Add New Plugin**: Create class inheriting IPlugin, register in MainWindow constructor
2. **Add Formatting Action**: Implement IFormattingAction, return from plugin->getFormattingActions()
3. **Alternative Storage**: Implement INoteRepository, swap SqliteNoteRepository for alternative
4. **Encryption**: Wrap serialize/deserialize in plugins with encryption layer
5. **Search Backends**: Enhance searchByTitle/Content with FTS5 or Elasticsearch
6. **Export Formats**: Add export plugins (PDF, HTML, etc.)