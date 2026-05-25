#include "NoteService.h"

#include "../../model/Note.h"
#include "../../repository/INoteRepository.h"

#include <QDebug>
#include <QVector>

NoteService::NoteService(INoteRepository *repository)
    : m_repository(repository) {}

std::unique_ptr<Note> NoteService::loadNote(qint64 noteId, const QString &password, bool *wrongPassword, QString *errorMessage) {
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

    if (note.title().size() > kMaxTitleCharacters) {
        if (errorMessage) {
            *errorMessage = QString("Title exceeds the maximum of %1 characters").arg(kMaxTitleCharacters);
        }
        return false;
    }

    if (note.content().toUtf8().size() > kMaxContentBytes) {
        if (errorMessage) {
            *errorMessage = QString("Content exceeds the maximum of %1 bytes").arg(kMaxContentBytes);
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

std::unique_ptr<Note> NoteService::createNote(const QString &typeId, const QString &title) {
    return std::make_unique<Note>(typeId, title);
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

std::vector<std::unique_ptr<Note>> NoteService::searchByTitlePaged(const QString &query, int pageSize, int offset) {
    return m_repository ? m_repository->searchByTitlePaged(query, pageSize, offset) : std::vector<std::unique_ptr<Note>>();
}

bool NoteService::trashNote(qint64 noteId) {
    if (!m_repository) return false;
    return m_repository->trashNote(noteId);
}

std::unique_ptr<Note> NoteService::loadNoteRobust(qint64 noteId, const QString &password, bool *needsPassword, bool *wrongPassword, QString *errorMessage) {
    if (!m_repository) {
        if (errorMessage) *errorMessage = "Note repository is unavailable";
        return nullptr;
    }

    // If no password was provided, fetch metadata and detect if password is required.
    if (password.isEmpty()) {
        std::unique_ptr<Note> note = m_repository->getById(noteId);
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