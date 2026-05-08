#include "PluginManager.h"

PluginManager &PluginManager::instance() {
    static PluginManager manager;
    return manager;
}

void PluginManager::registerPlugin(IPlugin *plugin) {
    if (plugin) {
        plugins[plugin->formatId()] = plugin;
    }
}

IPlugin *PluginManager::getPlugin(const QString &formatId) const {
    return plugins.value(formatId, nullptr);
}

QStringList PluginManager::availableFormats() const {
    return plugins.keys();
}
