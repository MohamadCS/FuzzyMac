#pragma once

#include "FuzzyMac/DefaultConfig.hpp"

#include <QObject>
#include <toml++/toml.h>

#include <initializer_list>
#include <string>

#include <QFileSystemWatcher>
#include <QList>

// Generic get with fallback from default

class ConfigManager : public QObject {
    Q_OBJECT

public:
    ConfigManager();

    void load();

    template <typename T>
    T get(std::initializer_list<std::string> keys, T fallback = T{}) const;

    template <typename T>
    QList<T> getList(std::initializer_list<std::string> keys, QList<T> fallback = {}) const;

signals:
    void configChange();

private:
    toml::table tbl;
    QString config_path;
    QFileSystemWatcher watcher;

    toml::table default_config;
};

template <typename T>
T ConfigManager::get(std::initializer_list<std::string> keys, T fallback) const {
    const toml::node* node = &tbl;
    for (const auto& key : keys) {
        if (auto tbl = node->as_table()) {
            node = tbl->get(key);
            if (!node)
                break;
        } else {
            node = nullptr;
            break;
        }
    }

    // If not found in user, look in default
    if (!node) {
        node = &default_config;
        for (const auto& key : keys) {
            if (auto tbl = node->as_table()) {
                node = tbl->get(key);
                if (!node)
                    break;
            } else {
                node = nullptr;
                break;
            }
        }
    }

    if (node) {
        if (auto val = node->value<T>())
            return *val;
    }

    return fallback; // fallback if nothing found
}

template <typename T>
QList<T> ConfigManager::getList(std::initializer_list<std::string> keys, QList<T> fallback) const {
    const toml::node* node = &tbl;
    for (const auto& key : keys) {
        if (auto tbl = node->as_table()) {
            node = tbl->get(key);
            if (!node)
                break;
        } else {
            node = nullptr;
            break;
        }
    }

    // If not found in user, look in default
    if (!node) {
        node = &default_config;
        for (const auto& key : keys) {
            if (auto tbl = node->as_table()) {
                node = tbl->get(key);
                if (!node)
                    break;
            } else {
                node = nullptr;
                break;
            }
        }
    }

    if (node) {
        if (const auto* arr = node->as_array()) {
            QList<T> result;
            for (const auto& element : *arr) {
                if (auto val = element.value<T>()) {
                    result.push_back(*val);
                }
            }
            return result;
        }
    }

    return fallback; // fallback if nothing found
}
