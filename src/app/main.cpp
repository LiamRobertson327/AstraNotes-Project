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
    notes.push_back(std::make_unique<Note>(NoteType::Text, "Test note 1"));
    notes.push_back(std::make_unique<Note>(NoteType::Text, "Test note 2"));
    for(const auto& note : notes){
        note->setContent("This text will demonstrate that the dispaly function works correctly");
        note->display();
    }
    std::cout <<"--- Test Complete ---" << std::endl;
    /*
    End block
    */

    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}