#ifndef SQLITENOTEPOSITORY_H
#define SQLITENOTEPOSITORY_H

#include "INoteRepository.h"
#include <QSqlDatabase>

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

    // isConnected(): Utility to check if database is open and ready.
    bool isConnected() const { return db.isOpen(); }

private:
    QSqlDatabase db;                // Qt SQL database object (SQLite driver)
    bool initializeDatabase();       // Delegate to schema setup
    bool createTablesIfNeeded();     // Create tables and indexes on first run
};

#endif // SQLITENOTEPOSITORY_H
