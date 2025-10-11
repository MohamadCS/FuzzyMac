#pragma once
#include <Carbon/Carbon.h>
#include <QObject>
#include <QKeySequence>
#include <QDebug>
#include <QMap>

class GlobalHotkeyBridge : public QObject {
    Q_OBJECT
public:
    explicit GlobalHotkeyBridge(QObject* parent = nullptr)
        : QObject(parent)
    {
        // Install Carbon event handler for hotkeys
        EventTypeSpec eventType;
        eventType.eventClass = kEventClassKeyboard;
        eventType.eventKind  = kEventHotKeyPressed;

        InstallApplicationEventHandler(&GlobalHotkeyBridge::hotkeyHandler, 1, &eventType, this, nullptr);
    }

    // Registers a QKeySequence as a global shortcut
    bool registerHotkey(const QKeySequence& seq) {
        if (seq.isEmpty()) return false;

        int keyInt = seq[0]; // combined key + modifiers

        // Extract modifiers
        Qt::KeyboardModifiers mods = static_cast<Qt::KeyboardModifiers>(keyInt & Qt::KeyboardModifierMask);

        // Extract key
        Qt::Key qtKey = static_cast<Qt::Key>(keyInt & ~Qt::KeyboardModifierMask);

        // Map Qt modifiers to macOS Carbon flags
        UInt32 carbonMods = 0;
        if (mods & Qt::ControlModifier) carbonMods |= controlKey;
        if (mods & Qt::ShiftModifier)   carbonMods |= shiftKey;
        if (mods & Qt::AltModifier)     carbonMods |= optionKey;
        if (mods & Qt::MetaModifier)    carbonMods |= cmdKey;

        // Map Qt key to macOS virtual keycode
        UInt32 keyCode = qtKeyToMac(qtKey);
        if (keyCode == UINT32_MAX) {
            qWarning() << "Unsupported key in hotkey:" << qtKey;
            return false;
        }

        EventHotKeyRef hotKeyRef;
        EventHotKeyID hotKeyID;
        hotKeyID.signature = 'GHKB'; // arbitrary 4-char signature
        hotKeyID.id = ++nextId;

        OSStatus err = RegisterEventHotKey(
            keyCode,
            carbonMods,
            hotKeyID,
            GetApplicationEventTarget(),
            0,
            &hotKeyRef
        );

        if (err != noErr) {
            qWarning() << "RegisterEventHotKey failed:" << err;
            return false;
        }

        hotkeys[hotKeyID.id] = this;
        return true;
    }

signals:
    void activated();

private:
    inline static QMap<int, GlobalHotkeyBridge*> hotkeys;
    inline static int nextId = 0;

    // Map Qt::Key to macOS virtual keycodes (extend as needed)
    static UInt32 qtKeyToMac(Qt::Key key) {
        switch (key) {
            case Qt::Key_Space: return kVK_Space;
            case Qt::Key_A:     return kVK_ANSI_A;
            case Qt::Key_B:     return kVK_ANSI_B;
            case Qt::Key_C:     return kVK_ANSI_C;
            case Qt::Key_D:     return kVK_ANSI_D;
            case Qt::Key_E:     return kVK_ANSI_E;
            case Qt::Key_F:     return kVK_ANSI_F;
            case Qt::Key_G:     return kVK_ANSI_G;
            case Qt::Key_H:     return kVK_ANSI_H;
            case Qt::Key_I:     return kVK_ANSI_I;
            case Qt::Key_J:     return kVK_ANSI_J;
            case Qt::Key_K:     return kVK_ANSI_K;
            case Qt::Key_L:     return kVK_ANSI_L;
            case Qt::Key_M:     return kVK_ANSI_M;
            case Qt::Key_N:     return kVK_ANSI_N;
            case Qt::Key_O:     return kVK_ANSI_O;
            case Qt::Key_P:     return kVK_ANSI_P;
            case Qt::Key_Q:     return kVK_ANSI_Q;
            case Qt::Key_R:     return kVK_ANSI_R;
            case Qt::Key_S:     return kVK_ANSI_S;
            case Qt::Key_T:     return kVK_ANSI_T;
            case Qt::Key_U:     return kVK_ANSI_U;
            case Qt::Key_V:     return kVK_ANSI_V;
            case Qt::Key_W:     return kVK_ANSI_W;
            case Qt::Key_X:     return kVK_ANSI_X;
            case Qt::Key_Y:     return kVK_ANSI_Y;
            case Qt::Key_Z:     return kVK_ANSI_Z;
            case Qt::Key_0:     return kVK_ANSI_0;
            case Qt::Key_1:     return kVK_ANSI_1;
            case Qt::Key_2:     return kVK_ANSI_2;
            case Qt::Key_3:     return kVK_ANSI_3;
            case Qt::Key_4:     return kVK_ANSI_4;
            case Qt::Key_5:     return kVK_ANSI_5;
            case Qt::Key_6:     return kVK_ANSI_6;
            case Qt::Key_7:     return kVK_ANSI_7;
            case Qt::Key_8:     return kVK_ANSI_8;
            case Qt::Key_9:     return kVK_ANSI_9;
            default: return UINT32_MAX;
        }
    }

    // Carbon event handler
    static pascal OSStatus hotkeyHandler(EventHandlerCallRef, EventRef event, void* userData) {
        EventHotKeyID hotKeyID;
        GetEventParameter(event, kEventParamDirectObject, typeEventHotKeyID, nullptr, sizeof(hotKeyID), nullptr, &hotKeyID);

        auto it = hotkeys.find(hotKeyID.id);
        if (it != hotkeys.end()) {
            emit it.value()->activated();
        }
        return noErr;
    }
};

