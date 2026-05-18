#include "TrashService.h"

#include "../../model/Note.h"
#include "../../repository/INoteRepository.h"

#include <QDebug>

TrashService::TrashService(INoteRepository *repository)
    : m_repository(repository) {}

bool TrashService::trashNote(qint64 noteId) {
    if (!m_repository) {
        qWarning() << "[TrashService::trashNote] Repository is unavailable";
        return false;
    }
    return m_repository->trashNote(noteId);
}

bool TrashService::restoreNote(qint64 noteId) {
    if (!m_repository) {
        qWarning() << "[TrashService::restoreNote] Repository is unavailable";
        return false;
    }
    return m_repository->restoreNote(noteId);
}

bool TrashService::purgeNote(qint64 noteId) {
    if (!m_repository) {
        qWarning() << "[TrashService::purgeNote] Repository is unavailable";
        return false;
    }

    // Check if note exists and is trashed before purging
    Note *note = m_repository->getById(noteId);
    if (!note) {
        return false; // Note doesn't exist or is already purged
    }

    // Verify the note is actually in the trashed state by scanning trashed notes
    bool isTrashed = false;
    QVector<Note *> trashed = m_repository->getTrashedNotes();
    for (Note *t : trashed) {
        if (t->noteId() == noteId) {
            isTrashed = true;
            break;
        }
    }

    // Clean up temporary trashed note objects
    for (Note *t : trashed) {
        delete t;
    }

    if (!isTrashed) {
        delete note;
        return false; // Do not allow purging of non-trashed notes
    }

    // Perform permanent deletion via repository helper
    bool deleted = m_repository->deleteById(noteId);
    delete note;
    return deleted;
}

void TrashService::purgeOldTrashedNotes(int retentionDays) {
    if (!m_repository) {
        qWarning() << "[TrashService::purgeOldTrashedNotes] Repository is unavailable";
        return;
    }
    m_repository->purgeTrashedNotes(retentionDays);
}

QVector<Note *> TrashService::getTrashedNotes(int limit, int offset) {
    if (!m_repository) {
        return QVector<Note *>();
    }

    QVector<Note *> trashedNotes = m_repository->getTrashedNotes();

    // Apply pagination: limit and offset
    QVector<Note *> result;
    for (int i = offset; i < trashedNotes.size() && result.size() < limit; ++i) {
        result.append(trashedNotes.at(i));
    }

    // Clean up notes that were not included in the result
    for (int i = 0; i < trashedNotes.size(); ++i) {
        if (!result.contains(trashedNotes.at(i))) {
            delete trashedNotes.at(i);
        }
    }

    return result;
}

int TrashService::countTrashedNotes() {
    if (!m_repository) {
        return 0;
    }
    QVector<Note *> trashedNotes = m_repository->getTrashedNotes();
    int count = trashedNotes.size();
    for (Note *note : trashedNotes) {
        delete note;
    }
    return count;
}

bool TrashService::isNoteTrashed(qint64 noteId) {
    if (!m_repository) {
        return false;
    }

    // Check if the note exists in the trashed notes list
    QVector<Note *> trashedNotes = m_repository->getTrashedNotes();
    bool found = false;
    for (Note *note : trashedNotes) {
        if (note->noteId() == noteId) {
            found = true;
            break;
        }
    }

    // Clean up
    for (Note *note : trashedNotes) {
        delete note;
    }

    return found;
}
