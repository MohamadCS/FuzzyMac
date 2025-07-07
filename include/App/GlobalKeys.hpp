#pragma once

#include "Carbon/Carbon.h"
#include "MainFrame.hpp"



OSStatus hotKeyHandler(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData); 
void registerGlobalHotkey(MainFrame* frame); 

