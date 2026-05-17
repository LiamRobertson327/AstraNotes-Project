#ifndef NOTESERVICE_H
#define NOTESERVICE_H

#include "../interfaces/INoteService.h"
#include <QString>

class Note;
class SqliteNoteRepository;

/// NoteService implements INoteService and handles core note lifecycle:
/// load, save, and create. Snapshot and trash operations are delegated to
/// SnapshotService and TrashService respectively.
class NoteService : public INoteService {
public:
    explicit NoteService(SqliteNoteRepository *repository);

    // INoteService implementation
    Note *loadNote(qint64 noteId, const QString &password = QString(),
                   bool *wrongPassword = nullptr, QString *errorMessage = nullptr) override;
    bool saveNote(Note &note, const QString &password,
                  QString *errorMessage = nullptr) override;
    Note *createNote(const QString &typeId, const QString &title) override;

    // Storage helpers for UI
    bool isConnected() override;
    int countActiveNotes() override;
    int countActiveNotesByType(const QString &typeId) override;
    QVector<Note*> searchByTitlePaged(const QString &query, int pageSize, int offset) override;

    bool trashNote(qint64 noteId) override;

    // Robust loader: indicates when a password is required without performing UI prompts.
    Note *loadNoteRobust(qint64 noteId, const QString &password, bool *needsPassword = nullptr,
                         bool *wrongPassword = nullptr, QString *errorMessage = nullptr);

private:
    SqliteNoteRepository *m_repository;
};

#endif // NOTESERVICE_H