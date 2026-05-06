#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QFrame>

// ... existing includes ...
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QWidget *centralWidget;
    QLineEdit *searchBar;
    QLabel *saveIndicator;
    QPushButton *saveButton;
    
    // Left side
    QLabel *listTitle; // New title label
    QListWidget *noteList;

    // Right side
    QLineEdit *titleBar;
    QTextEdit *noteContext;

    void applyCustomStyles();
};

#endif // MAINWINDOW_H