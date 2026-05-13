#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include <QString>
#include <QDateTime>
#include <QtGlobal>
#include "../api/ISnapshot.h"

class Snapshot : public ISnapshot {
public:
    Snapshot(qint64 noteId, const QString& title, const QString& content);

    // Accessors
    virtual qint64 snapshotId() const override { return m_snapshotId; }
    virtual qint64 noteId() const override { return m_noteId; }
    virtual QString content() const override { return m_content; }
    virtual QString title() const override { return m_title; }
    virtual QDateTime createdAt() const override { return m_createdAt; }

    // Mutators
    virtual void setSnapshotId(qint64 newId) override;
    virtual void setNoteId(qint64 newNoteId) override;
    virtual void setContent(const QString& newContent) override;
    virtual void setTitle(const QString& newTitle) override;
    virtual void setCreatedAt(const QDateTime& dt) override;

    bool isSecured() const { return m_isSecured; }
    QString encryptionSalt() const { return m_encryptionSalt; }
    QString encryptionIv() const { return m_encryptionIv; }
    QString encryptionTag() const { return m_encryptionTag; }

    void setSecured(bool secured);
    void setEncryptionSalt(const QString &salt);
    void setEncryptionIv(const QString &iv);
    void setEncryptionTag(const QString &tag);
    QString displayText() const;

private:
    qint64 m_snapshotId = -1;          // -1 means not yet persisted
    qint64 m_noteId = -1;              // Parent note ID
    QString m_title;                   // Snapshot of title at creation time
    QString m_content;                 // Snapshot of content at creation time
    QDateTime m_createdAt;             // Immutable creation timestamp
    bool m_isSecured = false;
    QString m_encryptionSalt;
    QString m_encryptionIv;
    QString m_encryptionTag;
};

#endif // SNAPSHOT_H
