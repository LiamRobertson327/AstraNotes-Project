#include "model/Note.h"
#include <QDebug>       //Provides an output stream for debugging information.

Note::Note(QString noteTypeId, QString noteTitle)
    : m_typeId(std::move(noteTypeId)), m_title(std::move(noteTitle)) {
    m_createdAt = QDateTime::currentDateTimeUtc();
    m_lastModified = QDateTime::currentDateTimeUtc();
}

void Note::setNoteId(qint64 newId) {
    m_noteId = newId;
}

void Note::setTitle(const QString& newTitle) {
    m_title = newTitle;
    m_lastModified = QDateTime::currentDateTimeUtc();
}

void Note::setContent(const QString& newContent) {
    m_content = newContent;
    m_lastModified = QDateTime::currentDateTimeUtc();
}

void Note::setTypeId(const QString& newTypeId) {
    m_typeId = newTypeId;
    m_lastModified = QDateTime::currentDateTimeUtc();
}

void Note::setCreatedAt(const QDateTime& dateTime) {
    m_createdAt = dateTime;
}

void Note::setLastModified(const QDateTime& dateTime) {
    m_lastModified = dateTime;
}

void Note::display() const {
    qInfo().noquote() << "[" + QString::number(m_noteId) + "] " + m_title;
    qInfo().noquote() << "Last Modified:" << m_lastModified.toString(Qt::ISODate);
    qInfo().noquote() << "Content:" << m_content;
}

// INote interface implementations
QString Note::displayText() const {
    QString t = m_title.isEmpty() ? "(Untitled)" : m_title;
    return t + "\n" + m_createdAt.toString("MMM dd, yyyy");
}