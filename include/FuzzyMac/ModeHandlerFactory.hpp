#pragma once

#include "FuzzyMac/ModeHandler.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "MainWindow.hpp"
#include "ModeHandler.hpp"

#include <memory>

class ModeHandlerFactory {
    MainWindow* win;
public:
    std::unique_ptr<ModeHandler> create(Mode mode, MainWindow* win);
};
