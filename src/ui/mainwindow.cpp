#include "MainWindow.h"

#include "TrashDialog.h"
#include "SettingsDialog.h"
#include <QSettings>

#include "../plugins/PluginManager.h"
#include "../plugins/PlaintextPlugin.h"
#include "../plugins/MarkdownPlugin.h"
#include "../model/Note.h"
#include "../model/Snapshot.h"
#include "../service/NoteService.h"
#include "../repository/SqliteNoteRepository.h"
#include <QTimer>
#include <QDebug>
#include <QAction>
#include <QDir>
#include <QStandardPaths>
#include <QApplication>
#include <QMenu>
#include <QMessageBox>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QDialog>
#include <QInputDialog>
#include <QTextCursor>
#include <QTextDocument>
#include <QRegularExpression>
#include <QColor>
#include <QShortcut>
#include <QScrollBar>
#include "../plugins/IFormattingAction.h"

QTextEdit *MainWindow::activeSearchEditor() const {
    if (!viewStack) {
        return nullptr;
    }

    switch (viewStack->currentIndex()) {
    case 0:
        return writeEditor;
    case 1:
        return readViewer;
    case 2:
        return splitEditor;
    default:
        return writeEditor;
    }
}

void MainWindow::setUnsavedChanges(bool dirty) {
    hasUnsavedChanges = dirty;
    if (!saveIndicator) {
        return;
    }

    if (dirty) {
        saveIndicator->setText("● Unsaved");
        saveIndicator->setStyleSheet("color: #E91E63;");
    }
}

void MainWindow::applySearchHighlight() {
    QTextEdit *editor = activeSearchEditor();
    if (!editor) {
        return;
    }

    QList<QTextEdit::ExtraSelection> selections;
    for (int i = 0; i < currentSearchMatches.size(); ++i) {
        const QPair<int, int> &match = currentSearchMatches.at(i);
        QTextEdit::ExtraSelection selection;
        QTextCursor cursor(editor->document());
        cursor.setPosition(match.first);
        cursor.setPosition(match.first + match.second, QTextCursor::KeepAnchor);
        selection.cursor = cursor;
        QTextCharFormat format;
        format.setBackground(QColor("#FFF59D"));
        selection.format = format;
        selections.append(selection);
    }

    if (currentSearchMatchIndex >= 0 && currentSearchMatchIndex < currentSearchMatches.size()) {
        const QPair<int, int> &match = currentSearchMatches.at(currentSearchMatchIndex);
        QTextEdit::ExtraSelection currentSelection;
        QTextCursor cursor(editor->document());
        cursor.setPosition(match.first);
        cursor.setPosition(match.first + match.second, QTextCursor::KeepAnchor);
        currentSelection.cursor = cursor;
        QTextCharFormat format;
        format.setBackground(QColor("#FFCA28"));
        format.setForeground(Qt::black);
        currentSelection.format = format;
        selections.append(currentSelection);
        editor->setTextCursor(cursor);
    }

    editor->setExtraSelections(selections);
}

void MainWindow::highlightCurrentTitleSearch(const QString &query) {
    if (!titleBar) {
        return;
    }

    const QString needle = query.trimmed();
    if (needle.isEmpty()) {
        titleBar->deselect();
        return;
    }

    const QString titleText = titleBar->text();
    const int index = titleText.indexOf(needle, 0, Qt::CaseInsensitive);
    if (index >= 0) {
        // Highlight the match without stealing focus from search bar
        titleBar->setSelection(index, needle.size());
    } else {
        titleBar->deselect();
    }
}

void MainWindow::updateMetadataDisplay() {
    if (!metadataLabel) {
        return;
    }

    if (!currentNote) {
        metadataLabel->setText("No note selected");
        return;
    }

    const QString created = currentNote->createdAt().toLocalTime().toString(Qt::ISODate);
    const QString modified = currentNote->lastModified().toLocalTime().toString(Qt::ISODate);
    const QString formatId = currentNote->typeId().isEmpty() ? "unknown" : currentNote->typeId();

    metadataLabel->setText(QString("Created: %1 | Modified: %2 | Format: %3").arg(created, modified, formatId));
}

bool MainWindow::promptForPassword(const QString &title, const QString &label, QString *password) {
    bool accepted = false;
    const QString entered = QInputDialog::getText(this, title, label, QLineEdit::Password, QString(), &accepted);
    if (!accepted) {
        return false;
    }
    if (password) {
        *password = entered;
    }
    return true;
}

