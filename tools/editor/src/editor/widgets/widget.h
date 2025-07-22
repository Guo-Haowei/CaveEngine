#pragma once
#include "engine/math/geomath.h"

struct ImVec2;

namespace cave {

enum class AssetType : uint32_t;
class AssetHandle;
struct AssetMetaData;
class Guid;
struct IAsset;
struct ImageAsset;

constexpr float DEFAULT_COLUMN_WIDTH = 80.0f;

void PushDisabled();
void PopDisabled();

bool DrawDragInt(const char* p_label,
                 int& p_out,
                 float p_speed,
                 int p_min,
                 int p_max,
                 float p_column_width = DEFAULT_COLUMN_WIDTH);

bool DrawDragFloat(const char* p_label,
                   float& p_out,
                   float p_speed,
                   float p_min,
                   float p_max,
                   float p_column_width = DEFAULT_COLUMN_WIDTH);

bool DrawCheckBoxBitflag(const char* p_title, uint32_t& p_flags, const uint32_t p_bit);

bool DrawVec3Control(const char* p_label,
                     Vector3f& p_out,
                     float p_reset_value = 0.0f,
                     float p_column_width = DEFAULT_COLUMN_WIDTH);

bool DrawColorControl(const char* p_label,
                      Vector3f& p_out,
                      float p_reset_value = 1.0f,
                      float p_column_width = DEFAULT_COLUMN_WIDTH);

bool DrawInputText(const char* p_label,
                   std::string& p_string,
                   float p_text_width = DEFAULT_COLUMN_WIDTH,
                   float p_text_box_width = 0,
                   bool p_enter_returns_true = true);

bool DrawColorPicker3(const char* p_label,
                      float* p_out,
                      float p_column_width = DEFAULT_COLUMN_WIDTH);

bool DrawColorPicker4(const char* p_label,
                      float* p_out,
                      float p_column_width = DEFAULT_COLUMN_WIDTH);

bool ToggleButton(const char* p_str_id, bool& p_value);

Option<AssetHandle> DragDropTarget(AssetType p_mask);

/// image
void CenteredImage(const ImageAsset* p_image,
                   const ImVec2& p_background_region,
                   uint64_t p_background);

/// tool tip
void ShowAssetToolTip(const Guid& p_guid);

void ShowAssetToolTip(const AssetMetaData& p_meta, const IAsset* p_asset);

/// asset inspector
struct AssetChildPanel {
    const char* name;
    float width;
    std::function<void()> func;
};

void DrawContents(float p_full_width, const std::vector<AssetChildPanel>& p_descs);

/// image button
struct ToolBarButtonDesc {
    const char* display{ nullptr };
    const char* tooltip{ nullptr };
    std::function<void()> execute_func;
    std::function<bool()> is_enabled_func;
};

void DrawToolBarButton(const ToolBarButtonDesc& p_button);

}  // namespace cave
