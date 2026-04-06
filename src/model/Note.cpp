#include "model/Note.h"
#include <iostream>
#include <format>

int Note::nextId = 1;

Note::Note(NoteType noteType, std::string noteTitle) 
    : type(noteType), title(noteTitle){

    this->id = nextId++;
    this->last_modified = std::chrono::system_clock::now();
}

void Note::setTitle(const std::string& new_title){
    title = new_title;
    last_modified = std::chrono::system_clock::now();
}

void Note::setContent(const std::string& newContent){
    content = newContent;
    last_modified = std::chrono::system_clock::now();
}

void Note::display() const{
    std::cout << "[" << id << "] " << title << "\n"
            << "Last Modified: " << std::format("{:%F %R}", last_modified) << "\n"
            << "Content: " << content << std::endl;
}