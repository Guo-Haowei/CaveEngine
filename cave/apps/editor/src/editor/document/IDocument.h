#pragma once
#include "DocId.h"

#include "cave/core/ids/SceneId.h"

#include "cave/runtime/assets/AssetHandle.h"

namespace cave {

class IEditCmd;

struct DocInfo {
    DocId id{};
    std::string title;  // "MyScene.scene"
    std::string path;   // on disk (can be empty for new/untitled)
    bool is_dirty{ false };
};

class IDocument {
public:
    virtual ~IDocument() = default;

    virtual bool apply(std::unique_ptr<IEditCmd> cmd,
                       uint32_t coalesce) = 0;

    virtual bool canUndo() const = 0;
    virtual bool canRedo() const = 0;

    virtual bool undo() = 0;
    virtual bool redo() = 0;

    virtual void markSaved() = 0;
    virtual bool isDirty() const = 0;

    virtual bool save() = 0;
    virtual bool saveAs(std::string_view new_path) = 0;

    virtual void undoLabels(std::vector<std::string>& out, int max_items) const = 0;
    virtual void redoLabels(std::vector<std::string>& out, int max_items) const = 0;

    AssetHandle rawHandle() const {
        return m_handle;
    }

    template<typename T>
    Handle<T> handle() const {
        static_assert(requires { T::ASSET_TYPE; }, "T must define static constexpr ASSET_TYPE");
        AssetHandle copy = m_handle;
        return Handle<T>(std::move(copy));
    }

    virtual Guid guid() const = 0;

    // @TODO: remove this, not all doc is related to scene
    virtual SceneId previewScene() const { return {}; }

    virtual std::unique_ptr<Scene> createPreviewScene() const = 0;
    virtual void reloadPreviewScene() = 0;

protected:
    AssetHandle m_handle;
    AssetRef m_asset;
};

}  // namespace cave
