class SqliteNoteRepository;
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QTextBrowser>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QFrame>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QToolBar>
#include <QVector>
#include <QTimer>
#include <QCheckBox>

class QCloseEvent;
class QKeyEvent;

class Note;  // Forward declaration for Phase 1
class NoteService;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // --- Top Header ---
    QWidget *centralWidget;
        // Repository for persistence (Phase 4)
        SqliteNoteRepository *noteRepository;
        NoteService *noteService;
    QLineEdit *searchBar;
    QPushButton *searchPrevButton;
    QPushButton *searchNextButton;
    QLabel *searchMatchLabel;
    QPushButton *newButton;
    QLabel *saveIndicator;
    QPushButton *saveButton;
    QPushButton *historyButton;
    QPushButton *trashButton;
    QPushButton *deleteButton;
    QPushButton *settingsButton;
    QCheckBox *secureToggle;
    QCheckBox *autoSaveToggle;
    QTimer *autoSaveTimer;
    bool autoSaveEnabled;
    int autoSaveDebounceMs;
    QString sessionPassword;
    QTimer *purgeTimer;
    int retentionDays;
    bool autoPurgeEnabled;

    // --- Left Sidebar (Saved Notes) ---
    QLabel *listTitle;
    QListWidget *noteList;
    // Pagination state for large note collections (Phase 6)
    int notesPageSize;
    int notesCurrentOffset;
    bool notesAllLoaded;

    // --- Right Editor Area ---
    QLineEdit *titleBar;
    QLabel *metadataLabel;
    QToolBar *formattingToolbar;
    
        // Current note and type state (Phase 1)
        Note *currentNote;
        QString currentTypeId;
        bool isLoadingDocument;
        bool hasUnsavedChanges;
        QString currentSearchQuery;
        QVector<QPair<int, int>> currentSearchMatches;
        int currentSearchMatchIndex;
    
    // Mode Selection Buttons
    QPushButton *btnWrite;
    QPushButton *btnRead;
    QPushButton *btnSplit;

    // The Stack that swaps between views
    QStackedWidget *viewStack;

    // PAGE 0: Write Mode
    QTextEdit *writeEditor;

    // PAGE 1: Read Mode
    QTextBrowser *readViewer;

    // PAGE 2: Split Mode
    QWidget *splitWidget;
    QTextEdit *splitEditor;
    QTextBrowser *splitPreview;

    // Helper Methods
    void applyCustomStyles();
    void setupSignals(); // New: To keep the logic separate from UI layout
    void setNoteType(const QString &typeId);  // Phase 1: Update UI based on note type
    void populateFormattingToolbar(const QString &typeId);  // Phase 3: Populate toolbar from plugin
    void loadNotesFromDatabase();  // Phase 5: Load all saved notes on startup
    void loadNotesPage(int offset); // Phase 6: Load a page of notes into the sidebar
    void onNoteListScrolled();      // Phase 6: Handle scrollbar events for lazy-load
    void loadNoteIntoEditor(qint64 noteId);  // Phase 5: Load a specific note into the editor
    void createNewNote(const QString &typeId);  // Create a new note of specified type
    void updateSearchState(const QString &query);
    void updateSavedNotesSearchState(const QString &query);
    void highlightCurrentTitleSearch(const QString &query);
    void navigateSearchMatch(int direction);
    void applySearchHighlight();
    void updateMetadataDisplay();
    bool promptForSessionPassword();
    bool promptForPassword(const QString &title, const QString &label, QString *password);
    QTextEdit *activeSearchEditor() const;
    void setUnsavedChanges(bool dirty);
    bool saveCurrentNote(bool createSnapshot = true);
    bool confirmUnsavedChanges(const QString &actionText);
    void createSnapshotForCurrentNote();  // Phase 6: FR8 - Auto-create snapshot on save
    void showSnapshotHistoryDialog();      // Phase 6: FR8 - Display snapshot list and revert/delete UI
    void showTrashDialog();                // Phase 8: Show trashed notes dialog
    void showSettingsDialog();             // Phase 8: Settings for retention and purge

private slots:
    void handleAutoSaveTimeout();
    void onRevertToSnapshot(qint64 snapshotId);  // Phase 6: FR8 - Revert note to a snapshot

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // MAINWINDOW_H