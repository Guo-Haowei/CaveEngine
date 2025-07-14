#pragma once
#include "asset_handle.h"
#include "asset_interface.h"

#include "engine/math/box.h"

// clang-format off
namespace YAML { class Node; }
// clang-format on

namespace my {

class SpriteSheetAsset : public IAsset {
    DECLARE_ASSET(SpriteSheetAsset, AssetType::SpriteSheet)
public:
    static constexpr const int VERSION = 2;

    auto SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> override;

    auto LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> override;

    std::vector<Guid> GetDependencies() const override;

    Handle<ImageAsset>& GetHandle() { return m_image_handle; }

    void SetImage(const std::string& p_path);

    uint32_t GetRow() const { return m_row; }
    uint32_t GetCol() const { return m_column; }
    void SetRow(uint32_t p_row);
    void SetCol(uint32_t p_col);

    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }

private:
    auto LoadFromDiskCurrent(const YAML::Node& p_node) -> Result<void>;

    void SetHandle(Handle<ImageAsset>&& p_handle);
    void UpdateFrames();

    Guid m_image;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_row = 1;
    uint32_t m_column = 1;

    /// Non serialized
    std::vector<Rect> m_frames;  // frames are calculated
    Handle<ImageAsset> m_image_handle;
};

}  // namespace my
