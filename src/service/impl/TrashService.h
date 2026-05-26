#ifndef TRASHSERVICE_H
#define TRASHSERVICE_H

#include "../interfaces/ITrashService.h"
#include <memory>
#include <vector>

class INoteRepository;
class Note;

/// TrashService implements trash and retention management operations.
/// It provides a clean service-layer interface for soft-delete, restoration,
/// and automatic purging of old notes.
class TrashService : public ITrashService {
public:
    explicit TrashService(INoteRepository *repository);

    // ITrashService implementation
    bool trashNote(qint64 noteId) override;
    bool restoreNote(qint64 noteId) override;
    bool purgeNote(qint64 noteId) override;
    void purgeOldTrashedNotes(int retentionDays = 14) override;
    std::vector<std::unique_ptr<Note>> getTrashedNotes(int limit = 100, int offset = 0) override;
    int countTrashedNotes() override;
    bool isNoteTrashed(qint64 noteId) override;

private:
    INoteRepository *m_repository;
};

#endif // TRASHSERVICE_H
