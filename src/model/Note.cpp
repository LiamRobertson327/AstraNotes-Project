#include "model/Note.h"
#include <QDebug>       //Provides an output stream for debugging information.

Note::Note(QString noteTypeId, QString noteTitle)
    : typeId(std::move(noteTypeId)), title(std::move(noteTitle)) {
    this->createdAt = QDateTime::currentDateTimeUtc();
    this->lastModified = QDateTime::currentDateTimeUtc();
}

void Note::setId(qint64 newId) {
    id = newId;
}

void Note::setTitle(const QString& newTitle) {
    title = newTitle;
    lastModified = QDateTime::currentDateTimeUtc();
}

void Note::setContent(const QString& newContent) {
    content = newContent;
    lastModified = QDateTime::currentDateTimeUtc();
}

void Note::setTypeId(const QString& newTypeId) {
    typeId = newTypeId;
    lastModified = QDateTime::currentDateTimeUtc();
}

void Note::setCreatedAt(const QDateTime& dateTime) {
    createdAt = dateTime;
}

void Note::display() const {
    qInfo().noquote() << "[" + QString::number(id) + "] " + title;
    qInfo().noquote() << "Last Modified:" << lastModified.toString(Qt::ISODate);
    qInfo().noquote() << "Content:" << content;
}