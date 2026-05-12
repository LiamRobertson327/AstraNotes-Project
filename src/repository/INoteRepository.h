#ifndef INOTEREPOSITORY_H
#define INOTEREPOSITORY_H

#include "../model/Note.h"
#include <QVector>

// INoteRepository: Persistence abstraction for INote implementations.
// All methods work with INote-based models (currently Note; future VoiceNote, etc.).
// Implementations (e.g., SqliteNoteRepository) handle database specifics.
// NOTE: Caller assumes ownership of returned Note* pointers; must delete.
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

    // getAll(): Retrieve all notes, ordered by most recently modified first.
    //           Returns empty QVector if none exist or on error.
    //           Caller assumes ownership of all Note* pointers in the vector.
    virtual QVector<Note*> getAll() = 0;

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
    //                    Caller assumes ownership of returned Note* pointers.
    virtual QVector<Note*> searchByContent(const QString &query) = 0;
};

#endif // INOTEREPOSITORY_H
