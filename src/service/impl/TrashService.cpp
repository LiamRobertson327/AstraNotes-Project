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

    if (!m_repository->isNoteTrashed(noteId)) {
        return false;
    }

    return m_repository->deleteById(noteId);
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

    return m_repository->getTrashedNotes(limit, offset);
}

int TrashService::countTrashedNotes() {
    if (!m_repository) {
        return 0;
    }

    return m_repository->countTrashedNotes();
}

bool TrashService::isNoteTrashed(qint64 noteId) {
    if (!m_repository) {
        return false;
    }

    return m_repository->isNoteTrashed(noteId);
}
