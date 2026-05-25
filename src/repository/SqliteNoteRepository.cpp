#include "SqliteNoteRepository.h"
#include "model/Snapshot.h"
#include "../crypto/EncryptionService.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QVariant>
#include <QMessageBox>
#include <QTimeZone>

namespace {
QDateTime toUtcDateTime(const QVariant &value) {
    QDateTime dateTime = QDateTime::fromString(value.toString(), "yyyy-MM-dd HH:mm:ss");
    if (!dateTime.isValid()) {
        dateTime = value.toDateTime();
    }
    if (!dateTime.isValid()) {
        return dateTime;
    }
    return QDateTime(dateTime.date(), dateTime.time(), QTimeZone::utc());
}

bool ensureColumnExists(QSqlDatabase &db, const QString &tableName, const QString &columnName, const QString &columnDefinition) {
    QSqlQuery pragma(db);
    if (!pragma.exec(QString("PRAGMA table_info(%1)").arg(tableName))) {
        qWarning() << "[SqliteNoteRepository] Failed to inspect table" << tableName << pragma.lastError().text();
        return false;
    }

    while (pragma.next()) {
        if (pragma.value("name").toString() == columnName) {
            return true;
        }
    }

    QSqlQuery alter(db);
    if (!alter.exec(QString("ALTER TABLE %1 ADD COLUMN %2").arg(tableName, columnDefinition))) {
        qWarning() << "[SqliteNoteRepository] Failed to add column" << columnName << "to" << tableName << ":" << alter.lastError().text();
        return false;
    }

    return true;
}
} // namespace

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
            is_secured INTEGER DEFAULT 0,
            encryption_salt TEXT,
            encryption_iv TEXT,
            encryption_tag TEXT,
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

    // Create snapshots table for Phase 6: FR8 (version history)
    // Each snapshot captures note title and content at a specific moment.
    // Foreign key constraint ensures snapshots are deleted when parent note is deleted.
    // Max 2 snapshots per note; oldest deleted when 3rd is created.
    QString createSnapshotTableSql = R"(
        CREATE TABLE IF NOT EXISTS snapshots (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            note_id INTEGER NOT NULL,
            title TEXT,
            is_secured INTEGER DEFAULT 0,
            encryption_salt TEXT,
            encryption_iv TEXT,
            encryption_tag TEXT,
            content TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (note_id) REFERENCES notes(id) ON DELETE CASCADE
        );
    )";
    
    if (!query.exec(createSnapshotTableSql)) {
        qWarning() << "[SqliteNoteRepository] Failed to create snapshots table:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "[SqliteNoteRepository] Snapshots table created or already exists";
    
    // Index on note_id for fast lookup of snapshots for a given note.
    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_snapshot_note_id ON snapshots(note_id);")) {
        qWarning() << "[SqliteNoteRepository] Failed to create snapshot note_id index:" << query.lastError().text();
    } else {
        qDebug() << "[SqliteNoteRepository] Snapshot note_id index created or already exists";
    }

    ensureColumnExists(db, "notes", "is_secured", "is_secured INTEGER DEFAULT 0");
    ensureColumnExists(db, "notes", "encryption_salt", "encryption_salt TEXT");
    ensureColumnExists(db, "notes", "encryption_iv", "encryption_iv TEXT");
    ensureColumnExists(db, "notes", "encryption_tag", "encryption_tag TEXT");
    // Phase 8: Trash/Retention columns
    ensureColumnExists(db, "notes", "is_trashed", "is_trashed INTEGER DEFAULT 0");
    ensureColumnExists(db, "notes", "trashed_at", "trashed_at DATETIME");
    // Phase 6 (NFR1): Create FTS5 virtual table for title and content indexing
    // Provides fast full-text search for 10k+ notes without manual index management
    ensureColumnExists(db, "snapshots", "is_secured", "is_secured INTEGER DEFAULT 0");
    ensureColumnExists(db, "snapshots", "encryption_salt", "encryption_salt TEXT");
    ensureColumnExists(db, "snapshots", "encryption_iv", "encryption_iv TEXT");
    ensureColumnExists(db, "snapshots", "encryption_tag", "encryption_tag TEXT");
    QString createFtsSql = R"(
        CREATE VIRTUAL TABLE IF NOT EXISTS notes_fts USING fts5(
            title, content, content='notes', content_rowid='id'
        );
    )";
    
    if (!query.exec(createFtsSql)) {
        qWarning() << "[SqliteNoteRepository] Failed to create FTS5 table:" << query.lastError().text();
        // FTS5 is optional; don't fail initialization if it's unavailable
    } else {
        qDebug() << "[SqliteNoteRepository] FTS5 table created or already exists";
        
        // Rebuild FTS index from existing notes (in case of schema upgrades)
        if (!query.exec("INSERT INTO notes_fts(rowid, title, content) SELECT id, title, content FROM notes WHERE id NOT IN (SELECT rowid FROM notes_fts);")) {
            qDebug() << "[SqliteNoteRepository] FTS5 index population: " << query.lastError().text();
        }
    }
    
    return true;
}


