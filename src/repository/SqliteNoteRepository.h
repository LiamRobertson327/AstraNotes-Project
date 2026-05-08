#ifndef SQLITENOTEPOSITORY_H
#define SQLITENOTEPOSITORY_H

#include "INoteRepository.h"
#include <QSqlDatabase>

class SqliteNoteRepository : public INoteRepository {
public:
    SqliteNoteRepository(const QString &dbPath = "notes.db");
    ~SqliteNoteRepository();

    bool save(Note &note) override;
    Note* getById(qint64 id) override;
    QVector<Note*> getAll() override;
    bool deleteById(qint64 id) override;
    bool update(const Note &note) override;

    QVector<Note*> searchByTitle(const QString &query) override;
    QVector<Note*> searchByContent(const QString &query) override;

    bool isConnected() const { return db.isOpen(); }

private:
    QSqlDatabase db;
    bool initializeDatabase();
    bool createTablesIfNeeded();
};

#endif // SQLITENOTEPOSITORY_H
