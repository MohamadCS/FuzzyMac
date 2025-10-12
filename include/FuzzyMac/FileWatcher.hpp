#pragma once
#include <CoreServices/CoreServices.h>
#include <QDebug>
#include <QObject>
#include <QStringList>

#include "spdlog/spdlog.h"

class MacFileWatcher : public QObject {
    Q_OBJECT
public:
    explicit MacFileWatcher(QObject* parent = nullptr)
        : QObject(parent) {
    }

    ~MacFileWatcher() override {
        stop();
    }

    // Start watching multiple directories recursively
    void watch(const QStringList& directories) {
        if (directories.isEmpty()) {
            spdlog::warn("No directories to watch.");
            return;
        }

        // Merge with existing ones (no duplicates)
        for (const auto& dir : directories) {
            if (!watched_dirs.contains(dir))
                watched_dirs.append(dir);
        }

        restartStream();
    }

    // Stop watching specific directories
    void unwatch(const QStringList& directories) {
        if (directories.isEmpty())
            return;

        for (const auto& dir : directories)
            watched_dirs.removeAll(dir);

        if (watched_dirs.isEmpty())
            stop();
        else
            restartStream(); // rebuild stream with updated list
    }

    void stop() {
        if (stream_ref) {
            FSEventStreamStop(stream_ref);
            FSEventStreamSetDispatchQueue(stream_ref, nullptr);
            FSEventStreamInvalidate(stream_ref);
            FSEventStreamRelease(stream_ref);
            stream_ref = nullptr;
        }
    }

signals:
    void file_changed(const QString& path);

private:
    FSEventStreamRef stream_ref = nullptr;
    QStringList watched_dirs;

    void restartStream() {
        stop();

        CFMutableArrayRef paths_to_watch = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
        for (const QString& dir : watched_dirs) {
            CFStringRef cf_dir = CFStringCreateWithCString(nullptr, dir.toUtf8().constData(), kCFStringEncodingUTF8);
            CFArrayAppendValue(paths_to_watch, cf_dir);
            CFRelease(cf_dir);
        }

        FSEventStreamContext context = {0, this, nullptr, nullptr, nullptr};

        stream_ref = FSEventStreamCreate(kCFAllocatorDefault,
                                         &MacFileWatcher::fseventCallback,
                                         &context,
                                         paths_to_watch,
                                         kFSEventStreamEventIdSinceNow,
                                         1.0, // latency in seconds
                                         kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagWatchRoot |
                                             kFSEventStreamCreateFlagUseCFTypes);

        CFRelease(paths_to_watch);

        if (!stream_ref) {
            return;
        }

        FSEventStreamSetDispatchQueue(stream_ref, dispatch_get_main_queue());

        if (!FSEventStreamStart(stream_ref)) {
            qWarning() << "Failed to start FSEventStream.";
            stop();
        } else {
            qDebug() << "Watching started:" << watched_dirs;
        }
    }

    static void fseventCallback(ConstFSEventStreamRef, void* clientCallBackInfo, size_t numEvents, void* eventPaths,
                                const FSEventStreamEventFlags[], const FSEventStreamEventId[]) {
        auto* self = static_cast<MacFileWatcher*>(clientCallBackInfo);
        if (!self)
            return;

        CFArrayRef paths_arr = static_cast<CFArrayRef>(eventPaths);
        for (CFIndex i = 0; i < CFArrayGetCount(paths_arr); ++i) {
            CFStringRef cf_path = (CFStringRef)CFArrayGetValueAtIndex(paths_arr, i);
            char buffer[PATH_MAX];
            if (CFStringGetCString(cf_path, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
                emit self->file_changed(QString::fromUtf8(buffer));
            }
        }
    }
};
