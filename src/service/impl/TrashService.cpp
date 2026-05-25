#include "TrashService.h"

#include "../../model/Note.h"
#include "../../repository/INoteRepository.h"

#include <QDebug>
#include <memory>
#include <vector>

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

std::vector<std::unique_ptr<Note>> TrashService::getTrashedNotes(int limit, int offset) {
    if (!m_repository) {
        return {};
    }

    // Forward the RAII-owned vector from the repository directly. This avoids
    // copying or re-wrapping unique_ptrs and preserves move-only semantics.
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
