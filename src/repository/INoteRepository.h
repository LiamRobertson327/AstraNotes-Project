#ifndef INOTEREPOSITORY_H
#define INOTEREPOSITORY_H

#include "../model/Note.h"
#include <QVector>

class INoteRepository {
public:
    virtual ~INoteRepository() = default;

    // CRUD operations
    virtual bool save(Note &note) = 0;              // Insert or update; updates note.id on insert
    virtual Note* getById(qint64 id) = 0;           // Returns nullptr if not found
    virtual QVector<Note*> getAll() = 0;            // Get all notes
    virtual bool deleteById(qint64 id) = 0;         // Soft or hard delete
    virtual bool update(const Note &note) = 0;      // Update existing note

    // Search operations
    virtual QVector<Note*> searchByTitle(const QString &query) = 0;
    virtual QVector<Note*> searchByContent(const QString &query) = 0;
};

#endif // INOTEREPOSITORY_H
