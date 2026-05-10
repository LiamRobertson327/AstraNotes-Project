#include <QApplication>
#include "ui/mainwindow.h"

#include <iostream>
#include <vector>
#include "model/Note.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    qDebug() << "[main] QApplication created";
    
    qDebug() << "[main] Creating MainWindow...";
    MainWindow w;
    qDebug() << "[main] MainWindow created, about to show()";
    w.show();
    qDebug() << "[main] MainWindow shown, entering event loop";
    return QCoreApplication::exec();
}