bool MainWindow::promptForSessionPassword() {
    QString enteredPassword;
    if (!promptForPassword("Note Password", "Enter a password for this secured note:", &enteredPassword)) {
        sessionPassword.clear();
        return false;
    }

    sessionPassword = enteredPassword;
    return true;
}

void MainWindow::updateSavedNotesSearchState(const QString &query) {
    if (!noteList) {
        return;
    }

    const QString needle = query.trimmed();
    const bool filterEnabled = !needle.isEmpty();
    int matchCount = 0;
    QListWidgetItem *firstMatch = nullptr;
    QListWidgetItem *currentItem = noteList->currentItem();

    for (int i = 0; i < noteList->count(); ++i) {
        QListWidgetItem *item = noteList->item(i);
        const QString itemTitle = item->text().section('\n', 0, 0).trimmed();
        const bool matches = !filterEnabled || itemTitle.contains(needle, Qt::CaseInsensitive);

        item->setHidden(filterEnabled && !matches);
        item->setBackground(QBrush());

        if (matches && filterEnabled) {
            ++matchCount;
            item->setBackground(QColor("#E1BEE7"));
            if (!firstMatch) {
                firstMatch = item;
            }
        }
    }

    // Only auto-select first match if nothing is currently selected, or if current item is hidden by filter
    if ((!currentItem || currentItem->isHidden()) && firstMatch) {
        noteList->setCurrentItem(firstMatch);
        noteList->scrollToItem(firstMatch, QAbstractItemView::PositionAtTop);
    }

    Q_UNUSED(matchCount);
}

void MainWindow::updateSearchState(const QString &query) {
    currentSearchQuery = query;
    currentSearchMatches.clear();
    currentSearchMatchIndex = -1;

    highlightCurrentTitleSearch(query);
    updateSavedNotesSearchState(query);

    QTextEdit *editor = activeSearchEditor();
    if (!editor || query.trimmed().isEmpty()) {
        if (searchMatchLabel) {
            searchMatchLabel->setText("Search notes");
        }
        if (searchPrevButton) {
            searchPrevButton->setEnabled(false);
        }
        if (searchNextButton) {
            searchNextButton->setEnabled(false);
        }
        if (editor) {
            editor->setExtraSelections({});
        }
        return;
    }

    const QString content = editor->toPlainText();
    const QString needle = query.trimmed();
    int index = 0;
    while ((index = content.indexOf(needle, index, Qt::CaseInsensitive)) != -1) {
        currentSearchMatches.append(qMakePair(index, needle.size()));
        index += qMax(1, needle.size());
    }

    const bool titleMatches = titleBar && titleBar->text().contains(needle, Qt::CaseInsensitive);
    int sidebarMatches = 0;
    for (int i = 0; i < noteList->count(); ++i) {
        QListWidgetItem *item = noteList->item(i);
        if (!item->isHidden()) {
            ++sidebarMatches;
        }
    }

    if (!currentSearchMatches.isEmpty()) {
        currentSearchMatchIndex = 0;
        if (searchPrevButton) {
            searchPrevButton->setEnabled(true);
        }
        if (searchNextButton) {
            searchNextButton->setEnabled(true);
        }
        applySearchHighlight();
    } else {
        if (searchPrevButton) {
            searchPrevButton->setEnabled(false);
        }
        if (searchNextButton) {
            searchNextButton->setEnabled(false);
        }
        if (editor) {
            editor->setExtraSelections({});
        }
    }

    if (searchMatchLabel) {
        if (!currentSearchMatches.isEmpty() && (titleMatches || sidebarMatches > 0)) {
            searchMatchLabel->setText(QString("%1 in note, %2 note(s)").arg(currentSearchMatches.size()).arg(sidebarMatches));
        } else if (!currentSearchMatches.isEmpty()) {
            searchMatchLabel->setText(QString("%1 of %2").arg(currentSearchMatchIndex + 1).arg(currentSearchMatches.size()));
        } else if (titleMatches || sidebarMatches > 0) {
            searchMatchLabel->setText(QString("%1 note(s) matched").arg(sidebarMatches));
        } else {
            searchMatchLabel->setText("No matches");
        }
    }
}

void MainWindow::navigateSearchMatch(int direction) {
    if (currentSearchMatches.isEmpty()) {
        return;
    }

    currentSearchMatchIndex = (currentSearchMatchIndex + direction + currentSearchMatches.size()) % currentSearchMatches.size();
    if (searchMatchLabel) {
        searchMatchLabel->setText(QString("%1 of %2").arg(currentSearchMatchIndex + 1).arg(currentSearchMatches.size()));
    }
    applySearchHighlight();
}

