#include "PropertyEditors.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "cave/core/reflection/MetaRegistry.h"

#include "editor/services/DragDropService.h"
#include "editor/utility/ContentEntry.h"

// @TODO: refactor
#include "engine/private/core/math/Geomath.h"
#include "engine/private/runtime/framework/Engine.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/ui/Inputs.h"

namespace cave {

using namespace math;

bool DrawAsset(const DrawObjectCtx& ctx,
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

bool AssetEditor(const DrawObjectCtx& ctx,
                 void* object,
                 const FieldMetaBase* field) {
    return EditAndSubmit<Guid>(
        ctx,
        object,
        field,
        [&ctx](const char* label, Guid& guid) {
            return DrawAsset(ctx, label, guid);
        });
}

bool DrawPropertyAuto(const FieldMetaBase* property,
                      void* object,
                      const DrawObjectCtx& ctx) {
    switch (property->editor_hint) {
        case EditorHint::Asset: {
            return AssetEditor(ctx, object, property);
        } break;
        case EditorHint::EnumDropDown:
            return property->DrawEditor(object, ui::kDefaultColumnWidth);
        case EditorHint::Toggle:
            return EditAndSubmit<bool>(
                ctx, object, property,
                [](const char* label, bool& value) {
                    return ui::CheckBox(label, value);
                });
        case EditorHint::InputText:
            return EditAndSubmit<String>(
                ctx, object, property,
                [](const char* label, String& value) {
                    return ui::TextBox(label, value);
                });
        case EditorHint::InputInt:
            return EditAndSubmit<int>(
                ctx, object, property,
                [](const char* label, int& value) {
                    return ui::InputInt(label, value);
                });
        case EditorHint::InputFloat:
            return EditAndSubmit<float>(
                ctx, object, property,
                [](const char* label, float& value) {
                    return ui::InputFloat(label, value);
                });
        case EditorHint::BitMask: {
            return EditAndSubmit<uint32_t>(
                ctx, object, property,
                [](const char* label, uint32_t& value) {
                    return ui::DrawBitMask32(label, value);
                });
        } break;
        case EditorHint::DragInt:
            BreakIfDebug();
            return false;
        case EditorHint::DragFloat:
            return EditAndSubmit<float>(
                ctx, object, property,
                [&](const char* label, float& value) {
                    return ui::DragFloat(label,
                                         value,
                                         0.01f,
                                         property->v_min,
                                         property->v_max);
                });
        case EditorHint::Color:
            return EditAndSubmit<Vec4f>(
                ctx, object, property,
                [](const char* label, Vec4f& value) {
                    return ui::ColorPicker4(label, value);
                });
        case EditorHint::Translation2D:
            return EditAndSubmit<Vec2f>(
                ctx, object, property,
                [](const char* label, Vec2f& value) {
                    return ui::Float2(label, value, 0.0f);
                });
        case EditorHint::Translation:
            return EditAndSubmit<Vec3f>(
                ctx, object, property,
                [](const char* label, Vec3f& value) {
                    return ui::Float3(label, value, 0.0f);
                });
        case EditorHint::Scale:
            return EditAndSubmit<Vec3f>(
                ctx, object, property,
                [](const char* label, Vec3f& value) {
                    return ui::Float3(label, value, 1.0f);
                });
        case EditorHint::Rotation: {
            // @TODO: fix this
            Vec4f& q = property->template GetData<Vec4f>(object);
            glm::vec3 euler_ = glm::eulerAngles(glm::quat(q.w, q.x, q.y, q.z));
            Vec3f euler = *reinterpret_cast<Vec3f*>(&euler_);
            constexpr float RAD_TO_DEG = 180.0f / glm::pi<float>();
            constexpr float DEG_TO_RAD = glm::pi<float>() / 180.0f;
            euler *= RAD_TO_DEG;

            if (!ui::Float3(property->name, euler, 0.0f)) {
                return false;
            }

            euler *= DEG_TO_RAD;
            glm::quat q2 = glm::quat(reinterpret_cast<glm::vec3&>(euler));

            Vec4f old_v = q;
            Vec4f new_v{ q2.x, q2.y, q2.z, q2.w };

            auto cmd = MakeOwner<ChangePropertyCmd>(
                ctx.engine_services.sceneRegistry(),
                ComponentPropertyTarget{ ctx.entity, ctx.type_id, property->id },
                old_v,
                new_v);
            ctx.editor_services.edit().submit(ctx.doc_id, std::move(cmd));
            return true;
        } break;
        case EditorHint::VariantMap: {
            return EditAndSubmit<VariantMap>(
                ctx, object, property,
                [](const char* label, VariantMap& map) {
                    return DrawVariantMap(label, map);
                });
        } break;
        default:
            return false;
    }
}

bool DrawObjectAuto(StringId type_id, void* object, const DrawObjectCtx& ctx) {
    auto meta_table = engine::GetMetaRegistry().tryGet(type_id);
    if (!DEV_VERIFY(meta_table)) {
        return false;
    }

    auto props = meta_table->props;
    DrawObjectCtx ctx2 = ctx;
    ctx2.type_id = type_id;

    int dirty = 0;
    for (const auto& field : props) {
        dirty |= (int)DrawPropertyAuto(field, object, ctx2);
    }
    return (int)dirty;
}

}  // namespace cave
