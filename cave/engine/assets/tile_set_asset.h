#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/assets/asset_interface.h"

#include "engine/math/box.h"
#include "engine/reflection/reflection.h"

namespace cave {

class TileSetAsset : public IAsset {
    CAVE_ASSET(TileSetAsset, AssetType::TileSet, 0)

    CAVE_META(TileSetAsset)

private:
    CAVE_PROP(editor = Asset)
    Guid m_image_guid;

    CAVE_PROP(editor = DragFloat, min = 0.01f, max = 100.0f)
    float m_tile_scale = 1.0f;

    CAVE_PROP()
    uint32_t m_width = 0;

    CAVE_PROP()
    uint32_t m_height = 0;

    CAVE_PROP(editor = InputInt, min = 1, max = 1000)
    uint32_t m_row = 1;

    CAVE_PROP(editor = InputInt, min = 1, max = 1000)
    uint32_t m_column = 1;

    /// Non serialized
    std::vector<Rect> m_frames;  // frames are calculated
    Handle<ImageAsset> m_image_handle;
    bool m_dirty;

public:
    uint32_t GetRow() const { return m_row; }
    void SetRow(uint32_t p_row);

    uint32_t GetCol() const { return m_column; }
    void SetCol(uint32_t p_col);

    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }

    float GetScale() const { return m_tile_scale; }
    void SetScale(float p_scale);

    void SetImage(const Guid& p_guid);
    const Handle<ImageAsset>& GetHandle() const { return m_image_handle; }

    auto SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> override;
    auto LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> override;

    const Guid& GetImageGuid() const { return m_image_guid; }

    const auto& GetFrames() const { return m_frames; }

    bool IsDirty() const { return m_dirty; }
    void SetDirty(bool p_dirty = true) { m_dirty = p_dirty; }

    std::vector<Guid> GetDependencies() const override;

private:
    void SetHandle(Handle<ImageAsset>&& p_handle);
    void UpdateFrames();
};

}  // namespace cave
