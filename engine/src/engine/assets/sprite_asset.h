#pragma once
#include <nlohmann/json_fwd.hpp>

#include "asset_handle.h"
#include "asset_interface.h"

#include "engine/math/box.h"

namespace cave {

class SpriteAsset : public IAsset {
    DECLARE_ASSET(SpriteAsset, AssetType::Sprite)
public:
    static constexpr const int VERSION = 0;

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

    std::vector<Guid> GetDependencies() const override;

private:
    void LoadFromDiskCurrent(const nlohmann::json& j);

    void SetHandle(Handle<ImageAsset>&& p_handle);
    void UpdateFrames();

    Guid m_image_guid;
    float m_tile_scale = 1.0f;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_row = 1;
    uint32_t m_column = 1;

    /// Non serialized
    std::vector<Rect> m_frames;  // frames are calculated
    Handle<ImageAsset> m_image_handle;
};

}  // namespace cave
