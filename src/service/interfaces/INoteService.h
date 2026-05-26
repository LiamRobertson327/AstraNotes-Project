#ifndef INOTESERVICE_H
#define INOTESERVICE_H

#include <QString>
#include <QVector>
#include <vector>
#include <memory>

class Note;

/// INoteService defines the interface for core note lifecycle operations.
/// This interface decouples the UI (MainWindow) from implementation details,
/// allowing for easier testing and future refactoring.
class INoteService {
public:
    virtual ~INoteService() = default;

    /// Load a note by its ID. If password is empty, load unencrypted notes.
    /// Returns a new Note object (caller owns); nullptr on failure.
    /// @param noteId           Note ID to load
    /// @param password         Password for encrypted notes (empty for plaintext)
    /// @param wrongPassword    Set to true if decryption failed due to wrong password
    /// @param errorMessage     User-facing error message if load fails
    virtual std::unique_ptr<Note> loadNote(qint64 noteId, const QString &password = QString(),
                                           bool *wrongPassword = nullptr, QString *errorMessage = nullptr) = 0;

    /// Save or update a note. Handles both plaintext and encrypted saves.
    /// @param note             Note to save (will update note ID and timestamps)
    /// @param password         Password for encryption (empty for plaintext)
    /// @param errorMessage     User-facing error message if save fails
    virtual bool saveNote(Note &note, const QString &password,
                          QString *errorMessage = nullptr) = 0;

    /// Create a new note with default values. Caller owns the returned Note.
    /// @param typeId           Plugin format ID (e.g., "plaintext", "markdown")
    /// @param title            Initial note title
    virtual std::unique_ptr<Note> createNote(const QString &typeId, const QString &title) = 0;

    /// Check if underlying storage is connected/available
    virtual bool isConnected() = 0;

    /// Counts and paging helpers used by UI
    virtual int countActiveNotes() = 0;
    virtual int countActiveNotesByType(const QString &typeId) = 0;
    virtual std::vector<std::unique_ptr<Note>> searchByTitlePaged(const QString &query, int pageSize, int offset) = 0;
    virtual bool trashNote(qint64 noteId) = 0;

    /// Robust loader that can indicate when password input is needed.
    virtual std::unique_ptr<Note> loadNoteRobust(qint64 noteId, const QString &password,
                                                 bool *needsPassword = nullptr,
                                                 bool *wrongPassword = nullptr,
                                                 QString *errorMessage = nullptr) = 0;
};

#endif // INOTESERVICE_H
