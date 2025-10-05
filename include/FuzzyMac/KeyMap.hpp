#pragma once
#include <functional>
#include <map>

#include <QKeyEvent>

class Keymap {
public:
    // the action returns if it overrides the current behavior or will be on top of it.
    using Action = std::function<void()>;

    void bind(const QKeySequence& seq, Action action, bool override = true) {
        bindings[seq] = {action, override};
    }

    bool trigger(const QKeyEvent* ev) const {
        QKeySequence seq(ev->modifiers() | ev->key());
        if (auto it = bindings.find(seq); it != bindings.end()) {
            auto& [action, _] = it->second;
            action();
            return true;
        }

        return false;
    }

    bool defined(const QKeyEvent* ev) const {
        const QKeySequence seq(ev->modifiers() | ev->key());
        return bindings.find(seq) != bindings.end();
    }

    bool doesOveride(const QKeyEvent* ev) const {
        if (!defined(ev)) {
            return false;
        }

        const QKeySequence seq(ev->modifiers() | ev->key());
        return bindings.at(seq).second;
    }

private:
    std::map<QKeySequence, std::pair<Action, bool>> bindings;
};
