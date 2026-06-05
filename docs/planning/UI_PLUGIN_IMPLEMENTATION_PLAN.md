# AstraNotes UI Refinement & Plugin Architecture Implementation Guide

**Date**: May 6, 2026  
**Purpose**: Plan and track the implementation of note-type awareness, plugin system, formatting toolbar, and SQLite persistence based on Gemini's UI refinement suggestions.

---

> **After Project Completion Note**: This document provides technical implementation of how different note types can be resolved.  This is not a high-level planning document.  This only needs to be read by developers.

## Overview of New Requirements

This guide addresses four interconnected implementation areas:

1. **Note-Type Awareness**: Dynamic UI visibility based on Note.typeId
2. **Plugin System**: Concrete implementations of text and markdown plugins
3. **Formatting Toolbar**: DLL-based formatting actions via plugin interface
4. **Database Integration**: SQLite persistence using Note fields

---

## 1. Note-Type Awareness Implementation

### Goal
Control editor mode visibility based on the Note's `typeId` field.

### Requirements
- If `typeId == "plaintext"`: Hide "Read" and "Split" buttons, lock to writeEditor
- If `typeId == "markdown"`: Enable all three modes
- Apply this logic when a note is loaded or when typeId is changed

### Implementation Steps

#### Step 1.1: Add Note-Type State to MainWindow
**File**: `src/ui/mainwindow.h`

Add a member variable to track the current note's type:
```cpp
private:
    Note *currentNote;  // Pointer to the currently open note
    QString currentTypeId;  // Cache the current typeId for quick lookup
```

#### Step 1.2: Create a Method to Update UI Based on Note Type
**File**: `src/ui/mainwindow.h`

Add a public method:
```cpp
public:
    void setNoteType(const QString &typeId);  // Updates UI visibility based on typeId
```

**File**: `src/ui/mainwindow.cpp`

Implement the method:
```cpp
void MainWindow::setNoteType(const QString &typeId) {
    currentTypeId = typeId;
    
    if (typeId == "plaintext") {
        // Disable markdown modes
        btnRead->setVisible(false);
        btnSplit->setVisible(false);
        btnRead->setEnabled(false);
        btnSplit->setEnabled(false);
        
        // Lock to write mode
        btnWrite->setChecked(true);
        viewStack->setCurrentIndex(0);  // PAGE 0: Write Mode
        btnWrite->setEnabled(false);  // User cannot switch away
    } 
    else if (typeId == "markdown") {
        // Enable all modes
        btnRead->setVisible(true);
        btnSplit->setVisible(true);
        btnRead->setEnabled(true);
        btnSplit->setEnabled(true);
        btnWrite->setEnabled(true);
    }
    else {
        // Unknown type: default to write mode
        btnWrite->setChecked(true);
        viewStack->setCurrentIndex(0);
        btnRead->setVisible(false);
        btnSplit->setVisible(false);
    }
}
```

#### Step 1.3: Hook UI Actions to Mode Switcher (via QButtonGroup)
**File**: `src/ui/mainwindow.cpp`

In `setupSignals()`, connect the existing QButtonGroup to mode switching:
```cpp
void MainWindow::setupSignals() {
    // Update your existing QButtonGroup connection in Section 6
    connect(modeGroup, &QButtonGroup::buttonClicked, [this](QAbstractButton *button){
        if (button == btnWrite) {
            viewStack->setCurrentIndex(0);
        } else if (button == btnRead) {
            // Render the markdown before showing the view
            readViewer->setMarkdown(writeEditor->toPlainText());
            viewStack->setCurrentIndex(1);
        } else if (button == btnSplit) {
            splitEditor->setPlainText(writeEditor->toPlainText());
            splitPreview->setMarkdown(splitEditor->toPlainText());
            viewStack->setCurrentIndex(2);
        }
    });
    
    // Sync editors when text changes
    connect(writeEditor, &QTextEdit::textChanged, this, [this]() {
        splitEditor->setPlainText(writeEditor->toPlainText());
        splitPreview->setMarkdown(splitEditor->toPlainText());
    });
}
```

