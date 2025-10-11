#include "FuzzyMac/ConfigManager.hpp"
#include "FuzzyMac/DefaultConfig.hpp"
#include "FuzzyMac/Utils.hpp"

#include "spdlog/spdlog.h"

#include <QFileInfo>
#include <QWidget>

ConfigManager::ConfigManager() {
    default_config = DEFAULT_CONFIG;
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

        toml::table new_config;
        // reload config file
        try {
            new_config = toml::parse_file(config_path.toStdString());
            tbl = new_config;
            spdlog::info("Configuration change detected, loading the new configuration");

        } catch (const toml::parse_error& err) {
            spdlog::warn("Config file: {}", err.description());
        }

    } else {
        spdlog::warn("Config file not found in the expected location {}, falling back to default config.",
                     config_path.toStdString());
    }

    emit configChange();
}
