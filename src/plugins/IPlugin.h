#ifndef IPLUGIN_H
#define IPLUGIN_H

#include <QString>
#include <QObject>
#include <QList>

class IFormattingAction;

class IPlugin : public QObject {
    Q_OBJECT

public:
    explicit IPlugin(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IPlugin() = default;

    // Plugin metadata
    virtual QString formatId() const = 0;        // e.g., "plaintext", "markdown"
    virtual QString displayName() const = 0;     // e.g., "Plain Text", "Markdown"

    // Serialization
    virtual QString serialize(const QString &content) const = 0;     // Convert to storage format
    virtual QString deserialize(const QString &stored) const = 0;    // Convert from storage format

    // Rendering (optional, for display modes)
    virtual QString render(const QString &content) const { return content; }  // Default: no rendering

    // Capability flags
    virtual bool supportsMarkdown() const = 0;
    virtual bool supportsPlaintext() const = 0;

    // Formatting actions (optional)
    virtual QList<IFormattingAction*> getFormattingActions() const { return {}; }
};

#endif // IPLUGIN_H