---

## 2. Plugin System Implementation

### Goal
Create a plugin interface and concrete implementations for plaintext and markdown handling.

### Architecture

#### Plugin Interface: `IPlugin`
**File**: `src/plugins/IPlugin.h`

```cpp
#ifndef IPLUGIN_H
#define IPLUGIN_H

#include <QString>
#include <QObject>

class IPlugin : public QObject {
    Q_OBJECT

public:
    virtual ~IPlugin() = default;

    // Plugin metadata
    virtual QString formatId() const = 0;        // e.g., "plaintext", "markdown"
    virtual QString displayName() const = 0;     // e.g., "Plain Text", "Markdown"

    // Serialization
    virtual QString serialize(const QString &content) const = 0;     // Convert to storage format
    virtual QString deserialize(const QString &stored) const = 0;    // Convert from storage format

    // Rendering (optional, for display modes)
    virtual QString render(const QString &content) const { return content; }  // Default: no rendering

    // Capability flags
    virtual bool supportsMarkdown() const = 0;
    virtual bool supportsPlaintext() const = 0;
};

#endif // IPLUGIN_H
```

#### PlaintextPlugin
**File**: `src/plugins/PlaintextPlugin.h`

```cpp
#ifndef PLAINTEXTPLUGIN_H
#define PLAINTEXTPLUGIN_H

#include "IPlugin.h"

class PlaintextPlugin : public IPlugin {
    Q_OBJECT

public:
    QString formatId() const override { return "plaintext"; }
    QString displayName() const override { return "Plain Text"; }

    QString serialize(const QString &content) const override { return content; }
    QString deserialize(const QString &stored) const override { return stored; }

    bool supportsMarkdown() const override { return false; }
    bool supportsPlaintext() const override { return true; }
};

#endif // PLAINTEXTPLUGIN_H
```

#### MarkdownPlugin
**File**: `src/plugins/MarkdownPlugin.h`

```cpp
#ifndef MARKDOWNPLUGIN_H
#define MARKDOWNPLUGIN_H

#include "IPlugin.h"

class MarkdownPlugin : public IPlugin {
    Q_OBJECT

public:
    QString formatId() const override { return "markdown"; }
    QString displayName() const override { return "Markdown"; }

    QString serialize(const QString &content) const override { return content; }
    QString deserialize(const QString &stored) const override { return stored; }

    QString render(const QString &content) const override { return content; }  // QTextBrowser handles markdown natively

    bool supportsMarkdown() const override { return true; }
    bool supportsPlaintext() const override { return true; }
};

#endif // MARKDOWNPLUGIN_H
```

#### PluginManager
**File**: `src/plugins/PluginManager.h`

```cpp
#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "IPlugin.h"
#include <QMap>
#include <QString>

class PluginManager {
public:
    static PluginManager &instance();  // Singleton

    void registerPlugin(IPlugin *plugin);
    IPlugin *getPlugin(const QString &formatId) const;
    QStringList availableFormats() const;

private:
    PluginManager() = default;
    QMap<QString, IPlugin*> plugins;
};

#endif // PLUGINMANAGER_H
```

**File**: `src/plugins/PluginManager.cpp`

```cpp
#include "PluginManager.h"

PluginManager &PluginManager::instance() {
    static PluginManager manager;
    return manager;
}

void PluginManager::registerPlugin(IPlugin *plugin) {
    if (plugin) {
        plugins[plugin->formatId()] = plugin;
    }
}

IPlugin *PluginManager::getPlugin(const QString &formatId) const {
    return plugins.value(formatId, nullptr);
}

QStringList PluginManager::availableFormats() const {
    return plugins.keys();
}
```

---

