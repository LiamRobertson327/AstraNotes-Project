#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "../api/IPlugin.h"
#include <QMap>
#include <QString>
#include <QStringList>

class PluginManager {
public:
    static PluginManager &instance();  // Singleton

    void registerPlugin(IPlugin *plugin);
    IPlugin *getPlugin(const QString &formatId) const;
    QStringList availableFormats() const;

private:
    PluginManager() = default;
    ~PluginManager() = default;
    
    PluginManager(const PluginManager &) = delete;
    PluginManager &operator=(const PluginManager &) = delete;

    QMap<QString, IPlugin*> plugins;
};

#endif // PLUGINMANAGER_H
