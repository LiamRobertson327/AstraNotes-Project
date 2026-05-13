#include "NoteService.h"

#include "../model/Note.h"
#include "../model/Snapshot.h"
#include "../repository/SqliteNoteRepository.h"

#include <QDebug>

NoteService::NoteService(SqliteNoteRepository *repository)
    : m_repository(repository) {}

Note *NoteService::loadNote(qint64 noteId, const QString &password, bool *wrongPassword, QString *errorMessage) {
    if (!m_repository) {
        if (errorMessage) {
            *errorMessage = "Note repository is unavailable";
        }
        return nullptr;
    }

    if (password.isEmpty()) {
        return m_repository->getById(noteId);
    }

    return m_repository->getById(noteId, password, wrongPassword);
}

bool NoteService::saveNote(Note &note, const QString &password, bool createSnapshot, QString *errorMessage) {
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

    if (createSnapshot && !saveSnapshotForNote(note, QString(), errorMessage)) {
        return false;
    }

    return true;
}

bool NoteService::saveSnapshotForNote(const Note &note, const QString &password, QString *errorMessage) {
    if (!m_repository) {
        if (errorMessage) {
            *errorMessage = "Note repository is unavailable";
        }
        return false;
    }

    Snapshot snapshot(note.noteId(), note.title(), note.content());
    snapshot.setSecured(note.isSecured());

    if (note.isSecured()) {
        snapshot.setEncryptionSalt(note.encryptionSalt());
        snapshot.setEncryptionIv(note.encryptionIv());
        snapshot.setEncryptionTag(note.encryptionTag());
    }

    bool saved = false;
    if (note.isSecured()) {
        if (!note.encryptionSalt().isEmpty()) {
            saved = m_repository->saveSnapshot(snapshot, QString());
        } else {
            saved = m_repository->saveSnapshot(snapshot, password);
        }
    } else {
        saved = m_repository->saveSnapshot(snapshot, QString());
    }

    if (!saved && errorMessage) {
        *errorMessage = "Failed to save snapshot";
    }

    return saved;
}