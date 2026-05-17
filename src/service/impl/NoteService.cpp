#include "NoteService.h"

#include "../../model/Note.h"
#include "../../repository/SqliteNoteRepository.h"

#include <QDebug>
#include <QVector>

NoteService::NoteService(SqliteNoteRepository *repository)
    : m_repository(repository) {}

Note *NoteService::loadNote(qint64 noteId, const QString &password, bool *wrongPassword, QString *errorMessage) {
    // Forward to the robust loader to centralize password-detection
    // and error reporting behavior.
    bool needsPassword = false;
    return loadNoteRobust(noteId, password, &needsPassword, wrongPassword, errorMessage);
}

bool NoteService::saveNote(Note &note, const QString &password, QString *errorMessage) {
    if (!m_repository) {
        if (errorMessage) {
            *errorMessage = "Note repository is unavailable";
        }
        return false;
    }

    if (!m_repository->save(note, note.isSecured() ? password : QString())) {
        if (errorMessage) {
            *errorMessage = "Failed to save note";
        }
        return false;
    }
    return true;
}

Note *NoteService::createNote(const QString &typeId, const QString &title) {
    Note *newNote = new Note(typeId, title);
    return newNote;
}

bool NoteService::isConnected() {
    return m_repository ? m_repository->isConnected() : false;
}

int NoteService::countActiveNotes() {
    return m_repository ? m_repository->countActiveNotes() : 0;
}

int NoteService::countActiveNotesByType(const QString &typeId) {
    return m_repository ? m_repository->countActiveNotesByType(typeId) : 0;
}

QVector<Note*> NoteService::searchByTitlePaged(const QString &query, int pageSize, int offset) {
    return m_repository ? m_repository->searchByTitlePaged(query, pageSize, offset) : QVector<Note*>();
}

bool NoteService::trashNote(qint64 noteId) {
    if (!m_repository) return false;
    return m_repository->trashNote(noteId);
}

Note *NoteService::loadNoteRobust(qint64 noteId, const QString &password, bool *needsPassword, bool *wrongPassword, QString *errorMessage) {
    if (!m_repository) {
        if (errorMessage) *errorMessage = "Note repository is unavailable";
        return nullptr;
    }

    // If no password was provided, fetch metadata and detect if password is required.
    if (password.isEmpty()) {
        Note *note = m_repository->getById(noteId);
        if (!note) {
            if (errorMessage) *errorMessage = "Note not found";
            return nullptr;
        }
        if (needsPassword) {
            *needsPassword = note->isSecured();
        }
        return note;
    }

    // Password provided: attempt to load and decrypt.
    return m_repository->getById(noteId, password, wrongPassword);
}