#pragma once

#include <toml++/toml.h>

#include <QString>

#include <initializer_list>
#include <string>

// Generic get with fallback from default

const toml::table default_config = toml::parse(R"toml(
font = "JetBrainsMono Nerd Font"
[colors.query_input]
selection = "#cecacd"
selection_background = "#cecacd"
text = "#575279"
background = "#f2e9e1"

[colors.results_list]
selection = "#575279"
selection_background = "#cecacd"
text = "#575279"
background = "#f2e9e1"


[mode.apps]

dirs = ["/Applications/", "/System/Applications", "/Applications/Utilities/"]
show_icons = true

apps = [
  "/System/Library/CoreServices/Finder.app",
  "/System/Applications/Utilities/Activity Monitor.app",
]

[mode.files]
show_icons = true

dirs = ["$HOME/Library/Mobile Documents/com~apple~CloudDocs/", "$HOME/Desktop/"]
)toml");

template <typename T>
T get(const toml::table& user, std::initializer_list<std::string> keys, T fallback = T{}) {
    const toml::node* node = &user;
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
        node = &::default_config;
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
                     //
}

template <typename T>
std::vector<T> get_array(const toml::table& user, std::initializer_list<std::string> keys,
                         std::vector<T> fallback = {}) {
    const toml::node* node = &user;
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
        node = &::default_config;
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
            std::vector<T> result;
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

QString buildStyleSheet(const toml::table& config, const std::string& widget_name);
