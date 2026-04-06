#ifndef NOTE_H
#define NOTE_H

#include <string>
#include <chrono>

enum class NoteType {
    Text,
    Voice,
    Secure
};

class Note {
public:
    Note(NoteType noteType, std::string noteTitle);

    //Getters
    std::string getTitle() const { return title; }
    int getId() const { return id; }

    //Setters
    void setTitle(const std::string& new_title);
    void setContent(const std::string& newContect);

    //Methods
    void display() const;

private:
    static int nextId;
    int id;
    NoteType type;
    std::string title;
    std::string content;
    std::chrono::system_clock::time_point last_modified;

};

#endif