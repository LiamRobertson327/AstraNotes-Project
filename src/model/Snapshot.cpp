#include "model/Snapshot.h"
#include <QDebug>

Snapshot::Snapshot(qint64 noteId, const QString& title, const QString& content)
    : m_noteId(noteId), m_title(title), m_content(content) {
    m_createdAt = QDateTime::currentDateTime();
}

void Snapshot::setSnapshotId(qint64 newId) {
    m_snapshotId = newId;
}

void Snapshot::setNoteId(qint64 newNoteId) {
    m_noteId = newNoteId;
}

void Snapshot::setContent(const QString& newContent) {
    m_content = newContent;
}

void Snapshot::setTitle(const QString& newTitle) {
    m_title = newTitle;
}

void Snapshot::setCreatedAt(const QDateTime& dt) {
    m_createdAt = dt;
}

void Snapshot::setSecured(bool secured) {
    m_isSecured = secured;
}

void Snapshot::setEncryptionSalt(const QString &salt) {
    m_encryptionSalt = salt;
}

void Snapshot::setEncryptionIv(const QString &iv) {
    m_encryptionIv = iv;
}

void Snapshot::setEncryptionTag(const QString &tag) {
    m_encryptionTag = tag;
}

QString Snapshot::displayText() const {
    QString t = m_title.isEmpty() ? "(Untitled)" : m_title;
    if (m_isSecured) {
        t = QString::fromUtf8("🔒 ") + t;
    }
    return t + "\n" + m_createdAt.toString("yyyy-MM-dd HH:mm:ss");
}
