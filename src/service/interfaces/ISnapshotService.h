#ifndef ISNAPSHOTSERVICE_H
#define ISNAPSHOTSERVICE_H

#include <QString>
#include <memory>
#include <vector>

class Note;
class Snapshot;

/// ISnapshotService defines the interface for snapshot (version history) operations.
/// Snapshots allow users to view and revert to previous versions of notes.
class ISnapshotService {
public:
    virtual ~ISnapshotService() = default;

    /// Save a snapshot of the current note state.
    /// Handles encryption metadata reuse for secured notes.
    /// @param note           Note to snapshot
    /// @param password       Password for encryption (empty for plaintext)
    /// @param errorMessage   User-facing error message if save fails
    virtual bool saveSnapshot(const Note &note, const QString &password = QString(),
                              QString *errorMessage = nullptr) = 0;

    /// Retrieve all snapshots for a given note, ordered by creation date (newest first).
    /// Returns move-owned `unique_ptr<Snapshot>` instances in a std::vector.
    /// @param noteId         Note ID to fetch snapshots for
    virtual std::vector<std::unique_ptr<Snapshot>> getSnapshotsByNoteId(qint64 noteId) = 0;

    /// Get a specific snapshot by ID. Returns a move-owned `unique_ptr<Snapshot>`.
    /// @param snapshotId     Snapshot ID to fetch
    /// @param password       Password to decrypt if the snapshot is encrypted
    /// @param wrongPassword  Set to true if decryption failed due to wrong password
    virtual std::unique_ptr<Snapshot> getSnapshotById(qint64 snapshotId, const QString &password = QString(),
                                                     bool *wrongPassword = nullptr) = 0;

    /// Delete a snapshot by ID.
    /// @param snapshotId     Snapshot ID to delete
    virtual bool deleteSnapshot(qint64 snapshotId) = 0;

    /// Delete the oldest snapshot for a given note (used to enforce snapshot limits).
    /// @param noteId         Note ID
    virtual bool deleteOldestSnapshotForNote(qint64 noteId) = 0;

    /// Enforce the maximum snapshot limit for a note (e.g., keep only 2 most recent).
    /// @param noteId         Note ID
    /// @param maxSnapshots   Maximum snapshots to keep
    virtual void enforceSnapshotLimit(qint64 noteId, int maxSnapshots = 2) = 0;

    /// Revert workflow: save a safety snapshot then load and return target snapshot.
    /// Returns a move-owned `unique_ptr<Snapshot>` or nullptr on error.
    virtual std::unique_ptr<Snapshot> revertToSnapshot(Note &currentNote, qint64 snapshotId,
                                                       const QString &password = QString(),
                                                       QString *errorMessage = nullptr) = 0;
};

#endif // ISNAPSHOTSERVICE_H
