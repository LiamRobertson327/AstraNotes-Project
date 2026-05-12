#ifndef MARKDOWNPLUGIN_H
#define MARKDOWNPLUGIN_H

#include "../api/IPlugin.h"

class MarkdownPlugin : public IPlugin {
    Q_OBJECT

public:
    MarkdownPlugin(QObject *parent = nullptr) : IPlugin(parent) {}
    ~MarkdownPlugin() = default;

    QString formatId() const override { return "markdown"; }
    QString displayName() const override { return "Markdown"; }
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
        Q_UNUSED(payload)   // Supress compiler warnings about unused parameter.
        if (metadata.contains("text")) {
            note.setContent(metadata.value("text").toString());
            return true;
        }
        return false;
    }

    QString render(const INote &note) const override { return note.content(); }

    bool supportsMarkdown() const override { return true; }
    bool supportsPlaintext() const override { return true; }

    QList<IFormattingAction*> getFormattingActions() const override;
};

#endif // MARKDOWNPLUGIN_H
