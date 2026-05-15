#ifndef SQLITENOTEPOSITORY_H
#define SQLITENOTEPOSITORY_H

#include "INoteRepository.h"
#include "../api/ISnapshot.h"
#include <QSqlDatabase>

class Snapshot;
class Note;

// SqliteNoteRepository: SQLite-based implementation of INoteRepository.
// Uses WAL (Write-Ahead Logging) for crash resilience.
// Database schema: notes(id, typeId, title, content, created_at, modified_at).
// All calls are synchronous; no async/threading support.
class SqliteNoteRepository : public INoteRepository {
public:
    // Constructor: Opens or creates SQLite database at dbPath.
    // Initializes schema (tables, indexes) if needed.
    // If dbPath doesn't exist, creates it; caller must handle permission errors.
    SqliteNoteRepository(const QString &dbPath = "notes.db");

    // Destructor: Closes database connection safely.
    ~SqliteNoteRepository();

    // INoteRepository interface implementations
    bool save(Note &note) override;
    Note* getById(qint64 id) override;
    QVector<Note*> getAll() override;
    bool deleteById(qint64 id) override;
    bool update(const Note &note) override;
    QVector<Note*> searchByTitle(const QString &query) override;
    QVector<Note*> searchByContent(const QString &query) override;
    bool save(Note &note, const QString &password);
    Note* getById(qint64 id, const QString &password, bool *wrongPassword = nullptr);
    // Paged search API for large collections (Phase 6)
    QVector<Note*> searchByTitlePaged(const QString &query, int limit, int offset);
    int countTitleMatches(const QString &query);
    // Snapshot methods (Phase 6: FR8)
    bool saveSnapshot(Snapshot &snapshot);
    bool saveSnapshot(Snapshot &snapshot, const QString &password);
    QVector<Snapshot*> getSnapshotsByNoteId(qint64 noteId);
    QVector<Snapshot*> getSnapshotsByNoteId(qint64 noteId, const QString &password);
    Snapshot* getSnapshotById(qint64 snapshotId);  // Fetch single snapshot by ID
    Snapshot* getSnapshotById(qint64 snapshotId, const QString &password, bool *wrongPassword = nullptr);
    bool deleteSnapshotById(qint64 snapshotId);
    bool deleteOldestSnapshotForNote(qint64 noteId);
    // Trash/Retention (Phase 8)
    QVector<Note*> getTrashedNotes();
    bool trashNote(qint64 id);
    bool restoreNote(qint64 id);
    bool purgeTrashedNotes(int olderThanDays = 14);
    // Soft-delete (move to trash) API used by UI: mark notes trashed without removing snapshots
    bool markNoteTrashed(qint64 id);
    // isConnected(): Utility to check if database is open and ready.
    bool isConnected() const { return db.isOpen(); }
    bool pruneOldSnapshots(qint64 noteId);

private:
    QSqlDatabase db;                // Qt SQL database object (SQLite driver)
    bool initializeDatabase();       // Delegate to schema setup
    bool createTablesIfNeeded();     // Create tables and indexes on first run
};

#endif // SQLITENOTEPOSITORY_H
