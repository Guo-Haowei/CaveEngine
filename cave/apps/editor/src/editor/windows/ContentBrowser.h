#pragma once
#include "editor/windows/EditorWindow.h"

namespace cave {

struct ContentEntry;

class ContentBrowser : public EditorWindow {
public:
    ContentBrowser(EditorState& editor);

    void onAttach() override;
    void onDetach() override;

    const char* windowId() const override;

protected:
    class CurrentPath {
    public:
        auto& getMut() { return m_parts; }

        size_t size() const { return m_parts.size(); }
        const auto& at(int i) const { return m_parts[i]; }

        void setPropertyChangeCallback(std::function<void()>&& func) {
            m_property_change_func = std::move(func);
        }

        void onPropertyChange() {
            m_property_change_func();
        }

        void add(std::string part) {
            m_parts.emplace_back(std::move(part));
            onPropertyChange();
        }

        void splitVirtualPath(std::string_view path);
        std::string joinVirtualPath() const;

    private:
        std::function<void()> m_property_change_func;
        std::vector<std::string> m_parts;
    };

    void drawUIImpl() override;

    void drawContentBrowser();
    void drawBreadcrumb();

    const ContentEntry* navigate(const ContentEntry* node, int cur, int p_max);

    CurrentPath m_path;
    uint64_t m_folder_iamge;
    uint64_t m_fallback_iamge;
    std::unordered_map<std::string_view, uint64_t> m_thumbnail_lut;
};

}  // namespace cave
