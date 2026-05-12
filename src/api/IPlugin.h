#ifndef IPLUGIN_H
#define IPLUGIN_H

#include <QString>
#include <QObject>
#include <QList>
#include <QVariantMap>
#include <QByteArray>
#include "INote.h"

class IFormattingAction;

// Public plugin contract for developers.
// Plugins expose format metadata, serialize/deserialize note data,
// optionally render read-only content, and provide formatting actions.
class IPlugin : public QObject {
public:
    explicit IPlugin(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IPlugin() = default;

    // Plugin metadata
    virtual QString formatId() const = 0;        // e.g., "plaintext", "markdown"
    virtual QString displayName() const = 0;     // e.g., "Plain Text", "Markdown"

    // Serialization: plugins provide metadata plus an optional binary payload.
    // Metadata is a small key/value map; payload carries large or binary data.
    virtual QVariantMap serializeMetadata(const INote &note) const = 0;
    virtual QByteArray serializePayload(const INote &note) const = 0;

    // Deserialize from metadata + payload into an INote instance.
    virtual bool deserialize(const QVariantMap &metadata, const QByteArray &payload, INote &note) const = 0;

    // Rendering helper: read-only display for a note.
    virtual QString render(const INote &note) const { return note.content(); }

    // Capability flags
    virtual bool supportsMarkdown() const = 0;
    virtual bool supportsPlaintext() const = 0;

    // Formatting actions (optional)
    virtual QList<IFormattingAction*> getFormattingActions() const { return {}; }
};

#define IPlugin_IID "com.astranotes.IPlugin/1.0"
Q_DECLARE_INTERFACE(IPlugin, IPlugin_IID)

#endif // IPLUGIN_H
