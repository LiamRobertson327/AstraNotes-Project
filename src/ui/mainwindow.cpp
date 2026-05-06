#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);
    this->setMinimumSize(1000, 700);
    this->setWindowTitle("AstraNotes");

    QVBoxLayout *rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(15, 10, 15, 15);
    rootLayout->setSpacing(15);

    // --- HEADER SECTION ---
    QHBoxLayout *headerLayout = new QHBoxLayout();
    searchBar = new QLineEdit();
    searchBar->setObjectName("searchBar");
    searchBar->setPlaceholderText("Search notes...");
    
    saveIndicator = new QLabel("● Unsaved");
    saveButton = new QPushButton("Save");

    headerLayout->addWidget(searchBar, 4);
    headerLayout->addWidget(saveIndicator);
    headerLayout->addWidget(saveButton);
    rootLayout->addLayout(headerLayout);

    // --- CONTENT SECTION ---
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(15);

    // LEFT TILE: Note List
    QFrame *leftTile = new QFrame();
    leftTile->setObjectName("leftTile");
    QVBoxLayout *leftTileLayout = new QVBoxLayout(leftTile);
    leftTileLayout->setContentsMargins(10, 15, 10, 10); // More top margin for title

    listTitle = new QLabel("Saved Notes");
    listTitle->setObjectName("listTitle");
    
    noteList = new QListWidget();
    noteList->setObjectName("noteList");
    noteList->setSpacing(2);

    leftTileLayout->addWidget(listTitle);
    leftTileLayout->addWidget(noteList);

    // RIGHT TILE: Editor Area
    QFrame *rightTile = new QFrame();
    rightTile->setObjectName("rightTile");
    QVBoxLayout *rightTileLayout = new QVBoxLayout(rightTile);
    rightTileLayout->setContentsMargins(0, 0, 0, 0);
    rightTileLayout->setSpacing(0);

    titleBar = new QLineEdit();
    titleBar->setObjectName("titleBar");
    titleBar->setPlaceholderText("Enter Note Title...");
    titleBar->setFixedHeight(60);

    noteContext = new QTextEdit();
    noteContext->setObjectName("noteContext");
    noteContext->setPlaceholderText("Start typing...");

    rightTileLayout->addWidget(titleBar);
    rightTileLayout->addWidget(noteContext);

    contentLayout->addWidget(leftTile, 1);
    contentLayout->addWidget(rightTile, 3);
    rootLayout->addLayout(contentLayout);

    applyCustomStyles();
}

MainWindow::~MainWindow() {}

void MainWindow::applyCustomStyles() {
    this->setStyleSheet(
        "QMainWindow { background-color: #F3E5F5; }"
        
        /* Header */
        "QLineEdit#searchBar { background-color: white; border: 1px solid #CE93D8; border-radius: 15px; padding-left: 15px; font-size: 13pt; color: #212121; }"
        "QPushButton { background-color: #9C27B0; color: white; border-radius: 5px; padding: 8px; font-weight: bold; }"
        "QLabel { color: #E91E63; font-weight: bold; margin-right: 10px; }"

        /* LEFT TILE (Purple) */
        "QFrame#leftTile { background-color: #E1BEE7; border-radius: 12px; border: 1px solid #CE93D8; }"
        "QLabel#listTitle { color: #4A148C; font-size: 14pt; font-weight: bold; margin-bottom: 5px; padding-left: 5px; }"
        "QListWidget { background: transparent; border: none; color: #4A148C; font-size: 11pt; outline: none; }"
        "QListWidget::item { padding: 12px; border-bottom: 1px solid rgba(156, 39, 176, 0.1); color: #4A148C; }"
        "QListWidget::item:selected { background-color: rgba(156, 39, 176, 0.2); color: #4A148C; border-radius: 8px; font-weight: bold; }"

        /* RIGHT TILE (White/Tinted) */
        "QFrame#rightTile { background-color: white; border-radius: 12px; border: 1px solid #CE93D8; }"
        "QLineEdit#titleBar { "
        "   background-color: rgba(225, 190, 231, 0.4); "
        "   border: none; border-bottom: 1px solid #CE93D8; "
        "   font-size: 18pt; font-weight: bold; padding: 10px 15px; color: #4A148C; "
        "   border-top-left-radius: 11px; border-top-right-radius: 11px; "
        "}"
        "QTextEdit#noteContext { "
        "   background-color: white; border: none; font-size: 13pt; padding: 15px; color: #212121; "
        "   border-bottom-left-radius: 11px; border-bottom-right-radius: 11px; "
        "}"
    );
}

