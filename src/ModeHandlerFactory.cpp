#include "FuzzyMac/ModeHandlerFactory.hpp"
#include "FuzzyMac/AppModeHandler.hpp"
#include "FuzzyMac/CLIModeHandler.hpp"
#include "FuzzyMac/FileModeHandler.hpp"
#include "FuzzyMac/ClipModeHandler.hpp"

std::unique_ptr<ModeHandler> ModeHandlerFactory::create(Mode mode, MainWindow* win) {
    switch (mode) {
        case Mode::APP:
            return std::make_unique<AppModeHandler>(win);
        case Mode::FILE:
            return std::make_unique<FileModeHandler>(win);
        case Mode::CLI:
            return std::make_unique<CLIModeHandler>(win);
        case Mode::CLIP:
            return std::make_unique<ClipModeHandler>(win);
        default:
            return nullptr;
    }
}