bool SqliteNoteRepository::save(Note &note) {
    // If we call save without a password, we assume the note was already
    // encrypted or we're doing a non-secure update.
    return save(note, QString());
}

bool SqliteNoteRepository::save(Note &note, const QString &password) {
    QSqlQuery query(db);
    
    // 1. Prepare data for persistence
    QString contentToPersist = note.content();
    QString saltToPersist = note.encryptionSalt();
    QString ivToPersist = note.encryptionIv();
    QString tagToPersist = note.encryptionTag();

    // 2. Handle Encryption Logic
    if (note.isSecured()) {

        // We only perform a NEW encryption if a password was provided.
        if (!password.isEmpty()) {
            EncryptionService::EncryptedPayload payload = EncryptionService::encrypt(note.content(), password);
            
            if (payload.ciphertextBase64.isEmpty()) {
                QMessageBox::critical(nullptr, "Error", "Encryption failed internally.");
                return false;
            }

            contentToPersist = payload.ciphertextBase64;
            saltToPersist = payload.saltBase64;
            ivToPersist = payload.ivBase64;
            tagToPersist = payload.tagBase64;
            note.setEncryptionSalt(saltToPersist);
            note.setEncryptionIv(ivToPersist);
            note.setEncryptionTag(tagToPersist);

            note.setContent(contentToPersist);
        } else {
            // If no password provided but note is secured, we assume the content 
            // is ALREADY encrypted and just pass it through.
            if (saltToPersist.isEmpty()) {
                qWarning() << "Attempted to save secured note without password or existing salt.";
                return false;
            }
        }
    } else {
        // If not secured, ensure encryption fields are wiped in the DB
        saltToPersist.clear();
        ivToPersist.clear();
        tagToPersist.clear();
    }

    // 3. Database Execution
    if (note.noteId() == -1) {
        query.prepare(R"(
            INSERT INTO notes (typeId, title, content, is_secured, encryption_salt, encryption_iv, encryption_tag, modified_at)
            VALUES (:typeId, :title, :content, :is_secured, :encryption_salt, :encryption_iv, :encryption_tag, CURRENT_TIMESTAMP)
        )");
    } else {
        query.prepare(R"(
            UPDATE notes
            SET typeId = :typeId, title = :title, content = :content, 
                is_secured = :is_secured, encryption_salt = :encryption_salt, 
                encryption_iv = :encryption_iv, encryption_tag = :encryption_tag, 
                modified_at = CURRENT_TIMESTAMP
            WHERE id = :id
        )");
        query.bindValue(":id", note.noteId());
    }

    query.bindValue(":typeId", note.typeId());
    query.bindValue(":title", note.title());
    query.bindValue(":content", contentToPersist);
    query.bindValue(":is_secured", note.isSecured() ? 1 : 0);
    query.bindValue(":encryption_salt", saltToPersist);
    query.bindValue(":encryption_iv", ivToPersist);
    query.bindValue(":encryption_tag", tagToPersist);

    if (!query.exec()) {
        QString dbError = query.lastError().text();
        QMessageBox::critical(nullptr, "Database Save Failed", 
            "The database rejected the save.\nError: " + dbError);
        return false;
    }

    // Update ID if it was an insert
    if (note.noteId() == -1) {
        note.setNoteId(query.lastInsertId().toLongLong());
    }

    return true;
}

Note* SqliteNoteRepository::getById(qint64 id) {
    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, typeId, title, content, is_secured, encryption_salt, encryption_iv, encryption_tag, created_at, modified_at
        FROM notes
        WHERE id = :id
    )");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "[SqliteNoteRepository::getById] Failed to query note metadata:" << query.lastError().text();
        return nullptr;
    }

    if (!query.next()) {
        qDebug() << "[SqliteNoteRepository::getById] Note not found with ID:" << id;
        return nullptr;
    }

    Note *note = new Note(query.value("typeId").toString(), query.value("title").toString());
    note->setNoteId(query.value("id").toLongLong());
    note->setCreatedAt(toUtcDateTime(query.value("created_at")));
    note->setLastModified(toUtcDateTime(query.value("modified_at")));
    note->setSecured(query.value("is_secured").toInt() == 1);
    note->setEncryptionSalt(query.value("encryption_salt").toString());
    note->setEncryptionIv(query.value("encryption_iv").toString());
    note->setEncryptionTag(query.value("encryption_tag").toString());
    note->setContent(query.value("content").toString());
    return note;
}

