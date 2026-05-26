#ifndef SNAPSHOTSERVICE_H
#define SNAPSHOTSERVICE_H

#include "../interfaces/ISnapshotService.h"

#include <memory>
#include <vector>

class INoteRepository;
class Note;
class Snapshot;

/// SnapshotService implements snapshot (version history) operations.
/// It delegates to the repository and provides a clean service-layer interface
/// for managing note snapshots.
class SnapshotService : public ISnapshotService {
public:
    explicit SnapshotService(INoteRepository *repository);

    // ISnapshotService implementation
    bool saveSnapshot(const Note &note, const QString &password = QString(),
                      QString *errorMessage = nullptr) override;
    std::vector<std::unique_ptr<Snapshot>> getSnapshotsByNoteId(qint64 noteId) override;
    std::unique_ptr<Snapshot> getSnapshotById(qint64 snapshotId, const QString &password = QString(),
                                              bool *wrongPassword = nullptr) override;
    bool deleteSnapshot(qint64 snapshotId) override;
    bool deleteOldestSnapshotForNote(qint64 noteId) override;
    void enforceSnapshotLimit(qint64 noteId, int maxSnapshots = 2) override;

    // Revert workflow: create a safety snapshot of the current note state,
    // then fetch and return the requested snapshot (decrypted if password provided).
    // Returns a move-owned `unique_ptr<Snapshot>` or nullptr on error.
    std::unique_ptr<Snapshot> revertToSnapshot(class Note &currentNote, qint64 snapshotId,
                                               const QString &password = QString(),
                                               QString *errorMessage = nullptr) override;

private:
    INoteRepository *m_repository;
};

#endif // SNAPSHOTSERVICE_H
