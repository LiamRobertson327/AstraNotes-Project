#include "MainWindow.h"

#include "../plugins/PluginManager.h"
#include "../plugins/PlaintextPlugin.h"
#include "../plugins/MarkdownPlugin.h"
#include "../model/Note.h"
#include "../repository/SqliteNoteRepository.h"
#include <QTimer>
#include <QDebug>
#include <QAction>
#include <QDir>
#include <QStandardPaths>
#include <QApplication>
#include <QMenu>
#include "../plugins/IFormattingAction.h"

void MainWindow::setNoteType(const QString &typeId) {
    currentTypeId = typeId;
    
    // Phase 1: Lock/unlock modes based on note type
    if (typeId == "plaintext") {
        // Plain text: only Write mode, hide Read and Split
        btnRead->hide();
        btnSplit->hide();
        btnWrite->show();
        btnWrite->setChecked(true);
        viewStack->setCurrentIndex(0);
    } else if (typeId == "markdown") {
        // Markdown: all modes available
        btnRead->show();
        btnSplit->show();
        btnWrite->show();
        btnWrite->setChecked(true);
        viewStack->setCurrentIndex(0);
    }
    
    // Phase 3: Populate formatting toolbar based on plugin
    populateFormattingToolbar(typeId);
}

void MainWindow::populateFormattingToolbar(const QString &typeId) {
    if (!formattingToolbar) {
        qWarning() << "[MainWindow] CRITICAL: formattingToolbar is null!";
        return;
    }
    // Clear existing actions safely
    QList<QAction*> oldActions = formattingToolbar->actions();
    for (QAction *a : oldActions) {
        formattingToolbar->removeAction(a);
        delete a;
    }
    
    // Phase 3: Get formatting actions from plugin via PluginManager
    IPlugin *plugin = PluginManager::instance().getPlugin(typeId);
    if (!plugin) {
        formattingToolbar->setVisible(false);
        return;
    }
    
    QList<IFormattingAction*> actions = plugin->getFormattingActions();
    if (actions.isEmpty()) {
        formattingToolbar->setVisible(false);
        return;
    }
    
    formattingToolbar->setVisible(true);
    
    // Add each formatting action to the toolbar
    for (IFormattingAction *action : actions) {
        QAction *toolbarAction = new QAction(action->actionName(), formattingToolbar);
        toolbarAction->setToolTip(action->actionToolTip());
        
        // Connect toolbar action to format execution
        // The action is stored in a way that connects to the current editor
        connect(toolbarAction, &QAction::triggered, [this, action](){
            // Determine which editor has focus
            QTextEdit *editor = nullptr;
            if (writeEditor && writeEditor->hasFocus()) {
                editor = writeEditor;
            } else if (splitEditor && splitEditor->hasFocus()) {
                editor = splitEditor;
            }
            
            if (editor && action) {
                action->execute(editor);
            } else if (!action) {
                qWarning() << "[MainWindow] Toolbar action is null!";
            }
        });
        
        formattingToolbar->addAction(toolbarAction);
    }
}

void MainWindow::loadNotesFromDatabase() {
    if (!noteRepository) {
        qWarning() << "[MainWindow::loadNotesFromDatabase] noteRepository is null!";
        return;
    }
    
    qDebug() << "[MainWindow::loadNotesFromDatabase] Loading all notes from database";
    
    // Get all notes from repository
    QVector<Note*> allNotes = noteRepository->getAll();
    qDebug() << "[MainWindow::loadNotesFromDatabase] Found" << allNotes.count() << "notes";
    
    // Populate the note list widget
    for (Note *note : allNotes) {
        if (note) {
            QString displayText = note->title();
            if (displayText.isEmpty()) {
                displayText = "(Untitled)";
            }
            
            // Add creation date to the display text
            QString createdDate = note->createdAt().toString("MMM dd, yyyy");
            displayText = displayText + "\n" + createdDate;
            
            QListWidgetItem *item = new QListWidgetItem(displayText, noteList);
            // Store the note identifier using the INote API (`noteId()`)
            item->setData(Qt::UserRole, note->noteId());
            
            qDebug() << "[MainWindow::loadNotesFromDatabase] Added note:" << note->noteId() << "-" << displayText;
            delete note;  // We don't need to keep the Note object; we just stored the ID
        }
    }
}

