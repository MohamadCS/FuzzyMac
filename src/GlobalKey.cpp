#include "../include/App/GlobalKeys.hpp"
#include "../include/App/MacNativeHandler.hpp"


#include "wx/app.h"



OSStatus triggerApp(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData) {

    // Call back into wxWidgets (main frame) to toggle visibility
    auto* frame = static_cast<MainFrame*>(userData);


    if (frame->IsShown()) { //HIDE
        DeactivateApp();
        frame->Hide();
    } else {
        frame->Show();
        frame->Raise(); // Bring to front
        frame->wakeup(); // Bring to front
        MakeWindowKey(frame->GetHandle());
    }
    return noErr;
}

void registerGlobalHotkey(MainFrame* frame) {
    EventHotKeyRef g_hotKeyRef;
    EventHotKeyID g_hotKeyID;
    EventTypeSpec eventType;

    eventType.eventClass = kEventClassKeyboard;
    eventType.eventKind = kEventHotKeyPressed;

    InstallApplicationEventHandler(&triggerApp, 1, &eventType, (void*)frame, NULL);

    g_hotKeyID.signature = 'htk1';
    g_hotKeyID.id = 1;

    // Example: Command + Space
    RegisterEventHotKey(kVK_Space, cmdKey, g_hotKeyID, GetApplicationEventTarget(), 0, &g_hotKeyRef);
}

