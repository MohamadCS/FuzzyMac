#include "FuzzyMac/ConfigManager.hpp"
#include "FuzzyMac/Utils.hpp"

#include <QFileInfo>
#include <QWidget>

ConfigManager::ConfigManager() {
    QStringList p_{"$HOME/.config/FuzzyMac/config.toml"};
    expandPaths(p_);

    config_path = p_[0];

    tbl = default_config;
    load();

    connect(&watcher, &QFileSystemWatcher::fileChanged, [this]() { load(); });
    watcher.addPath(config_path);
}

void ConfigManager::load() {
    if (QFileInfo(config_path).exists()) {
        toml::table new_config = tbl;
        // reload config file
        try {
            new_config = toml::parse_file(config_path.toStdString());
        } catch (const toml::parse_error& err) {
            new_config = tbl;
        }
        tbl = new_config;
    }

    emit configChange();
}
