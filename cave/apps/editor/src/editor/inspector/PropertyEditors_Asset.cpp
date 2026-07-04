#include "PropertyEditors.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

// @TODO: refactor
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/ui/inputs.h"
#include "editor/widgets/DragDrop.h"
#include "editor/utility/ContentEntry.h"

namespace cave {

bool DrawAsset(const DrawComponentCtx& ctx,
               const char* name,
               const Guid& guid,
               std::function<bool(const AssetMetaData&)>&& callback) {
    auto handle_ = ctx.services.assetRegistry().findByGuid(guid);

    const AssetMetaData* meta = nullptr;
    const IAsset* asset = nullptr;

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, ui::kDefaultColumnWidth);
    ImGui::Text(ICON_FA_CUBE "  %s", name);
    ImGui::NextColumn();

    if (handle_.is_some()) {
        AssetHandle handle = handle_.unwrap_unchecked();
        meta = handle.meta();
        DEV_ASSERT(meta);
        asset = handle.get();
    }

    ImGui::Text(" %s ", meta ? meta->name.c_str() : "not set");

    const bool hovered = ImGui::IsItemHovered();

    bool dirty = false;
    if (callback) {
        dirty = callback(*meta);
    }

    ImGui::Columns(1);
    if (hovered && meta) {
        ShowAssetToolTip(ctx.thumbnail, handle_.unwrap_unchecked());
    }
    return dirty;
}

}  // namespace cave
