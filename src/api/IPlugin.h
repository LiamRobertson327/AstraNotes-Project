#ifndef IPLUGIN_H
#define IPLUGIN_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QtGlobal>

class INote;

class IPlugin : public QObject {
	Q_OBJECT
public:
	explicit IPlugin(QObject *parent = nullptr) : QObject(parent) {}
	virtual ~IPlugin() = default;

	// Stable identity used by the app and by saved notes.
	virtual QString pluginId() const = 0;
	virtual QString displayName() const = 0;
	virtual QString version() const = 0;
	virtual QString author() const = 0;

	// Capabilities advertised to the UI and plugin manager.
	virtual QStringList supportedNoteTypeIds() const = 0;
	virtual QStringList supportedMimeTypes() const = 0;

	// Lifecycle hooks.
	virtual bool initialize() = 0;
	virtual void shutdown() = 0;

	// Core operations for a plugin-driven note format.
	virtual QString serialize(const INote& note) const = 0;
	virtual bool deserialize(const QString& payload, INote& note) const = 0;
	virtual bool validate(const INote& note) const = 0;

	// Optional helper for future file-based import/export behavior.
	virtual QString defaultFileExtension() const = 0;
};

#define IPlugin_IID "com.astranotes.IPlugin/1.0"
Q_DECLARE_INTERFACE(IPlugin, IPlugin_IID)

#endif