void MainWindow::loadNoteIntoEditor(qint64 noteId) {
    if (!noteRepository) {
        qWarning() << "[MainWindow::loadNoteIntoEditor] noteRepository is null!";
        return;
    }
    
    qDebug() << "[MainWindow::loadNoteIntoEditor] Loading note with ID:" << noteId;
    
    // Retrieve the note from database
    Note *note = noteRepository->getById(noteId);
    if (!note) {
        qWarning() << "[MainWindow::loadNoteIntoEditor] Failed to load note with ID:" << noteId;
        return;
    }
    
    // Update current note
    if (currentNote) {
        delete currentNote;
    }
    currentNote = note;
    
    // Populate UI with note data
    titleBar->setText(note->title());
    writeEditor->setText(note->content());
    readViewer->setText(note->content());
    splitEditor->setText(note->content());

    // Set the note type to show/hide appropriate buttons
    setNoteType(note->typeId());
    
    // Clear unsaved indicator
    saveIndicator->setText("● Saved");
    saveIndicator->setStyleSheet("color: #4CAF50;");
    
    qDebug() << "[MainWindow::loadNoteIntoEditor] Note loaded successfully";
}

void MainWindow::createNewNote(const QString &typeId) {
    qDebug() << "[MainWindow::createNewNote] Creating new note of type:" << typeId;
    
    // Delete the current note if it exists
    if (currentNote) {
        delete currentNote;
    }
    
    // Create a new note with the selected type
    currentNote = new Note(typeId, "");
    
    // Clear the UI
    titleBar->clear();
    titleBar->setPlaceholderText("Enter Note Title...");
    titleBar->setFocus();
    
    writeEditor->clear();
    readViewer->clear();
    splitEditor->clear();
    splitPreview->clear();
    
    // Set the note type to show/hide appropriate buttons and populate toolbar
    setNoteType(typeId);
    
    // Reset the save indicator
    saveIndicator->setText("● New Note");
    saveIndicator->setStyleSheet("color: #2196F3;");  // Blue for new notes
    
    qDebug() << "[MainWindow::createNewNote] New note created and UI cleared";
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
        // Initialize Phase 1 members
        currentNote = nullptr;
        currentTypeId = "plaintext";
    
        // Phase 2: Register plugins with PluginManager
        PluginManager &pluginMgr = PluginManager::instance();
        pluginMgr.registerPlugin(new PlaintextPlugin(this));
        pluginMgr.registerPlugin(new MarkdownPlugin(this));
    
        // Phase 4: Initialize SQLite repository for persistence
    // Store database file in the same directory as the executable
    QString appDir = QApplication::applicationDirPath();
    QString dbPath = appDir + "/notes.db";
    
    qDebug() << "[MainWindow] Database path:" << dbPath;
    
        noteRepository = new SqliteNoteRepository(dbPath);
        if (!noteRepository->isConnected()) {
            qWarning() << "[MainWindow] Database connection failed!";
        } else {
            qDebug() << "[MainWindow] Database connected successfully";
        }
    
    // 1. Initialize Central Widget
    centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);
    this->setMinimumSize(1000, 700);
    this->setWindowTitle("AstraNotes");

    // 2. ROOT LAYOUT (The main vertical container)
    QVBoxLayout *rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(15, 10, 15, 15);
    rootLayout->setSpacing(15);

    // --- HEADER SECTION ---
    QHBoxLayout *headerLayout = new QHBoxLayout();
    searchBar = new QLineEdit();
    searchBar->setObjectName("searchBar");
    searchBar->setPlaceholderText("Search notes...");
    
    newButton = new QPushButton("+ New Note");
    newButton->setObjectName("newButton");
    
    saveIndicator = new QLabel("● Unsaved");
    saveIndicator->setObjectName("saveIndicator"); // Must match the CSS #saveIndicator
    
    saveButton = new QPushButton("Save");

    headerLayout->addWidget(searchBar, 4);
    headerLayout->addWidget(newButton);
    headerLayout->addWidget(saveIndicator);
    headerLayout->addWidget(saveButton);
    rootLayout->addLayout(headerLayout); // RootLayout is defined, so this works!

    // --- CONTENT SECTION (Horizontal) ---
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(15);

    // 3. LEFT TILE (Saved Notes)
    QFrame *leftTile = new QFrame();
    leftTile->setObjectName("leftTile");
    leftTile->setMinimumWidth(200);
    QVBoxLayout *leftTileLayout = new QVBoxLayout(leftTile);
    leftTileLayout->setContentsMargins(10, 15, 10, 10);

    listTitle = new QLabel("Saved Notes");
    listTitle->setObjectName("listTitle");
    
    noteList = new QListWidget();
    noteList->setObjectName("noteList");
    noteList->setSpacing(2);

    leftTileLayout->addWidget(listTitle);
    leftTileLayout->addWidget(noteList);

    // 4. RIGHT TILE (Editor Area)
    QFrame *rightTile = new QFrame();
    rightTile->setObjectName("rightTile");
    QVBoxLayout *rightTileLayout = new QVBoxLayout(rightTile);
    rightTileLayout->setContentsMargins(0, 0, 0, 0);
    rightTileLayout->setSpacing(0);

    titleBar = new QLineEdit();
    titleBar->setObjectName("titleBar");
    titleBar->setPlaceholderText("Enter Note Title...");
    titleBar->setFixedHeight(60);
    rightTileLayout->addWidget(titleBar);
    
    // Phase 3: Add formatting toolbar (initially empty, will populate when note is loaded)
    formattingToolbar = new QToolBar("Formatting");
    rightTileLayout->addWidget(formattingToolbar);

    // Mode Buttons
    btnWrite = new QPushButton("Write");
    btnRead = new QPushButton("Read");
    btnSplit = new QPushButton("Split");

    // Make them checkable (toggleable)
    btnWrite->setCheckable(true);
    btnRead->setCheckable(true);
    btnSplit->setCheckable(true);
    btnWrite->setChecked(true); // Set the default active button

    // Set a fixed width so they aren't "long"
    btnWrite->setFixedWidth(80);
    btnRead->setFixedWidth(80);
    btnSplit->setFixedWidth(80);

    // 2. Setup the Button Logic
    QButtonGroup *modeGroup = new QButtonGroup(this);
    modeGroup->addButton(btnWrite);
    modeGroup->addButton(btnRead);
    modeGroup->addButton(btnSplit);
    modeGroup->setExclusive(true); // This ensures only one is purple at a time
    
    // 3. Setup the Layout
    QHBoxLayout *modeLayout = new QHBoxLayout();
    modeLayout->setAlignment(Qt::AlignLeft); 
    modeLayout->setContentsMargins(15, 5, 15, 5); // Add some breathing room
    modeLayout->setSpacing(15); // Space between buttons

    modeLayout->addWidget(btnWrite);
    modeLayout->addWidget(btnRead);
    modeLayout->addWidget(btnSplit);
    modeLayout->addStretch(); // Move stretch to the END to push buttons to the left

    QWidget *modeContainer = new QWidget();
    modeContainer->setObjectName("modeContainer");
    modeContainer->setLayout(modeLayout);

    // 5. Add the container to the Right Title
    rightTileLayout->addWidget(modeContainer);

   
    // The Stack
    viewStack = new QStackedWidget();
    
    writeEditor = new QTextEdit();
    viewStack->addWidget(writeEditor);

    readViewer = new QTextBrowser();
    viewStack->addWidget(readViewer);

    // --- PAGE 2: SPLIT (With Vertical Line) ---
    splitWidget = new QWidget();
    QHBoxLayout *splitLayout = new QHBoxLayout(splitWidget);
    splitLayout->setContentsMargins(0,0,0,0);
    splitLayout->setSpacing(0); // No gap between editors and the line

    splitEditor = new QTextEdit();
    splitEditor->setFrameStyle(QFrame::NoFrame); // Clean look

    // The Vertical Line
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Plain);
    line->setFixedWidth(1);
    line->setStyleSheet("background-color: #CE93D8;"); // Light purple divider

    splitPreview = new QTextBrowser();
    splitPreview->setFrameStyle(QFrame::NoFrame);

    splitLayout->addWidget(splitEditor);
    splitLayout->addWidget(line);
    splitLayout->addWidget(splitPreview);
    
    viewStack->addWidget(splitWidget);
    rightTileLayout->addWidget(viewStack);

    // 5. ASSEMBLE CONTENT
    // Now that leftTile and rightTile are defined, we add them to contentLayout
    contentLayout->addWidget(leftTile, 1);
    contentLayout->addWidget(rightTile, 3);
    
    // Add the contentLayout to the rootLayout
    rootLayout->addLayout(contentLayout);

    // 6. LOGIC / SIGNALS
    connect(modeGroup, &QButtonGroup::buttonClicked, [this](QAbstractButton *button){
        if (button == btnWrite) {
            viewStack->setCurrentIndex(0);
        } else if (button == btnRead) {
            readViewer->setMarkdown(writeEditor->toPlainText());
            viewStack->setCurrentIndex(1);
        } else if (button == btnSplit) {
            splitEditor->setPlainText(writeEditor->toPlainText());
            viewStack->setCurrentIndex(2);
        }
    });

    connect(splitEditor, &QTextEdit::textChanged, [this](){
        splitPreview->setMarkdown(splitEditor->toPlainText());
        // Mirror back to writeEditor so content isn't lost when switching modes
        writeEditor->setPlainText(splitEditor->toPlainText());
    });

    // Phase 6: Wire New Note button to show type selection menu
    connect(newButton, &QPushButton::clicked, [this](){
        QMenu *typeMenu = new QMenu(this);
        
        // Style the menu to match the application theme
        typeMenu->setStyleSheet(
            "QMenu { "
            "   background-color: #E1BEE7; "
            "   border: 1px solid #CE93D8; "
            "   border-radius: 8px; "
            "   padding: 5px 0px; "
            "}"
            "QMenu::item { "
            "   background-color: transparent; "
            "   color: #4A148C; "
            "   padding: 8px 25px; "
            "   font-weight: bold; "
            "}"
            "QMenu::item:selected { "
            "   background-color: #CE93D8; "
            "   color: white; "
            "   border-radius: 4px; "
            "   margin: 2px 5px; "
            "}"
        );
        
        // Get available formats from PluginManager
        QStringList formats = PluginManager::instance().availableFormats();
        
        for (const QString &format : formats) {
            IPlugin *plugin = PluginManager::instance().getPlugin(format);
            if (plugin) {
                QString displayName = plugin->displayName();
                QAction *action = typeMenu->addAction(displayName);
                
                connect(action, &QAction::triggered, [this, format](){
                    createNewNote(format);
                });
            }
        }
        
        // Show menu at button location
        typeMenu->exec(newButton->mapToGlobal(QPoint(0, newButton->height())));
    });
    
    // Phase 4: Wire Save button to repository persistence
    connect(saveButton, &QPushButton::clicked, [this](){
        if (!noteRepository) {
            qWarning() << "noteRepository is null";
            saveIndicator->setText("● Error: DB not ready");
            saveIndicator->setStyleSheet("color: #F44336;");
            return;
        }
        
        QString title = titleBar->text();
        if (title.isEmpty()) {
            saveIndicator->setText("● Error: No title");
            saveIndicator->setStyleSheet("color: #F44336;");
            return;
        }
        
        if (!currentNote) {
            currentNote = new Note(currentTypeId, title);
        }
        
        currentNote->setTitle(title);
        currentNote->setContent(writeEditor->toPlainText());
        
        if (noteRepository->save(*currentNote)) {
            saveIndicator->setText("● Saved");
            saveIndicator->setStyleSheet("color: #4CAF50;");
            
            // Add newly saved note to the list if it's a new note (ID was -1 before save)
                // If repository assigned a positive ID, it's persisted
                if (currentNote->noteId() > 0) {
                // Check if note already exists in list
                bool noteExists = false;
                    for (int i = 0; i < noteList->count(); ++i) {
                    QListWidgetItem *item = noteList->item(i);
                    if (item->data(Qt::UserRole).toLongLong() == currentNote->noteId()) {
                        noteExists = true;
                        break;
                    }
                }
                
                // If new note, add it to the top of the list
                if (!noteExists) {
                    QString displayText = currentNote->title();
                    if (displayText.isEmpty()) {
                        displayText = "(Untitled)";
                    }
                    QString createdDate = currentNote->createdAt().toString("MMM dd, yyyy");
                    displayText = displayText + "\n" + createdDate;
                    
                    QListWidgetItem *newItem = new QListWidgetItem(displayText, nullptr);
                    // Use the INote `noteId()` for storage in the UI list
                    newItem->setData(Qt::UserRole, currentNote->noteId());
                    noteList->insertItem(0, newItem);
                }
            }
            
            // Reset status after 2 seconds
            QTimer::singleShot(2000, [this](){
                saveIndicator->setText("● Unsaved");
                saveIndicator->setStyleSheet("color: #E91E63;");
            });
        } else {
            saveIndicator->setText("● Error: Save failed");
            saveIndicator->setStyleSheet("color: #F44336;");
        }
    });
    
    // Phase 5: Wire note list item click to load note into editor
    connect(noteList, &QListWidget::itemClicked, [this](QListWidgetItem *item){
        qint64 noteId = item->data(Qt::UserRole).toLongLong();
        qDebug() << "[MainWindow] Loading note with ID:" << noteId;
        loadNoteIntoEditor(noteId);
    });
    
    // Phase 5: Load all saved notes from database on startup
    loadNotesFromDatabase();
    
    applyCustomStyles();
    setNoteType("markdown"); // Set default note type to trigger initial UI state
}

