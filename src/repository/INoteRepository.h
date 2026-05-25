#ifndef INOTEREPOSITORY_H
#define INOTEREPOSITORY_H

#include "../model/Note.h"
#include <QVector>
#include <QString>
#include <vector>
#include <memory>

// INoteRepository: Persistence abstraction for INote implementations.
// All methods work with INote-based models (currently Note; future VoiceNote, etc.).
// Implementations (e.g., SqliteNoteRepository) handle database specifics.
//
// Ownership / lifetime:
// - Legacy APIs: Returned `Note*` and `Snapshot*` pointers are caller-owned
//   and must be deleted by the caller. Input references remain caller-owned;
//   repository methods never take over ownership of Note or Snapshot
//   instances passed by reference.
// - RAII migration: Newer APIs return move-owning types (e.g.
//   `std::vector<std::unique_ptr<Note>>`) to transfer sole ownership to the
//   caller. Implementations should adopt these RAII-returning overloads where
//   possible to avoid raw-pointer ownership confusion.
class INoteRepository {
public:
    virtual ~INoteRepository() = default;

    // CRUD operations
    // save(): Insert new note if noteId==-1 (and assign DB-generated ID), else update.
    //         Returns true on success; updates note.noteId() on insert.
    virtual bool save(Note &note) = 0;

    // getById(): Retrieve a note by primary key. Returns nullptr if not found.
    //            Caller assumes ownership; must delete the returned Note*.
    virtual Note* getById(qint64 id) = 0;
    // Overload for encrypted notes: attempt to decrypt using `password`.
    // If the password is wrong and `wrongPassword` is provided, set it to true.
    virtual Note* getById(qint64 id, const QString &password, bool *wrongPassword = nullptr) = 0;

    // getAll(): Retrieve all notes, ordered by most recently modified first.
    //           Returns empty vector if none exist or on error.
    //           Caller receives move-owned `unique_ptr<Note>` instances and
    //           thus takes ownership; objects will be cleaned up automatically
    //           when the returned vector goes out of scope.
    virtual std::vector<std::unique_ptr<Note>> getAll() = 0;

    // deleteById(): Perform hard delete of a note by primary key.
    //               Returns true on success, false on error or if note not found.
    virtual bool deleteById(qint64 id) = 0;

    // update(): Update an existing note (same as save() if noteId > 0).
    //           Note parameter is const; caller retains ownership.
    virtual bool update(const Note &note) = 0;

    // Search operations
    // searchByTitle(): Case-insensitive LIKE search on title.
    //                  Returns matching notes ordered by most recently modified.
    //                  Caller assumes ownership of returned Note* pointers.
    virtual QVector<Note*> searchByTitle(const QString &query) = 0;

    // searchByContent(): Case-insensitive LIKE search on content.
    //                    Returns matching notes ordered by most recently modified.
    //                    Caller receives move-owned `unique_ptr<Note>` instances.
    virtual std::vector<std::unique_ptr<Note>> searchByContent(const QString &query) = 0;

    // Snapshot methods (Phase 6: FR8)
    virtual bool saveSnapshot(class Snapshot &snapshot) = 0;
    virtual bool saveSnapshot(class Snapshot &snapshot, const QString &password) = 0;
    virtual QVector<class Snapshot*> getSnapshotsByNoteId(qint64 noteId) = 0;
    virtual QVector<class Snapshot*> getSnapshotsByNoteId(qint64 noteId, const QString &password) = 0;
    virtual class Snapshot* getSnapshotById(qint64 snapshotId) = 0;
    virtual class Snapshot* getSnapshotById(qint64 snapshotId, const QString &password, bool *wrongPassword = nullptr) = 0;
    virtual bool deleteSnapshotById(qint64 snapshotId) = 0;
    virtual bool deleteOldestSnapshotForNote(qint64 noteId) = 0;
    virtual bool pruneOldSnapshots(qint64 noteId) = 0;

    // Trash / Retention API (Phase 8)
    // Returns trashed notes as move-owned `unique_ptr`s. Caller receives
    // ownership and must not attempt to delete the pointed-to objects directly
    // (they will be cleaned up when the unique_ptrs go out of scope).
    virtual std::vector<std::unique_ptr<Note>> getTrashedNotes() = 0;
    virtual std::vector<std::unique_ptr<Note>> getTrashedNotes(int limit, int offset) = 0;
    virtual int countTrashedNotes() = 0;
    virtual bool isNoteTrashed(qint64 id) = 0;
    virtual bool trashNote(qint64 id) = 0;
    virtual bool restoreNote(qint64 id) = 0;
    // Purge trashed notes older than `olderThanDays`. Default 14 days.
    virtual bool purgeTrashedNotes(int olderThanDays = 14) = 0;

    // Additional storage helpers used by service layer / UI
    virtual int countActiveNotes() const = 0;
    virtual int countActiveNotesByType(const QString &typeId) const = 0;
    virtual std::vector<std::unique_ptr<Note>> searchByTitlePaged(const QString &query, int limit, int offset) = 0;
    virtual int countTitleMatches(const QString &query) = 0;
    // Check if the underlying storage is available/connected
    virtual bool isConnected() const = 0;

    // Optional overload for implementations that support saving with a
    // password (encrypted notes). Default implementation forwards to
    // the basic `save(note)` so implementors that don't support
    // passworded saves do not need to change.
    virtual bool save(Note &note, const QString &password) {
        (void)password;
        return save(note);
    }
};

#endif // INOTEREPOSITORY_H
