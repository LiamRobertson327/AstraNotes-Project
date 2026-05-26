#ifndef NOTESERVICE_H
#define NOTESERVICE_H

#include "../interfaces/INoteService.h"
#include <QString>
#include <vector>
#include <memory>

class Note;
class INoteRepository;

/// NoteService implements INoteService and handles core note lifecycle:
/// load, save, and create. Snapshot and trash operations are delegated to
/// SnapshotService and TrashService respectively.
class NoteService : public INoteService {
public:
    static constexpr int kMaxTitleCharacters = 500;
    static constexpr qint64 kMaxContentBytes = 10LL * 1024 * 1024;

    explicit NoteService(INoteRepository *repository);

    // INoteService implementation
    std::unique_ptr<Note> loadNote(qint64 noteId, const QString &password = QString(),
                                   bool *wrongPassword = nullptr, QString *errorMessage = nullptr) override;
    bool saveNote(Note &note, const QString &password,
                  QString *errorMessage = nullptr) override;
    std::unique_ptr<Note> createNote(const QString &typeId, const QString &title) override;

    // Storage helpers for UI
    bool isConnected() override;
    int countActiveNotes() override;
    int countActiveNotesByType(const QString &typeId) override;
    std::vector<std::unique_ptr<Note>> searchByTitlePaged(const QString &query, int pageSize, int offset) override;

    bool trashNote(qint64 noteId) override;

    // Robust loader: indicates when a password is required without performing UI prompts.
    std::unique_ptr<Note> loadNoteRobust(qint64 noteId, const QString &password, bool *needsPassword = nullptr,
                                         bool *wrongPassword = nullptr, QString *errorMessage = nullptr) override;

private:
    INoteRepository *m_repository;
};

#endif // NOTESERVICE_H