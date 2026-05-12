#include "SqliteNoteRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QVariant>

SqliteNoteRepository::SqliteNoteRepository(const QString &dbPath) {
    qDebug() << "[SqliteNoteRepository] Constructor starting with path:" << dbPath;
    db = QSqlDatabase::addDatabase("QSQLITE");
    qDebug() << "[SqliteNoteRepository] QSqlDatabase driver added";
    db.setDatabaseName(dbPath);
    qDebug() << "[SqliteNoteRepository] Database name set to:" << dbPath;
    
    if (!db.open()) {
        qWarning() << "[SqliteNoteRepository] FAILED TO OPEN DATABASE:" << db.lastError().text();
    } else {
        qDebug() << "[SqliteNoteRepository] Database opened successfully at:" << dbPath;
        initializeDatabase();
        qDebug() << "[SqliteNoteRepository] Database initialization complete";
    }
}

SqliteNoteRepository::~SqliteNoteRepository() {
    if (db.isOpen()) {
        db.close();
    }
}

bool SqliteNoteRepository::initializeDatabase() {
    return createTablesIfNeeded();
}

bool SqliteNoteRepository::createTablesIfNeeded() {
    qDebug() << "[SqliteNoteRepository::createTablesIfNeeded] Starting";
    QSqlQuery query(db);
    
    // Enable WAL (Write-Ahead Logging) mode for crash recovery and concurrent access.
    // WAL keeps a log of uncommitted transactions; safer than default DELETE mode.
    if (!query.exec("PRAGMA journal_mode=WAL;")) {
        qWarning() << "[SqliteNoteRepository] Failed to enable WAL mode:" << query.lastError().text();
    } else {
        qDebug() << "[SqliteNoteRepository] WAL mode enabled";
    }
    
    // Define schema: id (auto-increment PK), typeId (format: plaintext/markdown/voice/etc),
    // title, content (both are INote::title() and INote::content()),
    // created_at (set once on insert), modified_at (updated on each save).
    QString createTableSql = R"(
        CREATE TABLE IF NOT EXISTS notes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            typeId TEXT NOT NULL DEFAULT 'plaintext',
            title TEXT,
            content TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            modified_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";
    
    if (!query.exec(createTableSql)) {
        qWarning() << "[SqliteNoteRepository] Failed to create notes table:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "[SqliteNoteRepository] Notes table created or already exists";
    
    // Create index on title for O(log n) LIKE searches via searchByTitle().
    // Improves performance for large note collections.
    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_title ON notes(title);")) {
        qWarning() << "[SqliteNoteRepository] Failed to create title index:" << query.lastError().text();
    } else {
        qDebug() << "[SqliteNoteRepository] Title index created or already exists";
    }
    
    return true;
}

bool SqliteNoteRepository::save(Note &note) {
    // Upsert logic: if note.noteId() == -1, perform INSERT (new note);
    // otherwise, UPDATE (existing note). On INSERT, assign DB-generated ID to the note.
    // Uses INote API (noteId(), title(), content(), typeId()) for access.
    qDebug() << "[SqliteNoteRepository::save] Starting save operation";
    qDebug() << "[SqliteNoteRepository::save] Database open?" << db.isOpen();
    qDebug() << "[SqliteNoteRepository::save] Database name:" << db.databaseName();
    qDebug() << "[SqliteNoteRepository::save] Note ID:" << note.noteId();
    qDebug() << "[SqliteNoteRepository::save] Title:" << note.title();
    qDebug() << "[SqliteNoteRepository::save] Content length:" << note.content().length();
    qDebug() << "[SqliteNoteRepository::save] TypeId:" << note.typeId();
    
    QSqlQuery query(db);
    
    if (note.noteId() == -1) {
        // INSERT: New note; no ID yet. Database will auto-generate via AUTOINCREMENT.
        qDebug() << "[SqliteNoteRepository::save] Performing INSERT";
        query.prepare(R"(
            INSERT INTO notes (typeId, title, content, modified_at)
            VALUES (:typeId, :title, :content, CURRENT_TIMESTAMP)
        )");
        query.addBindValue(note.typeId());
        query.addBindValue(note.title());
        query.addBindValue(note.content());
        
        qDebug() << "[SqliteNoteRepository::save] Query prepared, attempting exec()";
        if (!query.exec()) {
            qWarning() << "[SqliteNoteRepository::save] FAILED TO INSERT:" << query.lastError().text();
            qWarning() << "[SqliteNoteRepository::save] Query:" << query.lastQuery();
            return false;
        }
        
        // Retrieve the auto-generated rowid and assign it back to the note.
        // Caller can now use note.noteId() to identify this note in future saves.
        qint64 newId = query.lastInsertId().toLongLong();
        note.setNoteId(newId);
        qDebug() << "[SqliteNoteRepository::save] Note saved with ID:" << newId;
        return true;
    } else {
        // UPDATE: Existing note with ID > 0. Update all fields, set modified_at to NOW.
        qDebug() << "[SqliteNoteRepository::save] Performing UPDATE";
        query.prepare(R"(
            UPDATE notes
            SET typeId = :typeId, title = :title, content = :content, modified_at = CURRENT_TIMESTAMP
            WHERE id = :id
        )");
        
        query.addBindValue(note.typeId());
        query.addBindValue(note.title());
        query.addBindValue(note.content());
        query.addBindValue(note.noteId());
        
        qDebug() << "[SqliteNoteRepository::save] Query prepared, attempting exec()";
        if (!query.exec()) {
            qWarning() << "[SqliteNoteRepository::save] FAILED TO UPDATE:" << query.lastError().text();
            qWarning() << "[SqliteNoteRepository::save] Query:" << query.lastQuery();
            return false;
        }
        
        qDebug() << "[SqliteNoteRepository::save] Note updated with ID:" << note.noteId();
        return true;
    }
}