bool MainWindow::confirmUnsavedChanges(const QString &actionText) {
    if (!hasUnsavedChanges) {
        return true;
    }

    QMessageBox::StandardButton choice = QMessageBox::question(
        this,
        "Unsaved Changes",
        QString("You have unsaved changes. Do you want to save before %1?").arg(actionText),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);

    if (choice == QMessageBox::Cancel) {
        return false;
    }

    if (choice == QMessageBox::Save) {
        return saveCurrentNote();
    }

    return true;
}

bool MainWindow::saveCurrentNote(bool createSnapshot) {
    if (autoSaveTimer) autoSaveTimer->stop();
    if (!noteService) {
        qWarning() << "noteService is null";
        saveIndicator->setText("● Error: DB not ready");
        saveIndicator->setStyleSheet("color: #F44336;");
        return false;
    }

    QString title = titleBar->text();
    if (title.isEmpty()) {
        saveIndicator->setText("● Error: No title");
        saveIndicator->setStyleSheet("color: #F44336;");
        return false;
    }

    if (!currentNote) {
        currentNote = new Note(currentTypeId, title);
    }

    currentNote->setTitle(title);
    currentNote->setContent(writeEditor->toPlainText());
    currentNote->setSecured(secureToggle && secureToggle->isChecked());

    const bool hasEncryptedState = !currentNote->encryptionSalt().isEmpty();
    if (currentNote->isSecured() && (!hasEncryptedState || sessionPassword.isEmpty())) {
        if (!promptForSessionPassword()) {
            saveIndicator->setText("● Save cancelled");
            saveIndicator->setStyleSheet("color: #F44336;");
            return false;
        }
    }

    QString saveError;
    if (noteService && noteService->saveNote(*currentNote, currentNote->isSecured() ? sessionPassword : QString(), createSnapshot, &saveError)) {
        setUnsavedChanges(false);
        saveIndicator->setText("● Saved");
        saveIndicator->setStyleSheet("color: #4CAF50;");

        if (currentNote->noteId() > 0) {
            bool noteExists = false;
            for (int i = 0; i < noteList->count(); ++i) {
                QListWidgetItem *item = noteList->item(i);
                if (item->data(Qt::UserRole).toLongLong() == currentNote->noteId()) {
                    noteExists = true;
                    item->setText(currentNote->displayText());
                    break;
                }
            }

            if (!noteExists) {
                QListWidgetItem *newItem = new QListWidgetItem(currentNote->displayText(), nullptr);
                newItem->setData(Qt::UserRole, currentNote->noteId());
                noteList->insertItem(0, newItem);
            }
        }

        updateMetadataDisplay();
        updateSearchState(searchBar ? searchBar->text() : QString());

        QTimer::singleShot(2000, [this](){
            if (!hasUnsavedChanges) {
                saveIndicator->setText("● Saved");
                saveIndicator->setStyleSheet("color: #4CAF50;");
            }
        });
        return true;
    }

    if (!saveError.isEmpty()) {
        qWarning() << "[MainWindow::saveCurrentNote]" << saveError;
    }
    saveIndicator->setText("● Error: Save failed");
    saveIndicator->setStyleSheet("color: #F44336;");
    return false;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (confirmUnsavedChanges("closing the app")) {
        // Create a final snapshot of the current note before app exits
        if (currentNote && currentNote->noteId() > 0) {
            createSnapshotForCurrentNote();
        }
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event && event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_F) {
        if (searchBar) {
            searchBar->setFocus();
            searchBar->selectAll();
        }
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::handleAutoSaveTimeout() {
    if (autoSaveEnabled && hasUnsavedChanges && currentNote) {
        qDebug() << "[MainWindow] Auto-save triggered (500ms debounce)";
        // Auto-save should not create snapshots to avoid excessive snapshot churn
        saveCurrentNote(false);
    }
}

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
    // Deprecated: use paged loader `loadNotesPage(int)` to populate the sidebar.
    loadNotesPage(0);
}

void MainWindow::loadNotesPage(int offset) {
    if (!noteRepository || !noteList) {
        return;
    }

    if (offset == 0) {
        noteList->clear();
        notesCurrentOffset = 0;
        notesAllLoaded = false;
    }

    if (notesAllLoaded) {
        return;
    }

    QVector<Note*> page = noteRepository->searchByTitlePaged(QString(), notesPageSize, offset);
    qDebug() << "[MainWindow::loadNotesPage] Loaded" << page.size() << "notes for offset" << offset;

    for (Note *note : page) {
        if (!note) continue;
        QListWidgetItem *item = new QListWidgetItem(note->displayText(), noteList);
        item->setData(Qt::UserRole, note->noteId());
        delete note;
    }

    if (page.size() < notesPageSize) {
        notesAllLoaded = true;
    } else {
        notesCurrentOffset += page.size();
    }
}

void MainWindow::onNoteListScrolled() {
    if (!noteList || !noteList->verticalScrollBar()) return;
    QScrollBar *sb = noteList->verticalScrollBar();
    int val = sb->value();
    int max = sb->maximum();
    // When within 120px of bottom, load next page
    if (val >= max - 120 && !notesAllLoaded) {
        loadNotesPage(notesCurrentOffset);
    }
}

void MainWindow::loadNoteIntoEditor(qint64 noteId) {
    if (!noteService) {
        qWarning() << "[MainWindow::loadNoteIntoEditor] noteService is null!";
        return;
    }
    
    qDebug() << "[MainWindow::loadNoteIntoEditor] Loading note with ID:" << noteId;
    
    // Retrieve the note from the service so the workflow stays outside the UI layer
    Note *note = noteService ? noteService->loadNote(noteId) : nullptr;
    if (!note) {
        qWarning() << "[MainWindow::loadNoteIntoEditor] Failed to load note with ID:" << noteId;
        return;
    }

    if (note->isSecured()) {
        QString enteredPassword;
        if (!promptForPassword("Decrypt Note", QString("Enter the password for '%1':").arg(note->title()), &enteredPassword)) {
            delete note;
            return;
        }

        bool wrongPassword = false;
        QString loadError;
        Note *decryptedNote = noteService ? noteService->loadNote(noteId, enteredPassword, &wrongPassword, &loadError) : nullptr;
        delete note;
        if (!decryptedNote) {
            if (wrongPassword) {
                QMessageBox::warning(this, "Incorrect Password", "The password you entered could not decrypt this note.");
            }
            if (!loadError.isEmpty()) {
                qWarning() << "[MainWindow::loadNoteIntoEditor]" << loadError;
            }
            return;
        }

        sessionPassword = enteredPassword;
        note = decryptedNote;
    } else {
        sessionPassword.clear();
    }
    
    // Update current note
    if (currentNote) {
        delete currentNote;
    }
    currentNote = note;
    isLoadingDocument = true;
    
    // Populate UI with note data
    titleBar->setText(note->title());
    writeEditor->setText(note->content());
    readViewer->setText(note->content());
    splitEditor->setText(note->content());
    if (secureToggle) {
        secureToggle->blockSignals(true);
        secureToggle->setChecked(note->isSecured());
        secureToggle->blockSignals(false);
    }

    // Set the note type to show/hide appropriate buttons
    setNoteType(note->typeId());
    
    // Clear unsaved indicator
    saveIndicator->setText("● Saved");
    saveIndicator->setStyleSheet("color: #4CAF50;");
    setUnsavedChanges(false);
    updateMetadataDisplay();
    updateSearchState(searchBar->text());
    isLoadingDocument = false;
    
    qDebug() << "[MainWindow::loadNoteIntoEditor] Note loaded successfully";
}

void MainWindow::createNewNote(const QString &typeId) {
    qDebug() << "[MainWindow::createNewNote] Creating new note of type:" << typeId;

    if (!confirmUnsavedChanges("creating a new note")) {
        qDebug() << "[MainWindow::createNewNote] Cancelled due to unsaved changes";
        return;
    }
    
    // Delete the current note if it exists
    if (currentNote) {
        // Before closing current note, persist a manual snapshot
        if (currentNote->noteId() > 0) {
            createSnapshotForCurrentNote();
        }
        delete currentNote;
    }
    
    // Create a new note with the selected type
    currentNote = new Note(typeId, "");
    sessionPassword.clear();
    isLoadingDocument = true;
    
    // Clear the UI
    titleBar->clear();
    titleBar->setPlaceholderText("Enter Note Title...");
    titleBar->setFocus();
    
    writeEditor->clear();
    readViewer->clear();
    splitEditor->clear();
    splitPreview->clear();
    if (secureToggle) {
        secureToggle->blockSignals(true);
        secureToggle->setChecked(false);
        secureToggle->blockSignals(false);
    }
    
    // Set the note type to show/hide appropriate buttons and populate toolbar
    setNoteType(typeId);
    updateMetadataDisplay();
    
    // Reset the save indicator
    saveIndicator->setText("● New Note");
    saveIndicator->setStyleSheet("color: #2196F3;");  // Blue for new notes
    setUnsavedChanges(false);
    updateSearchState(searchBar->text());
    isLoadingDocument = false;
    
    qDebug() << "[MainWindow::createNewNote] New note created and UI cleared";
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
        // Initialize Phase 1 members
        currentNote = nullptr;
    noteService = nullptr;
        currentTypeId = "plaintext";
    isLoadingDocument = false;
    hasUnsavedChanges = false;
    currentSearchMatchIndex = -1;
    
    // Initialize Phase 5 auto-save members (FR1 requirement)
    autoSaveEnabled = true;
    QSettings settingsLoad;
    autoSaveDebounceMs = settingsLoad.value("autoSaveDebounceMs", 3000).toInt();
    autoSaveTimer = new QTimer(this);
    autoSaveTimer->setInterval(autoSaveDebounceMs);
    autoSaveTimer->setSingleShot(true);

    // Phase 5: Bind crtl + f to search
    new QShortcut(Qt::CTRL + Qt::Key_F, this, [this](){
    searchBar->setFocus();
    searchBar->selectAll();  // Convenience: pre-select text so user can immediately type
    });

    // Phase 5: Bind ctrl + s to manual save
    new QShortcut(Qt::CTRL + Qt::Key_S, this, [this](){
        saveCurrentNote();
    });
    
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
        noteService = new NoteService(noteRepository);

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

    searchPrevButton = new QPushButton("<");
    searchPrevButton->setFixedWidth(36);
    searchPrevButton->setEnabled(false);

    searchNextButton = new QPushButton(">");
    searchNextButton->setFixedWidth(36);
    searchNextButton->setEnabled(false);

    searchMatchLabel = new QLabel("Search notes");
    searchMatchLabel->setObjectName("searchMatchLabel");
    
    newButton = new QPushButton("+ New Note");
    newButton->setObjectName("newButton");
    
    saveIndicator = new QLabel("● Unsaved");
    saveIndicator->setObjectName("saveIndicator"); // Must match the CSS #saveIndicator
    
    saveButton = new QPushButton("Save");
    historyButton = new QPushButton("History");
    trashButton = new QPushButton("🗑️");
    trashButton->setToolTip("Show trashed notes");
    deleteButton = new QPushButton("Delete");
    deleteButton->setToolTip("Move selected notes to Trash");
    settingsButton = new QPushButton("Settings");
    secureToggle = new QCheckBox("Secure");
    secureToggle->setToolTip("Encrypt this note when saved");
    
    // Phase 5 (FR1): Auto-save toggle checkbox
    autoSaveToggle = new QCheckBox("Auto-save");
    autoSaveToggle->setChecked(autoSaveEnabled);
    autoSaveToggle->setObjectName("autoSaveToggle");

    headerLayout->addWidget(searchBar, 4);
    headerLayout->addWidget(searchPrevButton);
    headerLayout->addWidget(searchNextButton);
    headerLayout->addWidget(searchMatchLabel);
    headerLayout->addWidget(newButton);
    headerLayout->addWidget(saveIndicator);
    headerLayout->addWidget(saveButton);
    headerLayout->addWidget(historyButton);
    headerLayout->addWidget(trashButton);
    headerLayout->addWidget(deleteButton);
    headerLayout->addWidget(settingsButton);
    headerLayout->addWidget(secureToggle);
    headerLayout->addWidget(autoSaveToggle);  // Add auto-save toggle to header
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
    noteList->setSelectionMode(QAbstractItemView::ExtendedSelection);

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

    metadataLabel = new QLabel("No note selected");
    metadataLabel->setObjectName("metadataLabel");
    metadataLabel->setWordWrap(true);
    rightTileLayout->addWidget(metadataLabel);
    
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
        updateSearchState(currentSearchQuery);
    });

    connect(searchBar, &QLineEdit::textChanged, [this](const QString &text){
        updateSearchState(text);
        // Ensure search bar retains focus after state updates
        searchBar->setFocus();
    });

    connect(searchPrevButton, &QPushButton::clicked, [this](){
        navigateSearchMatch(-1);
    });

    connect(searchNextButton, &QPushButton::clicked, [this](){
        navigateSearchMatch(1);
    });

    connect(titleBar, &QLineEdit::textChanged, [this](const QString &){
        if (!isLoadingDocument) {
            setUnsavedChanges(true);
            // Reset auto-save timer on keystroke (500ms debounce)
            if (autoSaveEnabled) {
                autoSaveTimer->stop();
                autoSaveTimer->start();
            }
        }
    });

    connect(writeEditor, &QTextEdit::textChanged, [this](){
        if (!isLoadingDocument) {
            setUnsavedChanges(true);
            updateSearchState(currentSearchQuery);
            // Reset auto-save timer on keystroke (500ms debounce)
            if (autoSaveEnabled) {
                autoSaveTimer->stop();
                autoSaveTimer->start();
            }
        }
    });

    connect(splitEditor, &QTextEdit::textChanged, [this](){
        splitPreview->setMarkdown(splitEditor->toPlainText());
        // Mirror back to writeEditor so content isn't lost when switching modes
        writeEditor->setPlainText(splitEditor->toPlainText());
        if (!isLoadingDocument) {
            setUnsavedChanges(true);
            updateSearchState(currentSearchQuery);
            // Reset auto-save timer on keystroke (500ms debounce)
            if (autoSaveEnabled) {
                autoSaveTimer->stop();
                autoSaveTimer->start();
            }
        }
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
        saveCurrentNote();
    });
    
    // Phase 5 (FR1): Wire auto-save toggle
    connect(autoSaveToggle, &QCheckBox::toggled, [this](bool checked){
        autoSaveEnabled = checked;
        if (!checked) {
            // Cancel timer if auto-save is disabled
            autoSaveTimer->stop();
        }
    });
    
    // Phase 5 (FR1): Wire auto-save timer to slot
    connect(autoSaveTimer, &QTimer::timeout, this, &MainWindow::handleAutoSaveTimeout);
    
    // Phase 5: Wire note list item click to load note into editor
    connect(noteList, &QListWidget::itemClicked, [this](QListWidgetItem *item){
        qint64 noteId = item->data(Qt::UserRole).toLongLong();
        qDebug() << "[MainWindow] Loading note with ID:" << noteId;
        if (!confirmUnsavedChanges("opening another note")) {
            return;
        }

        // Before switching notes, create a snapshot of the current note (manual snapshot)
        if (currentNote && currentNote->noteId() > 0) {
            createSnapshotForCurrentNote();
        }

        loadNoteIntoEditor(noteId);
    });

    // Wire history button to show snapshot history dialog
    connect(historyButton, &QPushButton::clicked, [this]() {
        showSnapshotHistoryDialog();
    });

    connect(secureToggle, &QCheckBox::toggled, [this](bool checked) {
        if (currentNote) {
            currentNote->setSecured(checked);
            setUnsavedChanges(true);
        }
    });

    // Wire trash button to show trashed notes dialog
    connect(trashButton, &QPushButton::clicked, [this]() {
        showTrashDialog();
    });

    connect(deleteButton, &QPushButton::clicked, [this]() {
        // Trash selected notes from the sidebar
        QList<QListWidgetItem*> items = noteList->selectedItems();
        if (items.isEmpty()) {
            QMessageBox::information(this, "Delete", "No notes selected to delete.");
            return;
        }

        int ret = QMessageBox::question(this, "Confirm Delete", QString("Move %1 selected note(s) to Trash?").arg(items.size()));
        if (ret != QMessageBox::Yes) {
            return;
        }

        int moved = 0;
        bool currentNoteWasTrashed = false;
        for (QListWidgetItem *item : items) {
            qint64 noteId = item->data(Qt::UserRole).toLongLong();
            if (currentNote && currentNote->noteId() == noteId) {
                currentNoteWasTrashed = true;
            }
            if (noteRepository && noteRepository->trashNote(noteId)) {
                ++moved;
            }
        }

        QMessageBox::information(this, "Delete", QString("Moved %1 notes to Trash.").arg(moved));
        // Refresh list
        loadNotesPage(0);
        // If currentNote was moved, clear editor
        if (currentNoteWasTrashed) {
            delete currentNote;
            currentNote = nullptr;
            writeEditor->clear();
            titleBar->clear();
            updateMetadataDisplay();
        }
    });

    connect(settingsButton, &QPushButton::clicked, [this]() {
        showSettingsDialog();
    });
    
    // Phase 6: Initialize pagination state and connect lazy-load
    notesPageSize = 50; // reasonable default page size
    notesCurrentOffset = 0;
    notesAllLoaded = false;
    if (noteList && noteList->verticalScrollBar()) {
        connect(noteList->verticalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::onNoteListScrolled);
    }

    // Phase 5: Load initial page of saved notes from database on startup
    loadNotesPage(0);
    // Phase 8: Load retention settings and purge behavior
    QSettings s;
    retentionDays = s.value("retentionDays", 14).toInt();
    autoPurgeEnabled = s.value("autoPurgeEnabled", true).toBool();

    if (autoPurgeEnabled && noteRepository) {
        noteRepository->purgeTrashedNotes(retentionDays);
    }

    // Schedule periodic purge (24 hours) only if auto purge enabled
    purgeTimer = new QTimer(this);
    purgeTimer->setInterval(24 * 60 * 60 * 1000); // 24 hours
    connect(purgeTimer, &QTimer::timeout, [this]() {
        if (autoPurgeEnabled && noteRepository) {
            noteRepository->purgeTrashedNotes(retentionDays);
        }
    });
    if (autoPurgeEnabled) purgeTimer->start();
    
    applyCustomStyles();
    setNoteType("markdown"); // Set default note type to trigger initial UI state
}

