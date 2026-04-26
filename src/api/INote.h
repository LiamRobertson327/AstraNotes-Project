#ifndef INOTE_H
#define INOTE_H

#include <QString>
#include <QDateTime>
#include <QtGlobal>

class INote {
public:
	virtual ~INote() = default;

	// Identity and classification.
	virtual qint64 id() const = 0;
	virtual void setId(qint64 newId) = 0;
	virtual QString typeId() const = 0;
	virtual void setTypeId(const QString& newTypeId) = 0;

	// Display metadata.
	virtual QString title() const = 0;
	virtual void setTitle(const QString& newTitle) = 0;

	// Content and timestamps.
	virtual QString content() const = 0;
	virtual void setContent(const QString& newContent) = 0;
	virtual QDateTime lastModified() const = 0;

	// Convenience helper for debugging and UI rendering.
	virtual QString displayText() const = 0;
};

#endif
