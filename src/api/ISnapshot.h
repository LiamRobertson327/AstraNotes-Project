#ifndef ISNAPSHOT_H
#define ISNAPSHOT_H

#include <QString>
#include <QDateTime>
#include <QtGlobal>

// ISnapshot: Interface for note version snapshots (FR8)
// A snapshot captures the state of a note at a specific point in time.
// Each note can have at most 2 snapshots; when a 3rd is created, the oldest is deleted.
// Snapshots respect encryption: private-note snapshots remain encrypted until decrypted.
class ISnapshot {
public:
    virtual ~ISnapshot() = default;

    // Accessors
    virtual qint64 snapshotId() const = 0;           // Unique snapshot ID
    virtual qint64 noteId() const = 0;               // Parent note ID
    virtual QString content() const = 0;              // Snapshot of note content at time of creation
    virtual QString title() const = 0;                // Snapshot of note title at time of creation
    virtual QDateTime createdAt() const = 0;         // When this snapshot was created (immutable)

    // Mutators
    virtual void setSnapshotId(qint64 newId) = 0;
    virtual void setNoteId(qint64 newNoteId) = 0;
    virtual void setContent(const QString& newContent) = 0;
    virtual void setTitle(const QString& newTitle) = 0;
    virtual void setCreatedAt(const QDateTime& dt) = 0;
};

#endif // ISNAPSHOT_H
