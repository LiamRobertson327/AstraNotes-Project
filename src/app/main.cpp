#include <QApplication>
#include <QCoreApplication>
#include "ui/mainwindow.h"
#include "logging/AuditLogger.h"

#include <iostream>
#include <vector>
#include "model/Note.h"

int main(int argc, char *argv[]) {
    // Initialize Qt application
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("AstraNotes");
    QCoreApplication::setOrganizationDomain("astranotes.local");
    QCoreApplication::setApplicationName("AstraNotes");

    // Install audit logger to capture qDebug/qWarning etc into an immutable log
    AuditLogger::install();

    qDebug() << "[main] QApplication created";
    qDebug() << "[main] Creating MainWindow...";

    // Create the main window
    MainWindow w;

    qDebug() << "[main] MainWindow created, about to show()";
    
    // Show the main window
    w.show();
    
    qDebug() << "[main] MainWindow shown, entering event loop";
    
    // Enter the main event loop and waiti until exit() is called
    return app.exec();
}