#pragma once
#include "cave/core/variant/Variant.h"
#include "cave/runtime/framework/EngineServices.h"

#include "editor/services/EditorServices.h"
#include "editor/services/EditService.h"

#include "editor/edit/ChangePropertyCmd.h"
#include "editor/edit/ChangeObjectPropertyCmd.h"
#include "editor/edit/AddComponentCmd.h"
#include "editor/edit/RemoveComponentCmd.h"

#include "engine/private/runtime/ui/Inputs.h"

namespace cave {

class Scene;

struct DrawObjectCtx {
    EngineServices& engine_services;
    EditorServices& editor_services;
    DocId doc_id;
    StringId type_id;

    Scene* scene;
    ecs::Entity entity;
};

template<typename ValueT, typename UIFunc>
bool EditAndSubmit(const DrawObjectCtx& ctx,
                   void* component,
                   const FieldMetaBase* field,
                   UIFunc&& ui_func) {
    ValueT old_v = field->template GetData<ValueT>(component);
    ValueT new_v = old_v;
    if (!ui_func(field->name, new_v)) {
        return false;
    }

    auto& edit = ctx.editor_services.edit();

    if constexpr (std::is_trivially_copyable_v<ValueT>) {
        auto cmd = MakeOwner<ChangePropertyCmd>(
            ctx.engine_services.sceneRegistry(),
            ComponentPropertyTarget{ ctx.entity, ctx.type_id, field->id },
            old_v,
            new_v);
        edit.submit(ctx.doc_id, std::move(cmd));
    } else {
        auto cmd = MakeOwner<ChangeObjectPropertyCmd<ValueT>>(
            ctx.engine_services.sceneRegistry(),
            ctx.entity,
            ctx.type_id,
            field->id,
            std::move(old_v),
            std::move(new_v));
        edit.submit(ctx.doc_id, std::move(cmd));
    }

    return true;
}

// @TODO: refactor DrawComponent
template<ComponentType T, typename UIFunction>
static void DrawComponent(std::string_view name,
                          const DrawObjectCtx& ctx,
                          T* component,
                          UIFunction function) {
    const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                             ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
                                             ImGuiTreeNodeFlags_FramePadding;
    if (component) {
        ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
        float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImGui::Separator();
        bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, "%s", name.data());
        ImGui::PopStyleVar();
        ImGui::SameLine(contentRegionAvailable.x - line_height * 0.5f);
        if (ImGui::Button("-", ImVec2{ line_height, line_height })) {
            ImGui::OpenPopup("ComponentSettings");
        }

        if (ImGui::BeginPopup("ComponentSettings")) {
            if (ImGui::MenuItem("remove component")) {
                auto cmd = MakeOwner<RemoveComponentCmd<T>>(
                    ctx.engine_services.sceneRegistry(),
                    ctx.entity,
                    *component);
                ctx.editor_services.edit().submit(ctx.doc_id, std::move(cmd));
            }

            ImGui::EndPopup();
        }

        if (open) {
            function(*component);
            ImGui::TreePop();
        }
    }
}

bool DrawAsset(const DrawObjectCtx& ctx,
               const char* name,
               Guid& guid);

bool AssetEditor(const DrawObjectCtx& ctx,
                 void* object,
                 const FieldMetaBase* property);

bool DrawVariantMap(const char* label, VariantMap& map);

bool DrawPropertyAuto(const FieldMetaBase* property,
                      void* object,
                      const DrawObjectCtx& ctx);

bool DrawObjectAuto(StringId type_id, void* object, const DrawObjectCtx& ctx);

template<ComponentType T>
bool DrawObjectAuto(void* object, const DrawObjectCtx& ctx) {
    return DrawObjectAuto(T::kId, object, ctx);
}

}  // namespace cave
