#include "WorkspaceState.h"

// @TODO: refactor
#include "engine/private/runtime/serialization/YamlInclude.h"

namespace cave {

namespace fs = std::filesystem;

namespace {

// @TODO: move to util
bool EnsureParentDirExists(const fs::path& file_path) {
    if (fs::exists(file_path)) {
        return true;
    }

    std::error_code ec;
    fs::create_directories(file_path.parent_path(), ec);

    if (ec) {
        LOG_ERROR("Failed to create directory '{}': {}",
                  file_path.parent_path().string(),
                  ec.message());
        return false;
    }

    return true;
}

}  // namespace

void WorkspaceState::markDirty() {
    if (m_timer.finished()) {
        m_timer.start();
    }
    m_dirty = true;
}

bool WorkspaceState::save(const std::filesystem::path& path, float dt) {
    if (!m_dirty) {
        return true;
    }

    m_timer.tick(dt);

    if (m_timer.active()) {
        return true;
    }

    if (!saveImpl(path)) {
        return false;
    }

    m_dirty = false;
    m_timer.start();

    return true;
}

bool WorkspaceState::saveNow(const std::filesystem::path& path) {
    const bool ok = saveImpl(path);
    m_dirty = !ok;
    m_timer.start();
    return ok;
}

bool WorkspaceState::saveImpl(const fs::path& path) const {
    if (!EnsureParentDirExists(path)) {
        return false;
    }

    YamlSerializer yaml;
    yaml.beginMap(false);

    yaml.beginKey("content_browser")
        .beginMap(false)
        .beginKey("current_path")
        .write(content_browser.current_path)
        .endMap();

    if (!tabs.empty()) {
        yaml.beginKey("tabs")
            .beginArray(false);
        for (const auto& tab : tabs) {
            yaml.beginMap(false)
                .beginKey("guid")
                .write(tab.guid);
            if (tab.active) {
                yaml.beginKey("active").write(tab.active);
            }
            if (tab.camera.is_some()) {
                yaml.beginKey("camera").write(tab.camera.unwrap_unchecked());
            }
            if (tab.transform.is_some()) {
                yaml.beginKey("transform").write(tab.transform.unwrap_unchecked());
            }
            yaml.endMap();
        }
        yaml.endArray();
    }

    yaml.endMap();

    if (auto res = SaveYaml(path.string(), yaml); !res) {
        LOG_ERROR(LogChannel::FS, "{}", ToString(res.error()));
        return false;
    }

    return true;
}

bool WorkspaceState::load(const std::filesystem::path& path) {
    if (!fs::exists(path)) {
        return true;
    }

    YAML::Node root;
    if (auto res = LoadYaml(path.string(), root); !res) {
        LOG_ERROR(LogChannel::FS, "{}", ToString(res.error()));
        return false;
    }

    YamlDeserializer yaml;
    yaml.initialize(root);
    IDeserializer& d = yaml;

    if (d.tryEnterKey("content_browser")) {
        if (d.tryEnterKey("current_path")) {
            d.read(content_browser.current_path);
            d.leaveKey();
        }
        d.leaveKey();
    }

    tabs.clear();
    if (d.tryEnterKey("tabs")) {
        const int size = d.arraySize().unwrap_or(0);
        for (int i = 0; i < size; ++i) {
            if (d.tryEnterIndex(i)) {
                tabs.resize(tabs.size() + 1);
                auto& tab_state = tabs.back();
                if (d.tryEnterKey("guid")) {
                    d.read(tab_state.guid);
                    d.leaveKey();
                }

                if (d.tryEnterKey("active")) {
                    d.read(tab_state.active);
                    d.leaveKey();
                }

                TransformComponent transform;
                if (d.tryEnterKey("transform")) {
                    if (d.read(transform)) {
                        tab_state.transform = Some(transform);
                    }
                    d.leaveKey();
                }

                CameraComponent camera;
                if (d.tryEnterKey("camera")) {
                    if (d.read(camera)) {
                        tab_state.camera = Some(camera);
                    }
                    d.leaveKey();
                }

                d.leaveIndex();
            }
        }
        d.leaveKey();
    }

    return true;
}

}  // namespace cave