## 3. Formatting Toolbar Implementation

### Goal
Create a plugin-based formatting toolbar that dynamically adds buttons for format-specific actions.

#### FormattingAction Interface
**File**: `src/plugins/IFormattingAction.h`

```cpp
#ifndef IFORMATTINGACTION_H
#define IFORMATTINGACTION_H

#include <QString>
#include <QIcon>

class IFormattingAction {
public:
    virtual ~IFormattingAction() = default;

    virtual QString actionId() const = 0;        // e.g., "insert_bold", "insert_list"
    virtual QString actionName() const = 0;      // e.g., "Bold", "Bullet List"
    virtual QString actionToolTip() const = 0;   // Hover text
    virtual QIcon actionIcon() const = 0;        // Icon for the button

    // Called when the action button is clicked
    // editor: The current QTextEdit instance
    virtual void execute(QTextEdit *editor) = 0;
};

#endif // IFORMATTINGACTION_H
```

#### Extend IPlugin to Support Formatting Actions
**File**: `src/plugins/IPlugin.h` (updated)

```cpp
class IPlugin : public QObject {
    // ... existing methods ...

    // Formatting actions (optional)
    virtual QList<IFormattingAction*> getFormattingActions() const { return {}; }
};
```

#### MarkdownFormattingPlugin Example
**File**: `src/plugins/MarkdownFormattingPlugin.h`

```cpp
#ifndef MARKDOWNFORMATTINGPLUGIN_H
#define MARKDOWNFORMATTINGPLUGIN_H

#include "IFormattingAction.h"
#include <QTextEdit>

class BoldAction : public IFormattingAction {
public:
    QString actionId() const override { return "markdown_bold"; }
    QString actionName() const override { return "Bold"; }
    QString actionToolTip() const override { return "Insert bold text (Ctrl+B)"; }
    QIcon actionIcon() const override { return QIcon(":/icons/bold.png"); }

    void execute(QTextEdit *editor) override {
        if (!editor) return;
        QTextCursor cursor = editor->textCursor();
        QString selected = cursor.selectedText();
        if (!selected.isEmpty()) {
            cursor.insertText("**" + selected + "**");
        } else {
            cursor.insertText("****");
            cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, 2);
        }
        editor->setTextCursor(cursor);
    }
};

class BulletListAction : public IFormattingAction {
public:
    QString actionId() const override { return "markdown_bullet_list"; }
    QString actionName() const override { return "Bullet List"; }
    QString actionToolTip() const override { return "Insert bullet point"; }
    QIcon actionIcon() const override { return QIcon(":/icons/bullet_list.png"); }

    void execute(QTextEdit *editor) override {
        if (!editor) return;
        QTextCursor cursor = editor->textCursor();
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.insertText("- ");
        editor->setTextCursor(cursor);
    }
};

#endif // MARKDOWNFORMATTINGPLUGIN_H
```

#### Add Formatting Toolbar to MainWindow
**File**: `src/ui/mainwindow.h` (updated)

```cpp
private:
    QToolBar *formattingToolbar;

    void populateFormattingToolbar(const QString &typeId);  // New method
```

**File**: `src/ui/mainwindow.cpp` (updated)

In `MainWindow::MainWindow()`, after creating the mode buttons:

```cpp
// Formatting Toolbar
formattingToolbar = new QToolBar("Formatting");
formattingToolbar->setIconSize(QSize(16, 16));
rightTileLayout->insertWidget(0, formattingToolbar);  // Insert above title bar
```

Add method to populate toolbar dynamically:

