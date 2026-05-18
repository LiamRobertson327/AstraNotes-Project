#include "SnapshotService.h"

#include "../../model/Note.h"
#include "../../model/Snapshot.h"
#include "../../repository/INoteRepository.h"

#include <QDebug>

SnapshotService::SnapshotService(INoteRepository *repository)
    : m_repository(repository) {}

bool SnapshotService::saveSnapshot(const Note &note, const QString &password, QString *errorMessage) {
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
            // Reuse primary note encryption metadata
            saved = m_repository->saveSnapshot(snapshot, QString());
        } else {
            // First encryption; derive key from password
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

QVector<Snapshot *> SnapshotService::getSnapshotsByNoteId(qint64 noteId) {
    if (!m_repository) {
        return QVector<Snapshot *>();
    }
    return m_repository->getSnapshotsByNoteId(noteId);
}

Snapshot *SnapshotService::getSnapshotById(qint64 snapshotId, const QString &password, bool *wrongPassword) {
    if (!m_repository) {
        return nullptr;
    }
    return m_repository->getSnapshotById(snapshotId, password, wrongPassword);
}

bool SnapshotService::deleteSnapshot(qint64 snapshotId) {
    if (!m_repository) {
        return false;
    }
    return m_repository->deleteSnapshotById(snapshotId);
}

bool SnapshotService::deleteOldestSnapshotForNote(qint64 noteId) {
    if (!m_repository) {
        return false;
    }
    return m_repository->deleteOldestSnapshotForNote(noteId);
}

void SnapshotService::enforceSnapshotLimit(qint64 noteId, int maxSnapshots) {
    if (!m_repository) {
        return;
    }
    m_repository->pruneOldSnapshots(noteId);
}

Snapshot *SnapshotService::revertToSnapshot(Note &currentNote, qint64 snapshotId, const QString &password, QString *errorMessage) {
    if (!m_repository) {
        if (errorMessage) *errorMessage = "Repository unavailable";
        return nullptr;
    }

    // Fetch the target snapshot before creating a safety snapshot.
    // The repository prunes older snapshots when a new one is saved, so saving
    // the safety snapshot first can delete the very snapshot we are trying to restore.
    bool wrongPassword = false;
    Snapshot *target = m_repository->getSnapshotById(snapshotId, password, &wrongPassword);
    if (!target) {
        if (wrongPassword) {
            if (errorMessage) *errorMessage = "Incorrect password for snapshot";
        } else if (errorMessage) {
            *errorMessage = "Snapshot not found or failed to load";
        }
        return nullptr;
    }

    // Create safety snapshot of current state after the target has been loaded.
    Snapshot safety(currentNote.noteId(), currentNote.title(), currentNote.content());
    safety.setSecured(currentNote.isSecured());
    if (currentNote.isSecured()) {
        safety.setEncryptionSalt(currentNote.encryptionSalt());
        safety.setEncryptionIv(currentNote.encryptionIv());
        safety.setEncryptionTag(currentNote.encryptionTag());
    }

    bool safetySaved = false;
    if (safety.isSecured()) {
        if (!safety.encryptionSalt().isEmpty()) {
            safetySaved = m_repository->saveSnapshot(safety, QString());
        } else {
            safetySaved = m_repository->saveSnapshot(safety, password);
        }
    } else {
        safetySaved = m_repository->saveSnapshot(safety, QString());
    }

    if (!safetySaved) {
        qWarning() << "[SnapshotService::revertToSnapshot] Failed to create safety snapshot";
        // non-fatal; the target snapshot is already loaded in memory.
    }

    return target;
}
