#pragma once

#include "Carbon/Carbon.h"
#include "FuzzyMac/MainWindow.hpp"
#include "MainWindow.hpp"



OSStatus hotKeyHandler(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData); 
void registerGlobalHotkey(MainWindow* win); 

