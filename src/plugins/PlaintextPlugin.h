#ifndef PLAINTEXTPLUGIN_H
#define PLAINTEXTPLUGIN_H

#include "../api/IPlugin.h"

class PlaintextPlugin : public IPlugin {
    Q_OBJECT

public:
    PlaintextPlugin(QObject *parent = nullptr) : IPlugin(parent) {}
    ~PlaintextPlugin() = default;

    QString formatId() const override { return "plaintext"; }
    QString displayName() const override { return "Plain Text"; }
    QVariantMap serializeMetadata(const INote &note) const override {
        QVariantMap m;
        m.insert("format", formatId());
        m.insert("text", note.content());
        return m;
    }

    QByteArray serializePayload(const INote & /*note*/) const override {
        return QByteArray();
    }

    bool deserialize(const QVariantMap &metadata, const QByteArray &payload, INote &note) const override {
        Q_UNUSED(payload)
        if (metadata.contains("text")) {
            note.setContent(metadata.value("text").toString());
            return true;
        }
        return false;
    }

    QString render(const INote &note) const override { return note.content(); }

    bool supportsMarkdown() const override { return false; }
    bool supportsPlaintext() const override { return true; }

    QList<IFormattingAction*> getFormattingActions() const override { return {}; }
};

#endif // PLAINTEXTPLUGIN_H
