#pragma once

#include "FuzzyMac/InfoPanel.hpp"
#include "FuzzyMac/ClipModeHandler.hpp"

class ClipboardInfoPanel : public InfoPanelContent {
    Q_OBJECT;
public:
    ClipboardInfoPanel(QWidget* parent, API* api, const ClipboardManager::Entry&);
};
