#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/ModeHandler.hpp"
#include <vector>

class WallpaperMode : public ModeHandler {
public:
    WallpaperMode(MainWindow* win);
    void invokeQuery(const QString& query_) override;
    void load() override;
    InfoPanelContent* getInfoPanelContent() const override;
    std::vector<FuzzyWidget*> createMainModeWidgets() override;
    QString getModeText() override;
    QString getPrefix() const override;

private:
    std::vector<FuzzyWidget*> widgets;
    QFileSystemWatcher* fs_watcher;
    QFutureWatcher<QStringList>* future_watcher;
    QStringList entries;
    QStringList paths;

    void reloadEntries();
    void freeWidgets() override;
    void setupKeymaps();
};
