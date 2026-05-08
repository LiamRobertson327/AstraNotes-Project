#ifndef MARKDOWNPLUGIN_H
#define MARKDOWNPLUGIN_H

#include "IPlugin.h"

class MarkdownPlugin : public IPlugin {
    Q_OBJECT

public:
    MarkdownPlugin(QObject *parent = nullptr) : IPlugin(parent) {}
    ~MarkdownPlugin() = default;

    QString formatId() const override { return "markdown"; }
    QString displayName() const override { return "Markdown"; }

    QString serialize(const QString &content) const override { return content; }
    QString deserialize(const QString &stored) const override { return stored; }

    QString render(const QString &content) const override { return content; }

    bool supportsMarkdown() const override { return true; }
    bool supportsPlaintext() const override { return true; }

    QList<IFormattingAction*> getFormattingActions() const override;
};

#endif // MARKDOWNPLUGIN_H
