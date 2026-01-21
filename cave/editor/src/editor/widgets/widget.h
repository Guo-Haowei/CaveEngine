#pragma once
#include "engine/math/geomath.h"

struct ImVec2;

namespace cave {

enum class AssetType : uint32_t;
class AssetHandle;
struct AssetMetaData;
class Guid;
class IAsset;
struct ImageAsset;

constexpr float DEFAULT_COLUMN_WIDTH = 150.0f;

void PushDisabled();
void PopDisabled();

bool DrawCheckBoxBitflag(const char* p_title, uint32_t& p_flags, const uint32_t p_bit);

bool DrawColorPicker3(const char* p_label,
                      float* p_out,
                      float p_column_width = DEFAULT_COLUMN_WIDTH);

bool DrawColorPicker4(const char* p_label,
                      float* p_out,
                      float p_column_width = DEFAULT_COLUMN_WIDTH);

bool ToggleButton(const char* p_str_id, bool& p_value);

/// image
void CenteredImage(const ImageAsset* p_image,
                   const ImVec2& p_background_region,
                   uint64_t p_background);

/// asset inspector
struct AssetChildPanel {
    const char* name;
    float width;
    std::function<void()> func;
};

void DrawContents(float p_full_width, const std::vector<AssetChildPanel>& p_descs);

}  // namespace cave
