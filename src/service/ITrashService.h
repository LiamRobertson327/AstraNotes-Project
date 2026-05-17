#ifndef ITRASHSERVICE_H
#define ITRASHSERVICE_H

#include <QString>
#include <QVector>

class Note;

/// ITrashService defines the interface for trash and retention operations.
/// Manages soft-delete (trash state), restoration, and automatic purging of old notes.
class ITrashService {
public:
    virtual ~ITrashService() = default;

    /// Move a note to trash (soft-delete). The note remains in the database
    /// but is marked as trashed and hidden from normal lists.
    /// @param noteId         Note ID to trash
    virtual bool trashNote(qint64 noteId) = 0;

    /// Restore a trashed note back to active state.
    /// @param noteId         Note ID to restore
    virtual bool restoreNote(qint64 noteId) = 0;

    /// Permanently delete a trashed note (purge from database).
    /// @param noteId         Note ID to purge
    virtual bool purgeNote(qint64 noteId) = 0;

    /// Permanently delete all trashed notes older than the retention period.
    /// Executed at startup or on a scheduled timer.
    /// @param retentionDays  How many days a trashed note is kept before auto-purge
    virtual void purgeOldTrashedNotes(int retentionDays = 14) = 0;

    /// Retrieve all trashed notes. Caller owns the returned Note objects.
    /// @param limit          Maximum number of notes to return
    /// @param offset         Pagination offset
    virtual QVector<Note *> getTrashedNotes(int limit = 100, int offset = 0) = 0;

    /// Get the count of trashed notes.
    /// Reserved for possible future badge or summary UI.
    virtual int countTrashedNotes() = 0;

    /// Check if a note is currently in trash state.
    /// Reserved for possible future quick-status checks.
    /// @param noteId         Note ID to check
    virtual bool isNoteTrashed(qint64 noteId) = 0;
};

#endif // ITRASHSERVICE_H
