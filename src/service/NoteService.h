#ifndef NOTESERVICE_H
#define NOTESERVICE_H

#include <QString>

class Note;
class Snapshot;
class SqliteNoteRepository;

// NoteService centralizes note lifecycle orchestration so the UI layer only
// handles prompts, state rendering, and user interactions.
class NoteService {
public:
    explicit NoteService(SqliteNoteRepository *repository);

    Note *loadNote(qint64 noteId, const QString &password = QString(), bool *wrongPassword = nullptr, QString *errorMessage = nullptr);
    bool saveNote(Note &note, const QString &password, bool createSnapshot, QString *errorMessage = nullptr);
    bool saveSnapshotForNote(const Note &note, const QString &password = QString(), QString *errorMessage = nullptr);

private:
    SqliteNoteRepository *m_repository;
};

#endif // NOTESERVICE_H