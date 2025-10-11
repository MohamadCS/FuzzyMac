// #include "FuzzyMac/GlobalShortcut.hpp"
// #include <QDebug>
//
// QMap<int, GlobalShortcut*> GlobalShortcut::hotkeyMap;
// int GlobalShortcut::nextId = 1;
//
// GlobalShortcut::GlobalShortcut(QObject* parent)
//     : QObject(parent) {
// }
//
// GlobalShortcut::~GlobalShortcut() {
//     unregister();
// }
//
// bool GlobalShortcut::registerShortcut(const QKeySequence& seq) {
//     unregister();
//
//     // Parse QKeySequence to macOS virtual key and modifiers
//     QString str = seq.toString(QKeySequence::NativeText).toLower();
//
//     UInt32 keyCode = 0;
//     UInt32 modifiers = 0;
//
//     if (str.contains("cmd"))
//         modifiers |= cmdKey;
//     if (str.contains("shift"))
//         modifiers |= shiftKey;
//     if (str.contains("ctrl") || str.contains("control"))
//         modifiers |= controlKey;
//     if (str.contains("alt") || str.contains("option"))
//         modifiers |= optionKey;
//
//     if (str.contains("c"))
//         keyCode = kVK_ANSI_C;
//     else if (str.contains("space"))
//         keyCode = kVK_Space;
//     else if (str.contains("v"))
//         keyCode = kVK_ANSI_V;
//     else if (str.contains("x"))
//         keyCode = kVK_ANSI_X;
//     else if (str.contains("p"))
//         keyCode = kVK_ANSI_P;
//     else {
//         qWarning() << "Unsupported key:" << str;
//         return false;
//     }
//
//     hotKeyID.signature = 'gshk';
//     hotKeyID.id = nextId++;
//
//     OSStatus err = RegisterEventHotKey(keyCode, modifiers, hotKeyID, GetApplicationEventTarget(), 0, &hotKeyRef);
//
//     if (err != noErr) {
//         qWarning() << "Failed to register hotkey, error:" << err;
//         return false;
//     }
//
//     hotkeyMap.insert(hotKeyID.id, this);
//
//     static bool handlerInstalled = false;
//     if (!handlerInstalled) {
//         EventTypeSpec eventType;
//         eventType.eventClass = kEventClassKeyboard;
//         eventType.eventKind = kEventHotKeyPressed;
//         InstallApplicationEventHandler(&GlobalShortcut::hotkeyHandler, 1, &eventType, nullptr, nullptr);
//         handlerInstalled = true;
//     }
//
//     return true;
// }
//
// void GlobalShortcut::unregister() {
//     if (hotKeyRef) {
//         UnregisterEventHotKey(hotKeyRef);
//         hotKeyRef = nullptr;
//     }
// }
//
// OSStatus GlobalShortcut::hotkeyHandler(EventHandlerCallRef, EventRef event, void*) {
//     EventHotKeyID hotKeyID;
//     GetEventParameter(event, kEventParamDirectObject, typeEventHotKeyID, nullptr, sizeof(hotKeyID), nullptr, &hotKeyID);
//
//     if (GlobalShortcut::hotkeyMap.contains(hotKeyID.id)) {
//         emit GlobalShortcut::hotkeyMap[hotKeyID.id]->activated();
//     }
//
//     return noErr;
// }