```cpp
void MainWindow::populateFormattingToolbar(const QString &typeId) {
    // Clear existing actions
    formattingToolbar->clear();

    IPlugin *plugin = PluginManager::instance().getPlugin(typeId);
    if (!plugin) return;

    // Get formatting actions from plugin
    QList<IFormattingAction*> actions = plugin->getFormattingActions();
    for (IFormattingAction *action : actions) {
        QAction *toolbarAction = formattingToolbar->addAction(action->actionIcon(), action->actionName());
        toolbarAction->setToolTip(action->actionToolTip());
        
        connect(toolbarAction, &QAction::triggered, this, [this, action]() {
            // Determine which editor is active and pass it to the action
            QTextEdit *activeEditor = nullptr;
            int currentIndex = viewStack->currentIndex();
            
            if (currentIndex == 0) {
                activeEditor = writeEditor;
            } else if (currentIndex == 2) {
                activeEditor = splitEditor;
            }
            
            if (activeEditor) {
                action->execute(activeEditor);
            }
        });
    }
}
```

Update `setNoteType()` to populate the toolbar:

```cpp
void MainWindow::setNoteType(const QString &typeId) {
    currentTypeId = typeId;
    
    // ... existing visibility logic ...
    
    // Populate formatting toolbar based on type
    populateFormattingToolbar(typeId);
}
```

---

## 4. Database Integration

### Goal
Implement SQLite persistence using the Note model fields.

#### Repository Interface
**File**: `src/repository/INoteRepository.h`

```cpp
#ifndef INOTEREPOSITORY_H
#define INOTEREPOSITORY_H

#include "Note.h"
#include <QVector>

class INoteRepository {
public:
    virtual ~INoteRepository() = default;

    // CRUD operations
    virtual bool save(Note &note) = 0;              // Insert or update; updates note.id on insert
    virtual Note* getById(qint64 id) = 0;           // Returns nullptr if not found
    virtual QVector<Note*> getAll() = 0;            // Get all notes
    virtual bool deleteById(qint64 id) = 0;         // Soft or hard delete
    virtual bool update(const Note &note) = 0;      // Update existing note

    // Search operations
    virtual QVector<Note*> searchByTitle(const QString &query) = 0;
    virtual QVector<Note*> searchByContent(const QString &query) = 0;
};

#endif // INOTEREPOSITORY_H
```

#### SQLite Repository Implementation
**File**: `src/repository/SqliteNoteRepository.h`

```cpp
#ifndef SQLITENOTREPOSITORY_H
#define SQLITENOTREPOSITORY_H

#include "INoteRepository.h"
#include <QSqlDatabase>

class SqliteNoteRepository : public INoteRepository {
public:
    SqliteNoteRepository(const QString &dbPath = "notes.db");
    ~SqliteNoteRepository();

    bool save(Note &note) override;
    Note* getById(qint64 id) override;
    QVector<Note*> getAll() override;
    bool deleteById(qint64 id) override;
    bool update(const Note &note) override;

    QVector<Note*> searchByTitle(const QString &query) override;
    QVector<Note*> searchByContent(const QString &query) override;

private:
    QSqlDatabase db;
    bool initializeDatabase();
    bool createTablesIfNeeded();
};

#endif // SQLITENOTREPOSITORY_H
```

**File**: `src/repository/SqliteNoteRepository.cpp`