MainWindow::~MainWindow() {
    if (noteService) {
        delete noteService;
    }
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

// ============================================================================
// Snapshot Methods (Phase 6: FR8 - Version History)
// ============================================================================

void MainWindow::createSnapshotForCurrentNote() {
    // Auto-create a snapshot when a note is saved.
    // This captures the exact state of title and content at save time.
    if (!currentNote || !noteService) {
        return;
    }

    qDebug() << "[MainWindow::createSnapshotForCurrentNote] Creating snapshot for note ID:" << currentNote->noteId();

    QString snapshotError;
    if (noteService && noteService->saveSnapshotForNote(*currentNote, currentNote->isSecured() ? sessionPassword : QString(), &snapshotError)) {
        qDebug() << "[MainWindow::createSnapshotForCurrentNote] Snapshot created for note ID:" << currentNote->noteId();
    } else {
        if (!snapshotError.isEmpty()) {
            qWarning() << "[MainWindow::createSnapshotForCurrentNote]" << snapshotError;
        }
        qWarning() << "[MainWindow::createSnapshotForCurrentNote] Failed to save snapshot";
    }
}

void MainWindow::showSnapshotHistoryDialog() {
    // Display a dialog with list of snapshots for current note.
    // Allow user to view snapshot details, revert to snapshot, or delete snapshot.
    if (!currentNote || !noteRepository) {
        qWarning() << "[MainWindow::showSnapshotHistoryDialog] No note loaded";
        return;
    }

    qDebug() << "[MainWindow::showSnapshotHistoryDialog] Showing snapshot history for note ID:" << currentNote->noteId();

    QVector<Snapshot*> snapshots = noteRepository->getSnapshotsByNoteId(currentNote->noteId());

    if (snapshots.isEmpty()) {
        QMessageBox::information(this, "Snapshot History", "No snapshots yet for this note.");
        return;
    }

    // Create a dialog to display snapshots
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Snapshot History");
    dialog->setMinimumWidth(500);
    dialog->setMinimumHeight(300);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    // Create a list widget to show snapshots
    QListWidget *snapshotList = new QListWidget();

    for (Snapshot *snapshot : snapshots) {
        QListWidgetItem *item = new QListWidgetItem(snapshot->displayText());
        item->setData(Qt::UserRole, snapshot->snapshotId());
        snapshotList->addItem(item);
    }

    layout->addWidget(new QLabel("Select a snapshot to view or restore:"));
    layout->addWidget(snapshotList);

    // Button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *revertButton = new QPushButton("Revert to Snapshot");
    QPushButton *deleteButton = new QPushButton("Delete Snapshot");
    QPushButton *closeButton = new QPushButton("Close");

    buttonLayout->addWidget(revertButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    layout->addLayout(buttonLayout);

    // Connect buttons
    connect(revertButton, &QPushButton::clicked, [this, snapshotList, dialog]() {
        QListWidgetItem *item = snapshotList->currentItem();
        if (!item) {
            QMessageBox::warning(this, "Error", "Please select a snapshot to revert to.");
            return;
        }

        qint64 snapshotId = item->data(Qt::UserRole).toLongLong();
        onRevertToSnapshot(snapshotId);
        dialog->close();
    });

    connect(deleteButton, &QPushButton::clicked, [this, snapshotList, dialog, noteRepository = this->noteRepository]() {
        QListWidgetItem *item = snapshotList->currentItem();
        if (!item) {
            QMessageBox::warning(this, "Error", "Please select a snapshot to delete.");
            return;
        }

        qint64 snapshotId = item->data(Qt::UserRole).toLongLong();
        if (noteRepository->deleteSnapshotById(snapshotId)) {
            QMessageBox::information(this, "Success", "Snapshot deleted.");
            snapshotList->takeItem(snapshotList->row(item));
            delete item;
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete snapshot.");
        }
    });

    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::close);

    dialog->exec();

    // Clean up
    for (Snapshot *snap : snapshots) {
        delete snap;
    }
    delete dialog;
}

void MainWindow::onRevertToSnapshot(qint64 snapshotId) {
    // Revert current note to a previous snapshot.
    // This creates a NEW snapshot of the current state (before reverting),
    // then restores the selected snapshot's content.
    if (!currentNote || !noteRepository) {
        qWarning() << "[MainWindow::onRevertToSnapshot] No note loaded or repository not ready";
        return;
    }

    qDebug() << "[MainWindow::onRevertToSnapshot] Reverting to snapshot ID:" << snapshotId;

    // Fetch the target snapshot directly (before creating safety snapshot)
    Snapshot *targetSnapshot = noteRepository->getSnapshotById(snapshotId);

    if (!targetSnapshot) {
        qWarning() << "[MainWindow::onRevertToSnapshot] Snapshot not found with ID:" << snapshotId;
        QMessageBox::warning(this, "Error", "Snapshot not found.");
        return;
    }

    if (targetSnapshot->isSecured()) {
        QString enteredPassword;
        if (!promptForPassword("Decrypt Snapshot", QString("Enter the password for the selected snapshot:"), &enteredPassword)) {
            delete targetSnapshot;
            return;
        }

        bool wrongPassword = false;
        Snapshot *decryptedSnapshot = noteRepository->getSnapshotById(snapshotId, enteredPassword, &wrongPassword);
        delete targetSnapshot;
        if (!decryptedSnapshot) {
            if (wrongPassword) {
                QMessageBox::warning(this, "Incorrect Password", "The password you entered could not decrypt this snapshot.");
            }
            return;
        }

        sessionPassword = enteredPassword;
        targetSnapshot = decryptedSnapshot;
    }

    // Create a safety snapshot of the current state (undo point) only after we've loaded the target
    createSnapshotForCurrentNote();

    // Restore the snapshot's content
    isLoadingDocument = true;
    titleBar->setText(targetSnapshot->title());
    writeEditor->setPlainText(targetSnapshot->content());
    splitEditor->setPlainText(targetSnapshot->content());
    splitPreview->setMarkdown(targetSnapshot->content());
    if (secureToggle) {
        secureToggle->blockSignals(true);
        secureToggle->setChecked(currentNote ? currentNote->isSecured() : false);
        secureToggle->blockSignals(false);
    }
    isLoadingDocument = false;

    // Mark as unsaved so user can review before saving
    setUnsavedChanges(true);
    saveIndicator->setText("● Reverted (unsaved)");
    saveIndicator->setStyleSheet("color: #FF9800;");  // Orange for reverted

    qDebug() << "[MainWindow::onRevertToSnapshot] Reverted to snapshot ID:" << snapshotId;

    delete targetSnapshot;

    QMessageBox::information(this, "Snapshot Restored", "Note reverted to selected snapshot. Review and save to confirm.");
}

void MainWindow::showTrashDialog() {
    if (!noteRepository) {
        QMessageBox::warning(this, "Trash", "Repository not available.");
        return;
    }

    TrashDialog dlg(noteRepository, retentionDays, this);
    connect(&dlg, &TrashDialog::changesApplied, this, [this]() {
        // Refresh sidebar and current note state after actions
        loadNotesPage(0);
        if (currentNote && currentNote->noteId() > 0) {
            // If current note was restored/purged, reload
            Note *reloaded = noteRepository->getById(currentNote->noteId());
            if (!reloaded) {
                // Note may have been purged; clear editor
                currentNote = nullptr;
                writeEditor->clear();
                titleBar->clear();
                updateMetadataDisplay();
            } else {
                delete reloaded; // keep current in-memory until user reloads
            }
        }
    });

    dlg.exec();
}

void MainWindow::showSettingsDialog() {
    SettingsDialog dlg(retentionDays, autoPurgeEnabled, autoSaveDebounceMs, this);
    if (dlg.exec() == QDialog::Accepted) {
        retentionDays = dlg.retentionDays();
        autoPurgeEnabled = dlg.autoPurgeEnabled();
        autoSaveDebounceMs = dlg.autoSaveDebounceMs();
        
        QSettings s;
        s.setValue("retentionDays", retentionDays);
        s.setValue("autoPurgeEnabled", autoPurgeEnabled);
        s.setValue("autoSaveDebounceMs", autoSaveDebounceMs);

        // Apply auto-save debounce change instantly
        autoSaveTimer->setInterval(autoSaveDebounceMs);

        if (autoPurgeEnabled) {
            if (!purgeTimer->isActive()) purgeTimer->start();
            if (noteRepository) noteRepository->purgeTrashedNotes(retentionDays);
        } else {
            if (purgeTimer->isActive()) purgeTimer->stop();
        }
    }
}