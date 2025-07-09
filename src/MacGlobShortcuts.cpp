#include "../include/FuzzyMac/MacGlobShortcuts.hpp"
#include "../include/FuzzyMac/MainWindow.hpp"
#include "../include/FuzzyMac/NativeMacHandlers.hpp"

OSStatus triggerApp(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData) {

    // Call back into wxWidgets (main frame) to toggle visibility
    auto* win = static_cast<MainWindow*>(userData);

    if (win->isHidden()) { // HIDE
        win->wakeup();
    } else {
        win->sleep();
    }
    return noErr;
}

void registerGlobalHotkey(MainWindow* win) {
    EventHotKeyRef g_hotKeyRef;
    EventHotKeyID g_hotKeyID;
    EventTypeSpec eventType;

    eventType.eventClass = kEventClassKeyboard;
    eventType.eventKind = kEventHotKeyPressed;

    InstallApplicationEventHandler(&triggerApp, 1, &eventType, (void*)win, NULL);

    g_hotKeyID.signature = 'htk1';
    g_hotKeyID.id = 1;

    // Example: Command + Space
    RegisterEventHotKey(kVK_Space, cmdKey, g_hotKeyID, GetApplicationEventTarget(), 0, &g_hotKeyRef);
}
