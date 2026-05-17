#ifndef TRASHSERVICE_H
#define TRASHSERVICE_H

#include "../interfaces/ITrashService.h"

class SqliteNoteRepository;
class Note;

/// TrashService implements trash and retention management operations.
/// It provides a clean service-layer interface for soft-delete, restoration,
/// and automatic purging of old notes.
class TrashService : public ITrashService {
public:
    explicit TrashService(SqliteNoteRepository *repository);

    // ITrashService implementation
    bool trashNote(qint64 noteId) override;
    bool restoreNote(qint64 noteId) override;
    bool purgeNote(qint64 noteId) override;
    void purgeOldTrashedNotes(int retentionDays = 14) override;
    QVector<Note *> getTrashedNotes(int limit = 100, int offset = 0) override;
    int countTrashedNotes() override;
    bool isNoteTrashed(qint64 noteId) override;

private:
    SqliteNoteRepository *m_repository;
};

#endif // TRASHSERVICE_H
