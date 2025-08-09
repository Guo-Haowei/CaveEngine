#pragma once
#include "engine/core/io/logger.h"

#include "editor/editor_window.h"

namespace cave {

class LogPanel : public EditorWindow {

    struct LogCache {
        std::unordered_set<uint32_t> duplicate;
        std::vector<CompositeLogger::Log> logs;

        uint32_t GetCount() const {
            return static_cast<uint32_t>(logs.size());
        }

        void AddPermLog(const CompositeLogger::Log& p_log) {
            auto [_, ok] = duplicate.insert(p_log.id);
            if (ok) {
                logs.push_back(p_log);
            }
        }
    };

public:
    LogPanel(EditorLayer& editor)
        : EditorWindow(editor) {}

    const char* GetTitle() const override {
        return "Log";
    }

    uint32_t GetErrorCount() const {
        return m_error_logs.GetCount();
    }

    uint32_t GetWarningCount() const {
        return m_warning_logs.GetCount();
    }

    void RetrieveLogs();

protected:
    void UpdateInternal() override;

    bool m_auto_scroll{ true };
    bool m_scroll_to_bottom{ false };
    LogLevel m_filter{ LOG_LEVEL_ALL };
    std::vector<CompositeLogger::Log> m_logs;

    LogCache m_error_logs;
    LogCache m_warning_logs;
};

}  // namespace cave
