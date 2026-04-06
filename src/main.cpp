#include <iostream>
#include <vector>

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>

#include "Note.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("AstraNotes - Qt 6 Test");
    
    QPushButton *button = new QPushButton("Hello from Qt 6!", &window);
    window.setCentralWidget(button);

    window.show();
    

    // std::vector<std::unique_ptr<Note>> notes;

    // std::cout <<"--- AstraNotes Booting Up ---" << std::endl;

    // notes.push_back(std::make_unique<Note>(NoteType::Text, "Test note 1"));

    // notes.back()->setContent("This text will demonstrate that the dispaly function works correctly");

    // notes.back()->display();

    // std::cout <<"--- Test Complete ---" << std::endl;
    return QCoreApplication::exec();
}