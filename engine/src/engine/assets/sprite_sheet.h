#pragma once
#include "asset_handle.h"
#include "asset_interface.h"

#include "engine/math/box.h"
#include "engine/math/geomath.h"

// clang-format off
namespace YAML { class Node; }
// clang-format on

namespace my {

class SpriteSheetAsset : public IAsset {
public:
    SpriteSheetAsset()
        : IAsset(AssetType::SpriteSheet) {}

    auto SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> override;

    auto LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> override;

    std::vector<Guid> GetDependencies() const override;

    AssetHandle& GetHandle() { return m_image_handle; }

    const Vector2i& GetSeparation() const { return m_separation; }

    void SetSeparation(const Vector2i& p_sep);

    void SetImage(const std::string& p_path);

    static constexpr const int VERSION = 1;

private:
    auto LoadFromDiskV0(const YAML::Node& p_node) -> Result<void>;
    auto LoadFromDiskV1(const YAML::Node& p_node) -> Result<void>;

    void SetHandle(AssetHandle&& p_handle);

    Guid m_image;
    Vector2i m_dimension{ 0, 0 };
    Vector2i m_separation{ 16, 16 };
    Vector2i m_offset{ Vector2i::Zero };

    /// Non serialized
    std::vector<Rect> m_frames;  // frames are calculated
    AssetHandle m_image_handle;
};

}  // namespace my
