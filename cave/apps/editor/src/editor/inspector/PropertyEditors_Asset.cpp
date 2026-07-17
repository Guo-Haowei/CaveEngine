#include "PropertyEditors.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

// @TODO: refactor
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/ui/Inputs.h"

#include "editor/services/DragDropService.h"
#include "editor/utility/ContentEntry.h"

namespace cave {

namespace {

bool DrawAsset(const DrawComponentCtx& ctx,
               const char* name,
               Guid& guid) {
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, ui::kDefaultColumnWidth);
    ImGui::Text(ICON_FA_CUBE "  %s", name);
    ImGui::NextColumn();

    AssetHandle asset_handle;
    const AssetMetaData* meta = nullptr;
    if (auto handle_opt = ctx.engine_services.assetRegistry().findByGuid(guid)) {
        asset_handle = handle_opt.unwrap_unchecked();
        meta = asset_handle.meta();
        DEV_ASSERT(meta);
    }

    ImGui::Text(" %s ", meta ? meta->name.c_str() : "not set");

    const bool hovered = ImGui::IsItemHovered();

    auto& drag_drop = ctx.editor_services.dragDrop();

    bool dirty = false;
    if (auto handle_opt = drag_drop.dropAsset(meta ? meta->type : AssetType::All)) {
        auto handle = handle_opt.unwrap_unchecked();
        if (handle.guid() != guid) {
            dirty = true;
            guid = handle.guid();
        }
    }

    ImGui::Columns(1);
    if (hovered && meta) {
        ShowAssetToolTip(ctx.editor_services.thumbnail(), asset_handle);
    }
    return dirty;
}

}  // namespace

bool AssetEditor(const DrawComponentCtx& ctx,
                 void* component,
                 const FieldMetaBase* field) {
    return EditAndSubmit<Guid>(
        ctx,
        component,
        field,
        [&ctx](const char* label, Guid& guid) {
            return DrawAsset(ctx, label, guid);
        });
}

}  // namespace cave
