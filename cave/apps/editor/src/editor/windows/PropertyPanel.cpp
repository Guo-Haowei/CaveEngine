#include "PropertyPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/ui/UIComponents.h"

#include "editor/EditorState.h"
#include "editor/inspector/PropertyEditors.h"
#include "editor/utility/ContentEntry.h"
#include "editor/services/DragDropService.h"
#include "editor/services/SelectionService.h"
#include "editor/services/Workspace.h"

// @TODO: refactor
#include "engine/private/core/reflection/MetaEditor.h"
#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/ui/Layout.h"

namespace cave {

using namespace ::cave::literals;
using namespace ::cave::math;

// @TODO: refactor this
// @TODO: add motor to add velocity, collider, contact, etc
// @TODO: add motor2d?
#define COMPONENT_LIST             \
    COMPONENT_DECL(Camera)         \
    COMPONENT_DECL(NativeScript)   \
    COMPONENT_DECL(SpriteAnimator) \
    COMPONENT_DECL(Collider)       \
    COMPONENT_DECL(Trigger)        \
    COMPONENT_DECL(Velocity)       \
    COMPONENT_DECL(Motor)          \
    COMPONENT_DECL(Background)     \
    COMPONENT_DECL(MeshRenderer)   \
    COMPONENT_DECL(SpriteRenderer) \
    COMPONENT_DECL(TileMapLayer)   \
    COMPONENT_DECL(Facing)

template<ComponentType T>
void DrawComponentAuto(std::string_view name, const DrawObjectCtx& ctx) {
    T* component = ctx.scene->component<T>(ctx.entity);
    if (!component) return;

    const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                             ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
                                             ImGuiTreeNodeFlags_FramePadding;
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
        DrawObjectAuto<T>(component, ctx);
        ImGui::TreePop();
    }
}

