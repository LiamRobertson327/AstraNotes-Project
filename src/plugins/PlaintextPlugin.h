#ifndef PLAINTEXTPLUGIN_H
#define PLAINTEXTPLUGIN_H

#include "IPlugin.h"

class PlaintextPlugin : public IPlugin {
    Q_OBJECT

public:
    PlaintextPlugin(QObject *parent = nullptr) : IPlugin(parent) {}
    ~PlaintextPlugin() = default;

    QString formatId() const override { return "plaintext"; }
    QString displayName() const override { return "Plain Text"; }

    QString serialize(const QString &content) const override { return content; }
    QString deserialize(const QString &stored) const override { return stored; }

    QString render(const QString &content) const override { return content; }

    bool supportsMarkdown() const override { return false; }
    bool supportsPlaintext() const override { return true; }

    QList<IFormattingAction*> getFormattingActions() const override { return {}; }
};

#endif // PLAINTEXTPLUGIN_H
