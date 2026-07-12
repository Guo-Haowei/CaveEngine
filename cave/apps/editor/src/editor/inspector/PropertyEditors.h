#pragma once
#include "cave/core/variant/Variant.h"
#include "cave/runtime/framework/EngineServices.h"

#include "editor/services/EditService.h"
#include "editor/services/SelectionService.h"
#include "editor/services/ThumbnailService.h"
#include "editor/services/Workspace.h"

#include "editor/edit/ChangePropertyCmd.h"
#include "editor/edit/ChangeObjectPropertyCmd.h"
#include "editor/edit/AddComponentCmd.h"
#include "editor/edit/RemoveComponentCmd.h"

//
#include "engine/private/ui/inputs.h"

namespace cave {

class Scene;

struct DrawComponentCtx {
    EngineServices& services;
    EditService& edit;
    ThumbnailService& thumbnail;
    Scene* scene;
    ecs::Entity entity;
    DocId doc_id;

    ComponentId cid;
};

template<typename ValueT, typename UIFunc>
bool EditAndSubmit(const DrawComponentCtx& ctx,
                   void* component,
                   const FieldMetaBase* field,
                   UIFunc&& ui_func) {
    ValueT old_v = field->template GetData<ValueT>(component);
    ValueT new_v = old_v;
    if (!ui_func(field->name, new_v)) {
        return false;
    }

    if constexpr (std::is_trivially_copyable_v<ValueT>) {
        auto cmd = MakeOwner<ChangePropertyCmd>(
            ctx.services.sceneRegistry(),
            ctx.entity,
            ctx.cid,
            field->id,
            old_v,
            new_v);
        ctx.edit.submit(ctx.doc_id, std::move(cmd));
    } else {
        auto cmd = MakeOwner<ChangeObjectPropertyCmd<ValueT>>(
            ctx.services.sceneRegistry(),
            ctx.entity,
            ctx.cid,
            field->id,
            std::move(old_v),
            std::move(new_v));
        ctx.edit.submit(ctx.doc_id, std::move(cmd));
    }

    return true;
}

// @TODO: refactor DrawComponent
template<ComponentType T, typename UIFunction>
static void DrawComponent(const std::string& p_name,
                          const DrawComponentCtx& ctx,
                          T* p_component,
                          UIFunction p_function) {
    const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                             ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
                                             ImGuiTreeNodeFlags_FramePadding;
    if (p_component) {
        ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
        float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImGui::Separator();
        bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, "%s", p_name.c_str());
        ImGui::PopStyleVar();
        ImGui::SameLine(contentRegionAvailable.x - line_height * 0.5f);
        if (ImGui::Button("-", ImVec2{ line_height, line_height })) {
            ImGui::OpenPopup("ComponentSettings");
        }

        if (ImGui::BeginPopup("ComponentSettings")) {
            if (ImGui::MenuItem("remove component")) {
                auto cmd = MakeOwner<RemoveComponentCmd<T>>(
                    ctx.services.sceneRegistry(),
                    ctx.entity,
                    *p_component);
                ctx.edit.submit(ctx.doc_id, std::move(cmd));
            }

            ImGui::EndPopup();
        }

        if (open) {
            p_function(*p_component);
            ImGui::TreePop();
        }
    }
}

bool DrawAsset(const DrawComponentCtx& ctx,
               const char* name,
               Guid& guid);

bool DrawVariantMap(const char* label, VariantMap& map);

}  // namespace cave