MainWindow::~MainWindow() {
    if (noteRepository) {
        delete noteRepository;
    }
    if (currentNote) {
        delete currentNote;
    }
}

void MainWindow::applyCustomStyles() {
    this->setStyleSheet(
        /* 1. Main Background */
        "QMainWindow { background-color: #F3E5F5; }"
        
        /* 2. Top Search Bar */
        "QLineEdit#searchBar { "
        "   background-color: white; border: 1px solid #CE93D8; "
        "   border-radius: 15px; padding-left: 20px; font-size: 13pt; color: #212121; "
        "}"

        /* 3. Buttons (Pill-Shaped) */
        "QPushButton { "
        "   background-color: #9C27B0; color: white; "
        "   border-radius: 16px; padding: 5px 15px; "
        "   font-weight: bold; font-size: 10pt; border: none; min-height: 30px; "
        "}"
        "QPushButton:hover { background-color: #7B1FA2; }"
        "QPushButton:pressed { background-color: #4A148C; }"
        "QPushButton:checked { "
        "   background-color: #4A148C; " /* Much darker purple for the active mode */
        "   border: 2px solid #CE93D8; " /* Optional: add a light border to make it pop */
        "}"

        /* 4. Specific Labels (The Fix) */
        "QLabel#saveIndicator { color: #E91E63; font-weight: bold; margin-right: 10px; }"
        "QLabel#listTitle { color: #4A148C; font-size: 14pt; font-weight: bold; margin-bottom: 5px; padding-left: 5px; }"

        /* 5. Left Sidebar (Saved Notes) */
        "QFrame#leftTile { background-color: #E1BEE7; border-radius: 12px; border: 1px solid #CE93D8; }"
        "QListWidget { background: transparent; border: none; color: #4A148C; font-size: 11pt; outline: none; }"
        "QListWidget::item { padding: 12px; border-bottom: 1px solid rgba(156, 39, 176, 0.1); color: #4A148C; }"
        "QListWidget::item:selected { background-color: rgba(156, 39, 176, 0.2); color: #4A148C; border-radius: 8px; font-weight: bold; }"

        /* 6. Right Editor Tile */
        "QFrame#rightTile { background-color: white; border-radius: 12px; border: 1px solid #CE93D8; }"
        
        "QLineEdit#titleBar { "
        "   background-color: rgba(225, 190, 231, 0.4); border: none; "
        "   border-bottom: 1px solid #CE93D8; font-size: 18pt; font-weight: bold; "
        "   padding: 10px 15px; color: #4A148C; "
        "   border-top-left-radius: 11px; border-top-right-radius: 11px; "
        "}"
        /* Fix the Toolbar background to match the titleBar */
        "QToolBar { "
        "   background-color: rgba(225, 190, 231, 0.2); " /* Subtle purple tint */
        "   border-bottom: 1px solid #CE93D8; "
        "   padding: 5px; spacing: 10px; "
        "}"
        "QToolBar QToolButton { "
        "   background-color: white; border: 1px solid #CE93D8; border-radius: 4px; "
        "   color: #4A148C; font-weight: bold; padding: 4px; "
        "}"
        "QToolBar QToolButton:hover { background-color: #F3E5F5; }"

        /* Ensure the area behind the mode buttons has a distinct background */
        "QWidget#modeContainer { " /* We will define this ID in a second */
        "   background-color: rgba(225, 190, 231, 0.1); "
        "   border-bottom: 1px solid #CE93D8; "
        "}"

        /* 7. Editor & Browser Boxes */
        "QTextEdit, QTextBrowser { "
        "   background-color: white; border: none; font-size: 13pt; "
        "   padding: 15px; color: #212121; "
        "}"
    );
}