void PropertyPanel::drawUIImpl() {
    CAVE_PROFILE_EVENT();

    PreviewScene preview = m_editor_services.workspace().focusedPreviewScene();
    if (!preview.scene) {
        return;
    }

    SelectionKey selection = m_editor_services.selection().primary(preview.doc_id);

    ecs::Entity id = selection.entity;
    if (!id.valid()) {
        return;
    }

    Scene& scene = *preview.scene;
    DocId doc_id = preview.doc_id;

    NameComponent* name_component = scene.component<NameComponent>(id);
    if (!name_component) {
        return;
    }

    EditService& edit_service = m_editor_services.edit();

    const DrawObjectCtx ctx{
        .engine_services = m_engine_services,
        .editor_services = m_editor_services,
        .doc_id = doc_id,
        .type_id = StringId{},
        .scene = &scene,
        .entity = id,
    };

    {
        FixedString<64> name = name_component->nameRef();
        if (ui::TextBox("Name", name)) {
            auto cmd = MakeOwner<ChangePropertyCmd>(
                m_engine_services.sceneRegistry(),
                ComponentPropertyTarget{ id, NameComponent_Id, CAVE_SID("name") },
                name_component->nameRef(),
                name);
            edit_service.submit(doc_id, std::move(cmd));
        }
    }

    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (ImGui::Button(ICON_FA_SQUARE_PLUS)) {
        ImGui::OpenPopup("AddComponentPopup");
    }

    auto create_component = [&](ComponentId cid) {
        if (scene.storage().has(cid, id)) {
            LOG_ERROR("object {} already has component {}",
                      name_component->name(),
                      cid.hash());
            return;
        }
        auto cmd = MakeOwner<AddComponentCmd>(
            m_engine_services.sceneRegistry(),
            id,
            cid);
        edit_service.submit(doc_id, std::move(cmd));
    };

    if (ImGui::BeginPopup("AddComponentPopup")) {
        if (ImGui::MenuItem("Rigid Body")) {
            LOG_ERROR("TODO: implement add component");
            ImGui::CloseCurrentPopup();
        }

#define COMPONENT_DECL(NAME)                  \
    if (ImGui::MenuItem(#NAME)) {             \
        create_component(NAME##Component_Id); \
    }
        COMPONENT_LIST
#undef COMPONENT_DECL

        ImGui::EndPopup();
    }

    // @TODO: see how much this can be done with meta table
    TransformComponent* transform = scene.component<TransformComponent>(id);
    LightComponent* light = scene.component<LightComponent>(id);
    MaterialComponent* material = scene.component<MaterialComponent>(id);
    ColliderComponent* collider = scene.component<ColliderComponent>(id);
    NativeScriptComponent* native_script = scene.component<NativeScriptComponent>(id);
    CameraComponent* camera = scene.component<CameraComponent>(id);

#define DRAW_COMPONENT_ARGS(DISPLAY) DISPLAY, ctx

    DrawComponent(DRAW_COMPONENT_ARGS("Transform"), transform, [&ctx, &camera](TransformComponent& transform) {
        const math::Mat4f old_transform = transform.localMatrix();

        TransformComponent copy = transform;
        const bool dirty = DrawObjectAuto<TransformComponent>(&copy, ctx);
        if (dirty && camera) {
            camera->setDirty();
        }
    });

    DrawComponent(DRAW_COMPONENT_ARGS("Light"), light, [&ctx, &material](LightComponent& light) {
        bool dirty = DrawObjectAuto<LightComponent>(&light, ctx);
        if (dirty) {
            light.SetDirty();
        }

        if (material) {
            DrawObjectAuto<MaterialComponent>(material, ctx);
        }
    });

    DrawComponent(DRAW_COMPONENT_ARGS("Native Script"), native_script, [&](NativeScriptComponent& script) {
        // @TODO: fix this
        FixedString<32>& name = script.name;
        ui::TextBox("class_name", name);

        DrawObjectAuto<NativeScriptComponent>(&script, ctx);
    });

    DrawComponent(DRAW_COMPONENT_ARGS("Collider"), collider, [&](ColliderComponent& collider) {
        DrawObjectAuto<ColliderComponent>(&collider, ctx);

        Shape& shape = collider.shape();
        DrawEnumDropDown("shape", shape.type, ui::kDefaultColumnWidth);
        switch (shape.type) {
            case ShapeType::Round: {
                ui::InputFloat("radius", shape.data.radius);
            } break;
            case ShapeType::Box: {
                ui::Float3("half", shape.data.half, 0.5f);
            } break;
            default:
                ui::Float3("placeholder", shape.data.half, 0.5f);
                break;
        }
    });

    DrawComponentAuto<FacingComponent>("Facing", ctx);
    DrawComponentAuto<VelocityComponent>("Velocity", ctx);
    DrawComponentAuto<MotorComponent>("Motor", ctx);
    DrawComponentAuto<SpriteRendererComponent>("SpriteRenderer", ctx);
    DrawComponentAuto<BackgroundComponent>("Background", ctx);
    DrawComponentAuto<TileMapLayerComponent>("TileMapLayer", ctx);
    DrawComponentAuto<SpriteAnimatorComponent>("SpriteAnimator", ctx);

    DrawComponentAuto<UICanvasComponent>("UICanvas", ctx);
    DrawComponentAuto<UIRectTransformComponent>("UIRect", ctx);
    DrawComponentAuto<UIButtonComponent>("UIButton", ctx);
    DrawComponentAuto<UIImageComponent>("UIImage", ctx);

    DrawComponent(
        DRAW_COMPONENT_ARGS("SkeletalAnimation"),
        scene.component<SkeletalAnimationComponent>(id),
        [&](SkeletalAnimationComponent& p_anim) {
            DrawObjectAuto<SkeletalAnimationComponent>(&p_anim, ctx);
            ImGui::Separator();
            const float start = p_anim.GetStart();
            const float end = p_anim.GetEnd();
            float timer = p_anim.GetTimer();

            if (ImGui::SliderFloat("Frame", &timer, start, end)) {
                p_anim.SetPlaying();
                p_anim.SetTimer(timer);
            }
        });

    MeshRendererComponent* mesh_renderer = scene.component<MeshRendererComponent>(id);
    DrawComponent(DRAW_COMPONENT_ARGS("MeshRenderer"), mesh_renderer, [&](MeshRendererComponent& p_render) {
        DrawObjectAuto<MeshRendererComponent>(&p_render, ctx);

        if (ImGui::Button("+")) {
            // @TODO: command
            DEV_ASSERT(0 && "should go through cb");
#if 0
            auto& materials = p_render.GetMaterialInstances();
            auto name = std::format("mat_{}", materials.size());
            auto mat_id = EntityFactory::CreateNameEntity(scene, name);
            scene.create<MaterialComponent>(mat_id);
            p_render.AddMaterial(mat_id);
#endif
        }

        for (ecs::Entity id : p_render.materialInstances()) {
            if (MaterialComponent* material = scene.component<MaterialComponent>(id); material) {
                DrawObjectCtx copy_ctx = ctx;
                copy_ctx.entity = id;
                DrawObjectAuto<MaterialComponent>(material, copy_ctx);
            }
        }
    });

    DrawComponent(DRAW_COMPONENT_ARGS("Camera"), camera, [&](CameraComponent& p_camera) {
        if (DrawObjectAuto<CameraComponent>(&p_camera, ctx)) {
            p_camera.setDirty();
        }
    });
}

}  // namespace cave
