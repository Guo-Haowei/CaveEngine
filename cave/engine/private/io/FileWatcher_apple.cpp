#include "cave/io/FileWatcher.h"

#if USING(PLATFORM_APPLE)
#include <CoreServices/CoreServices.h>

namespace cave {

static void OnFSEvents(
    ConstFSEventStreamRef,
    void* context,
    size_t,
    void*,
    const FSEventStreamEventFlags[],
    const FSEventStreamEventId[]) {
    static_cast<std::atomic<bool>*>(context)->store(true);
}

FileWatcher::FileWatcher() : m_dir_handle{ nullptr } {}

void FileWatcher::start(std::string_view path) {
    m_path = path;
    m_stop = false;

    m_thread = std::thread([this]() {
        watchLoop();
    });
}

void FileWatcher::stop() {
    m_stop = true;

    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void FileWatcher::watchLoop() {
    CFStringRef root = CFStringCreateWithBytes(
        kCFAllocatorDefault,
        reinterpret_cast<const UInt8*>(m_path.data()),
        static_cast<CFIndex>(m_path.size()),
        kCFStringEncodingUTF8,
        false);

    if (!root) {
        LOG_ERROR(LogChannel::FS, "Failed to convert watcher path to CFString: {}", m_path);
        m_stop = true;
        return;
    }

    const void* paths[] = { root };
    CFArrayRef pathsToWatch =
        CFArrayCreate(kCFAllocatorDefault, paths, 1, &kCFTypeArrayCallBacks);
    CFRelease(root);

    if (!pathsToWatch) {
        LOG_ERROR(LogChannel::FS, "Failed to create FSEvents path array for {}", m_path);
        m_stop = true;
        return;
    }

    FSEventStreamContext context{};
    context.info = &m_changed;

    FSEventStreamRef stream = FSEventStreamCreate(
        kCFAllocatorDefault,
        &OnFSEvents,
        &context,
        pathsToWatch,
        kFSEventStreamEventIdSinceNow,
        0.1,
        kFSEventStreamCreateFlagFileEvents |
            kFSEventStreamCreateFlagNoDefer);

    CFRelease(pathsToWatch);

    if (!stream) {
        LOG_ERROR(LogChannel::FS, "Failed to create FSEvent stream for {}", m_path);
        m_stop = true;
        return;
    }

    CFRunLoopRef runLoop = CFRunLoopGetCurrent();
    FSEventStreamScheduleWithRunLoop(stream, runLoop, kCFRunLoopDefaultMode);

    if (!FSEventStreamStart(stream)) {
        LOG_ERROR(LogChannel::FS, "Failed to start FSEvent stream for {}", m_path);
        FSEventStreamInvalidate(stream);
        FSEventStreamRelease(stream);
        m_stop = true;
        return;
    }

    while (!m_stop.load()) {
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, true);
    }

    FSEventStreamStop(stream);
    FSEventStreamInvalidate(stream);
    FSEventStreamRelease(stream);

    m_stop = true;
}

}  // namespace cave

#endif