Note* SqliteNoteRepository::getById(qint64 id, const QString &password, bool *wrongPassword) {
    if (wrongPassword) {
        *wrongPassword = false;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, typeId, title, content, is_secured, encryption_salt, encryption_iv, encryption_tag, created_at, modified_at
        FROM notes
        WHERE id = :id
    )");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "[SqliteNoteRepository::getById] Failed to query note:" << query.lastError().text();
        return nullptr;
    }

    if (!query.next()) {
        qDebug() << "[SqliteNoteRepository::getById] Note not found with ID:" << id;
        return nullptr;
    }

    Note *note = new Note(query.value("typeId").toString(), query.value("title").toString());
    note->setNoteId(query.value("id").toLongLong());
    note->setCreatedAt(toUtcDateTime(query.value("created_at")));
    note->setLastModified(toUtcDateTime(query.value("modified_at")));

    const bool secured = query.value("is_secured").toInt() == 1;
    note->setSecured(secured);
    note->setEncryptionSalt(query.value("encryption_salt").toString());
    note->setEncryptionIv(query.value("encryption_iv").toString());
    note->setEncryptionTag(query.value("encryption_tag").toString());

    const QString storedContent = query.value("content").toString();
    if (!secured) {
        note->setContent(storedContent);
        return note;
    }

    EncryptionService::DecryptResult result = EncryptionService::decrypt(
        storedContent,
        password,
        note->encryptionSalt(),
        note->encryptionIv(),
        note->encryptionTag()
    );

    if (!result.success) {
        qWarning() << "[SqliteNoteRepository::getById] Decryption failed for note ID:" << id << result.errorMessage;
        delete note;
        if (wrongPassword) {
            *wrongPassword = true;
        }
        return nullptr;
    }

    note->setContent(result.plaintext);
    return note;
}