```cpp
#include "SqliteNoteRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>

SqliteNoteRepository::SqliteNoteRepository(const QString &dbPath) {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    
    if (!db.open()) {
        qWarning() << "Failed to open database:" << db.lastError().text();
    } else {
        initializeDatabase();
    }
}

SqliteNoteRepository::~SqliteNoteRepository() {
    if (db.isOpen()) {
        db.close();
    }
}

bool SqliteNoteRepository::initializeDatabase() {
    return createTablesIfNeeded();
}

bool SqliteNoteRepository::createTablesIfNeeded() {
    QSqlQuery query;
    
    // Create notes table with WAL mode
    query.exec("PRAGMA journal_mode=WAL;");
    
    QString createTableSql = R"(
        CREATE TABLE IF NOT EXISTS notes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            typeId TEXT NOT NULL DEFAULT 'plaintext',
            title TEXT,
            content TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            modified_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";
    
    if (!query.exec(createTableSql)) {
        qWarning() << "Failed to create notes table:" << query.lastError().text();
        return false;
    }
    
    // Create index on title for faster search
    query.exec("CREATE INDEX IF NOT EXISTS idx_title ON notes(title);");
    
    return true;
}

bool SqliteNoteRepository::save(Note &note) {
    QSqlQuery query;
    
    if (note.getId() == -1) {
        // Insert new note
        query.prepare(R"(
            INSERT INTO notes (typeId, title, content, modified_at)
            VALUES (:typeId, :title, :content, CURRENT_TIMESTAMP)
        )");
    } else {
        // Update existing note
        query.prepare(R"(
            UPDATE notes
            SET typeId = :typeId, title = :title, content = :content, modified_at = CURRENT_TIMESTAMP
            WHERE id = :id
        )");
        query.addBindValue(note.getId(), QSql::In);
    }
    
    query.addBindValue(note.getTypeId());
    query.addBindValue(note.getTitle());
    query.addBindValue(note.getContent());  // Note: You'll need a getter for this
    
    if (!query.exec()) {
        qWarning() << "Failed to save note:" << query.lastError().text();
        return false;
    }
    
    // Update note ID if it was an insert
    if (note.getId() == -1) {
        note.setId(query.lastInsertId().toLongLong());
    }
    
    return true;
}

Note* SqliteNoteRepository::getById(qint64 id) {
    QSqlQuery query;
    query.prepare("SELECT id, typeId, title, content FROM notes WHERE id = :id");
    query.addBindValue(id);
    
    if (!query.exec() || !query.next()) {
        return nullptr;
    }
    
    Note *note = new Note(query.value("typeId").toString(), query.value("title").toString());
    note->setId(query.value("id").toLongLong());
    note->setContent(query.value("content").toString());
    
    return note;
}

QVector<Note*> SqliteNoteRepository::getAll() {
    QVector<Note*> notes;
    QSqlQuery query("SELECT id, typeId, title, content FROM notes ORDER BY modified_at DESC");
    
    while (query.next()) {
        Note *note = new Note(query.value("typeId").toString(), query.value("title").toString());
        note->setId(query.value("id").toLongLong());
        note->setContent(query.value("content").toString());
        notes.append(note);
    }
    
    return notes;
}

bool SqliteNoteRepository::deleteById(qint64 id) {
    QSqlQuery query;
    query.prepare("DELETE FROM notes WHERE id = :id");
    query.addBindValue(id);
    
    return query.exec();
}

bool SqliteNoteRepository::update(const Note &note) {
    QSqlQuery query;
    query.prepare(R"(
        UPDATE notes
        SET typeId = :typeId, title = :title, content = :content, modified_at = CURRENT_TIMESTAMP
        WHERE id = :id
    )");
    query.addBindValue(note.getTypeId());
    query.addBindValue(note.getTitle());
    query.addBindValue(note.getContent());
    query.addBindValue(note.getId());
    
    return query.exec();
}

QVector<Note*> SqliteNoteRepository::searchByTitle(const QString &query_str) {
    QVector<Note*> results;
    QSqlQuery query;
    query.prepare("SELECT id, typeId, title, content FROM notes WHERE title LIKE :query");
    query.addBindValue("%" + query_str + "%");
    
    while (query.next()) {
        Note *note = new Note(query.value("typeId").toString(), query.value("title").toString());
        note->setId(query.value("id").toLongLong());
        note->setContent(query.value("content").toString());
        results.append(note);
    }
    
    return results;
}

QVector<Note*> SqliteNoteRepository::searchByContent(const QString &query_str) {
    QVector<Note*> results;
    QSqlQuery query;
    query.prepare("SELECT id, typeId, title, content FROM notes WHERE content LIKE :query");
    query.addBindValue("%" + query_str + "%");
    
    while (query.next()) {
        Note *note = new Note(query.value("typeId").toString(), query.value("title").toString());
        note->setId(query.value("id").toLongLong());
        note->setContent(query.value("content").toString());
        results.append(note);
    }
    
    return results;
}
```

