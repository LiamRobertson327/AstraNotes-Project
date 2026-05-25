#ifndef INOTE_H
#define INOTE_H

#include <QString>
#include <QDateTime>
#include <QtGlobal>

class INote {
public:
	virtual ~INote() = default;

	// Ownership / lifetime:
	// INote is a non-owning interface for value-like note data.
	// Implementations are responsible for their own internal storage, but
	// callers do not own or delete INote instances through this interface.

	// Accessors
	virtual qint64 noteId() const = 0;
	virtual QString typeId() const = 0;
	virtual QString content() const = 0;
	virtual QString title() const = 0;
	virtual QDateTime createdAt() const = 0;
	virtual QDateTime lastModified() const = 0;

	// Mutators
	virtual void setNoteId(qint64 newId) = 0;
	virtual void setTypeId(const QString& newTypeId) = 0;
	virtual void setContent(const QString& newContent) = 0;
	virtual void setTitle(const QString& newTitle) = 0;
	virtual void setCreatedAt(const QDateTime& dt) = 0;
	virtual void setLastModified(const QDateTime& dt) = 0;
	
	// Methods
	virtual QString displayText() const = 0;
};

#endif