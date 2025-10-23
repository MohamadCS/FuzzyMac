#include "FuzzyMac/ModeHandlerFactory.hpp"
#include "FuzzyMac/AppModeHandler.hpp"
#include "FuzzyMac/CLIModeHandler.hpp"
#include "FuzzyMac/FileModeHandler.hpp"
#include "FuzzyMac/ClipModeHandler.hpp"
#include "FuzzyMac/WallpaperMode.hpp"

std::unique_ptr<ModeHandler> ModeHandlerFactory::create(Mode mode, MainWindow* win) {
    auto* api = win->getAPI();
    switch (mode) {
        case Mode::APP:
            return std::make_unique<AppModeHandler>(win, api);
        case Mode::FILE:
            return std::make_unique<FileModeHandler>(win,api);
        case Mode::CLI:
            return std::make_unique<CLIModeHandler>(win,api);
        case Mode::CLIP:
            return std::make_unique<ClipModeHandler>(win,api);
        case Mode::WALLPAPER:
            return std::make_unique<WallpaperMode>(win,api);
        default:
            return nullptr;
    }
}