#### Add SQLite Integration to Note.h
**File**: `src/model/Note.h` (update)

```cpp
public:
    QString getContent() const { return content; }  // Add this getter
```

---

## Implementation Roadmap

### Phase 1: Note-Type Awareness (1-2 hours)
- [ ] Add currentNote and currentTypeId to MainWindow
- [ ] Implement `setNoteType()` method
- [ ] Hook up button click handlers
- [ ] Test plaintext and markdown mode switching

### Phase 2: Plugin System (2-3 hours)
- [ ] Create IPlugin interface
- [ ] Implement PlaintextPlugin
- [ ] Implement MarkdownPlugin
- [ ] Create PluginManager singleton
- [ ] Register plugins at startup

### Phase 3: Formatting Toolbar (2-3 hours)
- [ ] Create IFormattingAction interface
- [ ] Extend IPlugin to support formatting actions
- [ ] Implement BoldAction and BulletListAction
- [ ] Add formattingToolbar to MainWindow
- [ ] Wire toolbar actions to editor

### Phase 4: SQLite Persistence (3-4 hours)
- [ ] Create INoteRepository interface
- [ ] Implement SqliteNoteRepository
- [ ] Add `getContent()` getter to Note
- [ ] Integrate repository into MainWindow
- [ ] Test save/load/search operations

### Phase 5: Integration & Testing (2-3 hours)
- [ ] Wire UI to repository for save/load
- [ ] Test all three editor modes
- [ ] Test note type switching
- [ ] Verify toolbar updates on mode change
- [ ] Test database persistence

---

## Key Design Decisions

1. **Plugin Manager as Singleton**: Easy access from anywhere in the app
2. **IPlugin as QObject**: Allows signals/slots if needed in future
3. **IFormattingAction Separate Interface**: Keeps concerns separated
4. **WAL Mode by Default**: Ensures crash recovery as per requirements
5. **Soft Delete in Phase 2**: Repository prepared for trash/retention logic

---

## Notes for the Next Phase

- Wire the "Save" button to `repository->save(currentNote)`
- Wire the note list to `repository->getAll()`
- Wire the search bar to `repository->searchByTitle()`
- Add signal/slot for syncing note list when notes are saved/deleted
- Plan for snapshot functionality (Phase 2 of sprint plan)

---

## Files to Create

```
src/
├── plugins/
│   ├── IPlugin.h                       [NEW]
│   ├── IFormattingAction.h             [NEW]
│   ├── PluginManager.h                 [NEW]
│   ├── PluginManager.cpp               [NEW]
│   ├── PlaintextPlugin.h               [NEW]
│   ├── MarkdownPlugin.h                [NEW]
│   └── MarkdownFormattingPlugin.h      [NEW]
├── repository/
│   ├── INoteRepository.h               [NEW]
│   ├── SqliteNoteRepository.h          [NEW]
│   └── SqliteNoteRepository.cpp        [NEW]
├── model/
│   └── Note.h                          [UPDATED: add getContent()]
└── ui/
    ├── mainwindow.h                    [UPDATED: add plugin/toolbar support]
    └── mainwindow.cpp                  [UPDATED: implement setNoteType(), populate toolbar]
```

---

## Next Steps

1. **Start Phase 1**: Implement `setNoteType()` and hook button handlers
2. **Create Plugin Structure**: Set up the plugin interface and manager
3. **Test Mode Switching**: Verify plaintext locks UI, markdown enables all modes
4. **Move to Phase 2**: Implement concrete plugins once Phase 1 is working
5. **Add Database**: Implement repository layer for persistence
6. **Integration**: Wire everything together with signals/slots