Note* SqliteNoteRepository::getById(qint64 id) {
    // Retrieve a single note by primary key. Returns nullptr if not found or on error.
    // Caller assumes ownership of the returned Note* and must delete it.
    QSqlQuery query(db);
    query.prepare("SELECT id, typeId, title, content, created_at FROM notes WHERE id = :id");
    query.addBindValue(id);
    
    if (!query.exec()) {
        qWarning() << "[SqliteNoteRepository::getById] Failed to query note:" << query.lastError().text();
        return nullptr;
    }
    
    if (!query.next()) {
        qDebug() << "[SqliteNoteRepository::getById] Note not found with ID:" << id;
        return nullptr;
    }
    
    // Construct Note from row: pass typeId and title to constructor,
    // then populate ID, content, and createdAt via INote setters.
    Note *note = new Note(query.value("typeId").toString(), query.value("title").toString());
    note->setNoteId(query.value("id").toLongLong());
    note->setContent(query.value("content").toString());
    note->setCreatedAt(query.value("created_at").toDateTime());
    
    return note;
}

QVector<Note*> SqliteNoteRepository::getAll() {
    // Retrieve all notes from the database, sorted by most recently modified first.
    // Used on app startup to populate the UI sidebar.
    // Caller assumes ownership of all Note* pointers in the returned vector; must delete them.
    // Returns empty vector on error (not an error condition itself).
    qDebug() << "[SqliteNoteRepository::getAll] Starting getAll";
    QVector<Note*> notes;
    QSqlQuery query(db);
    query.prepare("SELECT id, typeId, title, content, created_at FROM notes ORDER BY modified_at DESC");
    
    if (!query.exec()) {
        qWarning() << "[SqliteNoteRepository::getAll] Failed to fetch all notes:" << query.lastError().text();
        return notes;
    }
    
    // Build a Note object for each row, preserving field order: constructor, then setters.
    while (query.next()) {
        Note *note = new Note(query.value("typeId").toString(), query.value("title").toString());
        note->setNoteId(query.value("id").toLongLong());
        note->setContent(query.value("content").toString());
        note->setCreatedAt(query.value("created_at").toDateTime());
        notes.append(note);
    }
    
    qDebug() << "[SqliteNoteRepository::getAll] Fetched" << notes.size() << "notes from database";
    return notes;
}

bool SqliteNoteRepository::deleteById(qint64 id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM notes WHERE id = :id");
    query.addBindValue(id);
    
    if (!query.exec()) {
        qWarning() << "Failed to delete note:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "Note deleted with ID:" << id;
    return true;
}

bool SqliteNoteRepository::update(const Note &note) {
    QSqlQuery query(db);
    query.prepare(R"(
        UPDATE notes
        SET typeId = :typeId, title = :title, content = :content, modified_at = CURRENT_TIMESTAMP
        WHERE id = :id
    )");
    
    query.addBindValue(note.typeId());
    query.addBindValue(note.title());
    query.addBindValue(note.content());
    query.addBindValue(note.noteId());
    
    if (!query.exec()) {
        qWarning() << "Failed to update note:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QVector<Note*> SqliteNoteRepository::searchByTitle(const QString &query_str) {
    // Case-insensitive substring search on note titles using LIKE '%query%'.
    // Leverages idx_title index for performance on large datasets.
    // Results ordered by most recently modified first (UI UX: newest edits first).
    // Caller assumes ownership of returned Note* pointers.
    QVector<Note*> results;
    QSqlQuery query(db);
    query.prepare("SELECT id, typeId, title, content, created_at FROM notes WHERE title LIKE :query ORDER BY modified_at DESC");
    query.addBindValue("%" + query_str + "%");
    
    if (!query.exec()) {
        qWarning() << "Failed to search by title:" << query.lastError().text();
        return results;
    }
    
    while (query.next()) {
        Note *note = new Note(query.value("typeId").toString(), query.value("title").toString());
        note->setNoteId(query.value("id").toLongLong());
        note->setContent(query.value("content").toString());
        note->setCreatedAt(query.value("created_at").toDateTime());
        results.append(note);
    }
    
    qDebug() << "Found" << results.size() << "notes matching:" << query_str;
    return results;
}

QVector<Note*> SqliteNoteRepository::searchByContent(const QString &query_str) {
    // Case-insensitive substring search on note content using LIKE '%query%'.
    // Note: No index on content column (would be too large for typical notes).
    // Full-text search can be added later if performance becomes an issue.
    // Results ordered by most recently modified first.
    // Caller assumes ownership of returned Note* pointers.
    QVector<Note*> results;
    QSqlQuery query(db);
    query.prepare("SELECT id, typeId, title, content, created_at FROM notes WHERE content LIKE :query ORDER BY modified_at DESC");
    query.addBindValue("%" + query_str + "%");
    
    if (!query.exec()) {
        qWarning() << "Failed to search by content:" << query.lastError().text();
        return results;
    }
    
    while (query.next()) {
        Note *note = new Note(query.value("typeId").toString(), query.value("title").toString());
        note->setNoteId(query.value("id").toLongLong());
        note->setContent(query.value("content").toString());
        note->setCreatedAt(query.value("created_at").toDateTime());
        results.append(note);
    }
    
    return results;
}
