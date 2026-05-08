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

class Note;  // Forward declaration for Phase 1

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
    QLineEdit *searchBar;
    QPushButton *newButton;
    QLabel *saveIndicator;
    QPushButton *saveButton;

    // --- Left Sidebar (Saved Notes) ---
    QLabel *listTitle;
    QListWidget *noteList;

    // --- Right Editor Area ---
    QLineEdit *titleBar;
    QToolBar *formattingToolbar;
    
        // Current note and type state (Phase 1)
        Note *currentNote;
        QString currentTypeId;
    
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
    void loadNoteIntoEditor(qint64 noteId);  // Phase 5: Load a specific note into the editor
    void createNewNote(const QString &typeId);  // Create a new note of specified type
};

#endif // MAINWINDOW_H