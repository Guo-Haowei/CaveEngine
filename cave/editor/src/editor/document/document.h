#pragma once
#include "engine/assets/asset_handle.h"

namespace cave {

class UndoStack;

class Document {
public:
    Document(const Guid& p_guid);

    virtual ~Document();

    template<typename T>
    Handle<T> GetHandle() const {
        static_assert(requires { T::ASSET_TYPE; }, "T must define static constexpr ASSET_TYPE");
        AssetHandle copy = m_handle;
        return Handle<T>(std::move(copy));
    }

    const Guid& GetGuid() const { return m_guid; }

    bool Save();

    void Undo();
    void Redo();
    bool CanUndo() const;
    bool CanRedo() const;

    bool IsDirty() const { return m_dirty; }
    void SetDirty(float p_dirty = true) { m_dirty = p_dirty; }

protected:
    Guid m_guid;
    AssetHandle m_handle;
    bool m_dirty;

    std::unique_ptr<UndoStack> m_undo_stack;
};

}  // namespace cave
