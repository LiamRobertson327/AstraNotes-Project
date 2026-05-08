#include <QApplication>
#include "ui/mainwindow.h"

#include <iostream>
#include <vector>
#include "model/Note.h"

int main(int argc, char *argv[]) {

    /*
    This block is used solely for testing the Note class and its functionality. 
    */
    std::vector<std::unique_ptr<Note>> notes;
    std::cout <<"--- AstraNotes Booting Up ---" << std::endl;
    notes.push_back(std::make_unique<Note>(QStringLiteral("core.text"), QStringLiteral("Test note 1")));
    notes.push_back(std::make_unique<Note>(QStringLiteral("core.text"), QStringLiteral("Test note 2")));
    for(const auto& note : notes){
        note->setContent(QStringLiteral("This text will demonstrate that the dispaly function works correctly"));
        note->display();
    }
    std::cout <<"--- Test Complete ---" << std::endl;
    /*
    End block
    */

    QApplication app(argc, argv);
    qDebug() << "[main] QApplication created";
    
    qDebug() << "[main] Creating MainWindow...";
    MainWindow w;
    qDebug() << "[main] MainWindow created, about to show()";
    w.show();
    qDebug() << "[main] MainWindow shown, entering event loop";
    return QCoreApplication::exec();
}