std::vector<std::unique_ptr<Note>> SqliteNoteRepository::getAll() {
    // Retrieve all notes from the database, sorted by most recently modified first.
    // Used on app startup to populate the UI sidebar.
    // Returns move-owned unique_ptrs in a std::vector; caller takes ownership.
    qDebug() << "[SqliteNoteRepository::getAll] Starting getAll";
    std::vector<std::unique_ptr<Note>> notes;
    QSqlQuery query(db);
    query.prepare("SELECT id, typeId, title, content, is_secured, encryption_salt, encryption_iv, encryption_tag, created_at, modified_at, trashed_at FROM notes WHERE is_trashed = 0 ORDER BY modified_at DESC");

    if (!query.exec()) {
        qWarning() << "[SqliteNoteRepository::getAll] Failed to fetch all notes:" << query.lastError().text();
        return notes;
    }

    // Build a Note object for each row and push into vector as unique_ptr.
    while (query.next()) {
        auto note = std::make_unique<Note>(query.value("typeId").toString(), query.value("title").toString());
        note->setNoteId(query.value("id").toLongLong());
        note->setContent(query.value("content").toString());
        note->setCreatedAt(toUtcDateTime(query.value("created_at")));
        // For trashed listing, prefer trashed_at if available to show purge date
        QDateTime trashedAt = toUtcDateTime(query.value("trashed_at"));
        if (trashedAt.isValid()) {
            note->setLastModified(trashedAt);
        } else {
            note->setLastModified(toUtcDateTime(query.value("modified_at")));
        }
        note->setSecured(query.value("is_secured").toInt() == 1);
        note->setEncryptionSalt(query.value("encryption_salt").toString());
        note->setEncryptionIv(query.value("encryption_iv").toString());
        note->setEncryptionTag(query.value("encryption_tag").toString());
        notes.push_back(std::move(note));
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

std::vector<std::unique_ptr<Note>> SqliteNoteRepository::getTrashedNotes() {
    return getTrashedNotes(-1, 0);
}

std::vector<std::unique_ptr<Note>> SqliteNoteRepository::getTrashedNotes(int limit, int offset) {
    std::vector<std::unique_ptr<Note>> results;
    QSqlQuery query(db);

    QString sql = "SELECT id, typeId, title, content, is_secured, encryption_salt, encryption_iv, encryption_tag, created_at, modified_at, trashed_at FROM notes WHERE is_trashed = 1 ORDER BY trashed_at DESC";
    if (limit >= 0) {
        sql += " LIMIT :limit OFFSET :offset";
    }

    query.prepare(sql);
    if (limit >= 0) {
        query.bindValue(":limit", limit);
        query.bindValue(":offset", offset);
    }

    if (!query.exec()) {
        qWarning() << "[SqliteNoteRepository::getTrashedNotes] Failed to fetch trashed notes:" << query.lastError().text();
        return results;
    }

    while (query.next()) {
        auto note = std::make_unique<Note>(query.value("typeId").toString(), query.value("title").toString());
        note->setNoteId(query.value("id").toLongLong());
        note->setContent(query.value("content").toString());
        note->setCreatedAt(toUtcDateTime(query.value("created_at")));
        note->setLastModified(toUtcDateTime(query.value("modified_at")));
        note->setSecured(query.value("is_secured").toInt() == 1);
        note->setEncryptionSalt(query.value("encryption_salt").toString());
        note->setEncryptionIv(query.value("encryption_iv").toString());
        note->setEncryptionTag(query.value("encryption_tag").toString());
        results.push_back(std::move(note));
    }

    return results;
}

int SqliteNoteRepository::countTrashedNotes() {
    QSqlQuery query(db);
    if (!query.exec("SELECT COUNT(*) AS cnt FROM notes WHERE is_trashed = 1")) {
        qWarning() << "[SqliteNoteRepository::countTrashedNotes] Failed to count trashed notes:" << query.lastError().text();
        return 0;
    }

    if (query.next()) {
        return query.value("cnt").toInt();
    }

    return 0;
}

bool SqliteNoteRepository::isNoteTrashed(qint64 id) {
    QSqlQuery query(db);
    query.prepare("SELECT is_trashed FROM notes WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "[SqliteNoteRepository::isNoteTrashed] Failed to query note state:" << query.lastError().text();
        return false;
    }

    if (!query.next()) {
        return false;
    }

    return query.value("is_trashed").toInt() == 1;
}

bool SqliteNoteRepository::trashNote(qint64 id) {
    QSqlQuery query(db);
    query.prepare("UPDATE notes SET is_trashed = 1, trashed_at = CURRENT_TIMESTAMP WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "[SqliteNoteRepository::trashNote] Failed to trash note:" << query.lastError().text();
        return false;
    }

    qDebug() << "Note trashed with ID:" << id;
    return true;
}

bool SqliteNoteRepository::markNoteTrashed(qint64 id) {
    // Alias to trashNote for backwards compatibility with interface naming.
    return trashNote(id);
}

bool SqliteNoteRepository::restoreNote(qint64 id) {
    QSqlQuery query(db);
    query.prepare("UPDATE notes SET is_trashed = 0, trashed_at = NULL WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "[SqliteNoteRepository::restoreNote] Failed to restore note:" << query.lastError().text();
        return false;
    }

    qDebug() << "Note restored with ID:" << id;
    return true;
}

bool SqliteNoteRepository::purgeTrashedNotes(int olderThanDays) {
    QSqlQuery query(db);
    // Permanently delete trashed notes older than the configured retention window.
    query.prepare(R"(
        DELETE FROM notes
        WHERE is_trashed = 1
        AND trashed_at <= datetime('now', :delta)
    )");
    
    QString delta = QString("-%1 days").arg(olderThanDays);
    query.bindValue(":delta", delta);

    if (!query.exec()) {
        qWarning() << "[SqliteNoteRepository::purgeTrashedNotes] Failed to purge trashed notes:" << query.lastError().text();
        return false;
    }

    qDebug() << "Purged trashed notes older than" << olderThanDays << "days; rows affected:" << query.numRowsAffected();
    return true;
}

bool SqliteNoteRepository::update(const Note &note) {
    return const_cast<SqliteNoteRepository *>(this)->save(const_cast<Note &>(note), QString());
}

std::vector<std::unique_ptr<Note>> SqliteNoteRepository::searchByTitle(const QString &query_str) {
    // Full-text search on titles using FTS5 virtual table when available.
    // Falls back to LIKE search if FTS5 is not available.
    // Results ordered by most recently modified first (UI UX: newest edits first).
    // Returns move-owned unique_ptrs in a std::vector.
    std::vector<std::unique_ptr<Note>> results;
    QSqlQuery query(db);

    if (query_str.isEmpty()) {
        // Empty query: return all notes
        query.prepare("SELECT id, typeId, title, content, is_secured, encryption_salt, encryption_iv, encryption_tag, created_at, modified_at FROM notes ORDER BY modified_at DESC");
    } else {
        // Try FTS5 search first
        query.prepare(R"(
            SELECT n.id, n.typeId, n.title, n.content, n.is_secured, n.encryption_salt, n.encryption_iv, n.encryption_tag, n.created_at, n.modified_at
            FROM notes n
            WHERE n.is_trashed = 0
            AND n.id IN (SELECT rowid FROM notes_fts WHERE notes_fts MATCH :fts_query)
            ORDER BY n.modified_at DESC
        )");
        query.addBindValue(query_str);

        if (!query.exec() || !query.first()) {
            // FTS5 not available or no results; fall back to LIKE
            query.clear();
            query.prepare("SELECT id, typeId, title, content, is_secured, encryption_salt, encryption_iv, encryption_tag, created_at, modified_at FROM notes WHERE is_trashed = 0 AND title LIKE :query ORDER BY modified_at DESC");
            query.addBindValue("%" + query_str + "%");

            if (!query.exec()) {
                qWarning() << "Failed to search by title:" << query.lastError().text();
                return results;
            }
        }
    }

    if (!query.exec()) {
        qWarning() << "Failed to search by title:" << query.lastError().text();
        return results;
    }

    while (query.next()) {
        auto note = std::make_unique<Note>(query.value("typeId").toString(), query.value("title").toString());
        note->setNoteId(query.value("id").toLongLong());
        note->setContent(query.value("content").toString());
        note->setCreatedAt(toUtcDateTime(query.value("created_at")));
        note->setLastModified(toUtcDateTime(query.value("modified_at")));
        note->setSecured(query.value("is_secured").toInt() == 1);
        note->setEncryptionSalt(query.value("encryption_salt").toString());
        note->setEncryptionIv(query.value("encryption_iv").toString());
        note->setEncryptionTag(query.value("encryption_tag").toString());
        results.push_back(std::move(note));
    }

    qDebug() << "Found" << results.size() << "notes matching:" << query_str;
    return results;
}

std::vector<std::unique_ptr<Note>> SqliteNoteRepository::searchByContent(const QString &query_str) {
    // Case-insensitive substring search on note content using LIKE '%query%'.
    // Results ordered by most recently modified first.
    // Returns move-owned unique_ptrs in a std::vector.
    std::vector<std::unique_ptr<Note>> results;
    QSqlQuery query(db);
    query.prepare("SELECT id, typeId, title, content, is_secured, encryption_salt, encryption_iv, encryption_tag, created_at, modified_at FROM notes WHERE is_trashed = 0 AND content LIKE :query ORDER BY modified_at DESC");
    query.addBindValue("%" + query_str + "%");

    if (!query.exec()) {
        qWarning() << "Failed to search by content:" << query.lastError().text();
        return results;
    }

    while (query.next()) {
        auto note = std::make_unique<Note>(query.value("typeId").toString(), query.value("title").toString());
        note->setNoteId(query.value("id").toLongLong());
        note->setContent(query.value("content").toString());
        note->setCreatedAt(toUtcDateTime(query.value("created_at")));
        note->setLastModified(toUtcDateTime(query.value("modified_at")));
        note->setSecured(query.value("is_secured").toInt() == 1);
        note->setEncryptionSalt(query.value("encryption_salt").toString());
        note->setEncryptionIv(query.value("encryption_iv").toString());
        note->setEncryptionTag(query.value("encryption_tag").toString());
        results.push_back(std::move(note));
    }

    return results;
}

std::vector<std::unique_ptr<Note>> SqliteNoteRepository::searchByTitlePaged(const QString &query, int limit, int offset) {
    std::vector<std::unique_ptr<Note>> results;
    QSqlQuery q(db);
    QString pattern = query.trimmed().isEmpty() ? "%%" : ("%" + query + "%");
    q.prepare("SELECT id, typeId, title, content, is_secured, encryption_salt, encryption_iv, encryption_tag, created_at, modified_at FROM notes WHERE is_trashed = 0 AND title LIKE :query ORDER BY modified_at DESC, id DESC LIMIT :limit OFFSET :offset");
    q.addBindValue(pattern);
    q.addBindValue(limit);
    q.addBindValue(offset);

    if (!q.exec()) {
        qWarning() << "[SqliteNoteRepository::searchByTitlePaged] Failed to run paged title search:" << q.lastError().text();
        return results;
    }

    while (q.next()) {
        auto note = std::make_unique<Note>(q.value("typeId").toString(), q.value("title").toString());
        note->setNoteId(q.value("id").toLongLong());
        note->setContent(q.value("content").toString());
        note->setCreatedAt(toUtcDateTime(q.value("created_at")));
        note->setLastModified(toUtcDateTime(q.value("modified_at")));
        note->setSecured(q.value("is_secured").toInt() == 1);
        note->setEncryptionSalt(q.value("encryption_salt").toString());
        note->setEncryptionIv(q.value("encryption_iv").toString());
        note->setEncryptionTag(q.value("encryption_tag").toString());
        results.push_back(std::move(note));
    }

    return results;
}

int SqliteNoteRepository::countTitleMatches(const QString &query) {
    QSqlQuery q(db);
    QString pattern = query.trimmed().isEmpty() ? "%%" : ("%" + query + "%");
    q.prepare("SELECT COUNT(*) as cnt FROM notes WHERE is_trashed = 0 AND title LIKE :query");
    q.addBindValue(pattern);
    if (!q.exec()) {
        qWarning() << "[SqliteNoteRepository::countTitleMatches] Failed to count title matches:" << q.lastError().text();
        return 0;
    }
    if (q.next()) {
        return q.value("cnt").toInt();
    }
    return 0;
}

int SqliteNoteRepository::countActiveNotes() const {
    QSqlQuery q(db);
    if (!q.exec("SELECT COUNT(*) AS cnt FROM notes WHERE is_trashed = 0")) {
        qWarning() << "[SqliteNoteRepository::countActiveNotes] Failed to count notes:" << q.lastError().text();
        return 0;
    }

    if (q.next()) {
        return q.value("cnt").toInt();
    }
    return 0;
}

int SqliteNoteRepository::countActiveNotesByType(const QString &typeId) const {
    QSqlQuery q(db);
    q.prepare("SELECT COUNT(*) AS cnt FROM notes WHERE is_trashed = 0 AND typeId = :typeId");
    q.bindValue(":typeId", typeId);

    if (!q.exec()) {
        qWarning() << "[SqliteNoteRepository::countActiveNotesByType] Failed to count notes by type:" << q.lastError().text();
        return 0;
    }

    if (q.next()) {
        return q.value("cnt").toInt();
    }
    return 0;
}

// ============================================================================
// Snapshot Methods (Phase 6: FR8 - Version History)
// ============================================================================

bool SqliteNoteRepository::pruneOldSnapshots(qint64 noteId) {
    QSqlQuery query(db);
    query.prepare(R"(
        DELETE FROM snapshots
        WHERE note_id = :note_id
        AND id NOT IN (
            SELECT id FROM snapshots
            WHERE note_id = :note_id2
            ORDER BY created_at DESC, id DESC
            LIMIT 2
        )
    )");
    query.bindValue(":note_id", noteId);
    query.bindValue(":note_id2", noteId);

    if (!query.exec()) {
        qWarning() << "[SqliteNoteRepository::pruneOldSnapshots] Cleanup failed:" << query.lastError().text();
        return false;
    } else {
        if (query.numRowsAffected() > 0) {
            qDebug() << "[SqliteNoteRepository::pruneOldSnapshots] Purged" << query.numRowsAffected() << "old snapshots.";
        }
    }

    return true;
}

bool SqliteNoteRepository::saveSnapshot(Snapshot &snapshot) {
    return saveSnapshot(snapshot, QString());
}

bool SqliteNoteRepository::saveSnapshot(Snapshot &snapshot, const QString &password) {
    qDebug() << "[SqliteNoteRepository::saveSnapshot] Saving snapshot for note ID:" << snapshot.noteId();

    // Keep insert/update + pruning in one atomic unit to avoid lock contention
    // and ensure snapshot limit enforcement can't diverge from persisted state.
    if (!db.transaction()) {
        qWarning() << "[SqliteNoteRepository::saveSnapshot] Failed to begin transaction:" << db.lastError().text();
        return false;
    }

    QString contentToPersist = snapshot.content();
    QString saltToPersist = snapshot.encryptionSalt();
    QString ivToPersist = snapshot.encryptionIv();
    QString tagToPersist = snapshot.encryptionTag();

    if (snapshot.isSecured()) {
        // If a password was provided, encrypt the snapshot content now.
        // Otherwise, if encryption metadata already exists on the snapshot,
        // assume the content is already encrypted and persist as-is.
        if (!password.trimmed().isEmpty()) {
            EncryptionService::EncryptedPayload payload = EncryptionService::encrypt(snapshot.content(), password);
            if (payload.ciphertextBase64.isEmpty() || payload.saltBase64.isEmpty() || payload.ivBase64.isEmpty() || payload.tagBase64.isEmpty()) {
                qWarning() << "[SqliteNoteRepository::saveSnapshot] Snapshot encryption failed";
                return false;
            }

            contentToPersist = payload.ciphertextBase64;
            saltToPersist = payload.saltBase64;
            ivToPersist = payload.ivBase64;
            tagToPersist = payload.tagBase64;
            snapshot.setEncryptionSalt(saltToPersist);
            snapshot.setEncryptionIv(ivToPersist);
            snapshot.setEncryptionTag(tagToPersist);
        } else {
            // No password provided: expect encryption metadata to already be present
            // and the content to be the already-encrypted blob.
            if (saltToPersist.isEmpty() || ivToPersist.isEmpty() || tagToPersist.isEmpty()) {
                qWarning() << "[SqliteNoteRepository::saveSnapshot] Secured snapshot missing encryption metadata and no password provided";
                return false;
            }
            // contentToPersist is left as-is (assumed encrypted)
        }
    } else {
        snapshot.setEncryptionSalt(QString());
        snapshot.setEncryptionIv(QString());
        snapshot.setEncryptionTag(QString());
    }

    QSqlQuery query(db);
    bool ok = false;

    if (snapshot.snapshotId() == -1) {
        query.prepare(R"(
            INSERT INTO snapshots (note_id, title, content, is_secured, encryption_salt, encryption_iv, encryption_tag, created_at)
            VALUES (:note_id, :title, :content, :is_secured, :encryption_salt, :encryption_iv, :encryption_tag, :created_at)
        )");
        query.bindValue(":note_id", snapshot.noteId());
        query.bindValue(":title", snapshot.title());
        query.bindValue(":content", contentToPersist);
        query.bindValue(":is_secured", snapshot.isSecured() ? 1 : 0);
        query.bindValue(":encryption_salt", saltToPersist);
        query.bindValue(":encryption_iv", ivToPersist);
        query.bindValue(":encryption_tag", tagToPersist);
        query.bindValue(":created_at", snapshot.createdAt());

        if (!query.exec()) {
            qWarning() << "[SqliteNoteRepository::saveSnapshot] Failed to insert snapshot:" << query.lastError().text();
            db.rollback();
            return false;
        }

        snapshot.setSnapshotId(query.lastInsertId().toLongLong());
        qDebug() << "[SqliteNoteRepository::saveSnapshot] Snapshot saved with ID:" << snapshot.snapshotId();
        ok = true;
    } else {
        query.prepare(R"(
            UPDATE snapshots
            SET note_id = :note_id,
                title = :title,
                content = :content,
                is_secured = :is_secured,
                encryption_salt = :encryption_salt,
                encryption_iv = :encryption_iv,
                encryption_tag = :encryption_tag,
                created_at = :created_at
            WHERE id = :id
        )");
        query.bindValue(":note_id", snapshot.noteId());
        query.bindValue(":title", snapshot.title());
        query.bindValue(":content", contentToPersist);
        query.bindValue(":is_secured", snapshot.isSecured() ? 1 : 0);
        query.bindValue(":encryption_salt", saltToPersist);
        query.bindValue(":encryption_iv", ivToPersist);
        query.bindValue(":encryption_tag", tagToPersist);
        query.bindValue(":created_at", snapshot.createdAt());
        query.bindValue(":id", snapshot.snapshotId());

        if (!query.exec()) {
            qWarning() << "[SqliteNoteRepository::saveSnapshot] Failed to update snapshot:" << query.lastError().text();
            db.rollback();
            return false;
        }

        qDebug() << "[SqliteNoteRepository::saveSnapshot] Snapshot updated with ID:" << snapshot.snapshotId();
        ok = true;
    }

    if (ok) {
        // Finalize the previous statement before committing.
        query.finish();
        query.clear();

        // Do NOT prune snapshots automatically here. Pruning should be an
        // explicit operation invoked by the service layer so callers can
        // control ordering (e.g. when creating safety snapshots before revert).
        if (!db.commit()) {
            qWarning() << "[SqliteNoteRepository::saveSnapshot] Failed to commit transaction:" << db.lastError().text();
            db.rollback();
            return false;
        }
        return true;
    }

    db.rollback();
    return false;
}

std::vector<std::unique_ptr<Snapshot>> SqliteNoteRepository::getSnapshotsByNoteId(qint64 noteId) {
    // Retrieve all snapshots for a given note, ordered by creation time descending (newest first).
    // Used by the snapshot list UI.
    // Caller assumes ownership of all Snapshot* pointers; must delete them.
    qDebug() << "[SqliteNoteRepository::getSnapshotsByNoteId] Fetching snapshots for note ID:" << noteId;
    
    std::vector<std::unique_ptr<Snapshot>> snapshots;
    QSqlQuery query(db);
    query.prepare("SELECT id, note_id, title, content, is_secured, encryption_salt, encryption_iv, encryption_tag, created_at FROM snapshots WHERE note_id = :note_id ORDER BY created_at DESC");
    query.addBindValue(noteId);
    
    if (!query.exec()) {
        qWarning() << "[SqliteNoteRepository::getSnapshotsByNoteId] Failed to query snapshots:" << query.lastError().text();
        return snapshots;
    }
    
    while (query.next()) {
        auto snapshot = std::make_unique<Snapshot>(
            query.value("note_id").toLongLong(),
            query.value("title").toString(),
            query.value("content").toString()
        );
        snapshot->setSnapshotId(query.value("id").toLongLong());
        snapshot->setCreatedAt(toUtcDateTime(query.value("created_at")));
        snapshot->setSecured(query.value("is_secured").toInt() == 1);
        snapshot->setEncryptionSalt(query.value("encryption_salt").toString());
        snapshot->setEncryptionIv(query.value("encryption_iv").toString());
        snapshot->setEncryptionTag(query.value("encryption_tag").toString());
        snapshots.push_back(std::move(snapshot));
    }
    
    qDebug() << "[SqliteNoteRepository::getSnapshotsByNoteId] Fetched" << snapshots.size() << "snapshots for note ID:" << noteId;
    return snapshots;
}

std::vector<std::unique_ptr<Snapshot>> SqliteNoteRepository::getSnapshotsByNoteId(qint64 noteId, const QString &password) {
    Q_UNUSED(password);
    // For listing purposes we return the same metadata list; decryption is handled by getSnapshotById when needed.
    return getSnapshotsByNoteId(noteId);
}

bool SqliteNoteRepository::deleteSnapshotById(qint64 snapshotId) {
    // Delete a single snapshot by ID.
    // Used when user explicitly deletes a snapshot from the history UI.
    qDebug() << "[SqliteNoteRepository::deleteSnapshotById] Deleting snapshot ID:" << snapshotId;
    
    QSqlQuery query(db);
    query.prepare("DELETE FROM snapshots WHERE id = :id");
    query.addBindValue(snapshotId);
    
    if (!query.exec()) {
        qWarning() << "[SqliteNoteRepository::deleteSnapshotById] Failed to delete snapshot:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "[SqliteNoteRepository::deleteSnapshotById] Snapshot deleted with ID:" << snapshotId;
    return true;
}

std::unique_ptr<Snapshot> SqliteNoteRepository::getSnapshotById(qint64 snapshotId) {
    QSqlQuery query(db);
    query.prepare("SELECT id, note_id, title, content, is_secured, encryption_salt, encryption_iv, encryption_tag, created_at FROM snapshots WHERE id = :id");
    query.bindValue(":id", snapshotId);

    if (!query.exec()) {
        qWarning() << "[SqliteNoteRepository::getSnapshotById] Failed to query snapshot metadata:" << query.lastError().text();
        return nullptr;
    }

    if (!query.next()) {
        qDebug() << "[SqliteNoteRepository::getSnapshotById] Snapshot not found with ID:" << snapshotId;
        return nullptr;
    }

    auto snapshot = std::make_unique<Snapshot>(
        query.value("note_id").toLongLong(),
        query.value("title").toString(),
        query.value("content").toString()
    );
    snapshot->setSnapshotId(query.value("id").toLongLong());
    snapshot->setCreatedAt(toUtcDateTime(query.value("created_at")));
    snapshot->setSecured(query.value("is_secured").toInt() == 1);
    snapshot->setEncryptionSalt(query.value("encryption_salt").toString());
    snapshot->setEncryptionIv(query.value("encryption_iv").toString());
    snapshot->setEncryptionTag(query.value("encryption_tag").toString());
    return snapshot;
}

std::unique_ptr<Snapshot> SqliteNoteRepository::getSnapshotById(qint64 snapshotId, const QString &password, bool *wrongPassword) {
    if (wrongPassword) {
        *wrongPassword = false;
    }

    // Fetch a single snapshot by ID. Returns nullptr if not found or on error.
    // Caller assumes ownership and must delete the returned pointer.
    QSqlQuery query(db);
    query.prepare("SELECT id, note_id, title, content, is_secured, encryption_salt, encryption_iv, encryption_tag, created_at FROM snapshots WHERE id = :id");
    query.bindValue(":id", snapshotId);

    if (!query.exec()) {
        qWarning() << "[SqliteNoteRepository::getSnapshotById] Failed to query snapshot:" << query.lastError().text();
        return nullptr;
    }

    if (!query.next()) {
        qDebug() << "[SqliteNoteRepository::getSnapshotById] Snapshot not found with ID:" << snapshotId;
        return nullptr;
    }

    auto snapshot = std::make_unique<Snapshot>(
        query.value("note_id").toLongLong(),
        query.value("title").toString(),
        query.value("content").toString()
    );
    snapshot->setSnapshotId(query.value("id").toLongLong());
    snapshot->setCreatedAt(toUtcDateTime(query.value("created_at")));
    snapshot->setSecured(query.value("is_secured").toInt() == 1);
    snapshot->setEncryptionSalt(query.value("encryption_salt").toString());
    snapshot->setEncryptionIv(query.value("encryption_iv").toString());
    snapshot->setEncryptionTag(query.value("encryption_tag").toString());

    if (!snapshot->isSecured()) {
        return snapshot;
    }

    EncryptionService::DecryptResult result = EncryptionService::decrypt(
        snapshot->content(),
        password,
        snapshot->encryptionSalt(),
        snapshot->encryptionIv(),
        snapshot->encryptionTag()
    );

    if (!result.success) {
        qWarning() << "[SqliteNoteRepository::getSnapshotById] Decryption failed for snapshot ID:" << snapshotId << result.errorMessage;
        if (wrongPassword) {
            *wrongPassword = true;
        }
        return nullptr;
    }

    snapshot->setContent(result.plaintext);

    return snapshot;
}

bool SqliteNoteRepository::deleteOldestSnapshotForNote(qint64 noteId) {
    // Delete the oldest snapshot for a given note.
    // Called when a 3rd snapshot is created (max 2 per note per FR8 requirement).
    qDebug() << "[SqliteNoteRepository::deleteOldestSnapshotForNote] Deleting oldest snapshot for note ID:" << noteId;
    
    QSqlQuery query(db);
    query.prepare(R"(
        DELETE FROM snapshots
        WHERE id = (
            SELECT id FROM snapshots
            WHERE note_id = :note_id
            ORDER BY created_at ASC
            LIMIT 1
        )
    )");
    query.addBindValue(noteId);
    
    if (!query.exec()) {
        qWarning() << "[SqliteNoteRepository::deleteOldestSnapshotForNote] Failed to delete oldest snapshot:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "[SqliteNoteRepository::deleteOldestSnapshotForNote] Oldest snapshot deleted for note ID:" << noteId;
    return true;
}
