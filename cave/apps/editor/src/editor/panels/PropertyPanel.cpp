#include "PropertyPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

#include "editor/edit/ChangePropertyCmd.h"
#include "editor/edit/AddComponentCmd.h"
#include "editor/edit/RemoveComponentCmd.h"
#include "editor/services/EditService.h"
#include "editor/services/SelectionService.h"
#include "editor/services/Workspace.h"

// @TODO: refactor

#include "engine/private/core/reflection/MetaEditor.h"
#include "engine/private/runtime/assets/SpriteAnimationAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/ui/inputs.h"
#include "engine/private/ui/layout.h"

#include "editor/EditorState.h"
#include "editor/utility/ContentEntry.h"
#include "editor/widgets/DragDrop.h"

namespace cave {

using namespace ::cave::literals;
using namespace ::cave::math;

// @TODO: refactor this
#define COMPONENT_LIST              \
    COMPONENT_DECL(Camera)          \
    COMPONENT_DECL(LuaScript)       \
    COMPONENT_DECL(NativeScript)    \
    COMPONENT_DECL(SpriteAnimator)  \
    COMPONENT_DECL(Collider)        \
    COMPONENT_DECL(Velocity)        \
    COMPONENT_DECL(MeshRenderer)    \
    COMPONENT_DECL(SpriteRenderer)  \
    COMPONENT_DECL(Facing)          \
    COMPONENT_DECL(TileMapInstance) \
    COMPONENT_DECL(PrefabInstance)

struct DrawComponentCtx {
    IApplication& app;
    EditService& edit;
    ThumbnailService& thumbnail;
    Scene* scene;
    ecs::Entity entity;
    DocId doc_id;
};

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
                auto cmd = std::make_unique<RemoveComponentCmd<T>>(
                    ctx.app.services().sceneRegistry(),
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

template<typename T>
concept HasSetResourceGuid = requires(T& t, const Guid& guid) {
    { t.SetResourceGuid(guid) } -> std::same_as<bool>;
};

static_assert(HasSetResourceGuid<LuaScriptComponent>);

// @TODO: make this an editable command
template<typename T>
bool DrawAsset(const char* p_name,
               const Guid& p_guid,
               T* p_component,
               const DrawComponentCtx& p_context) {
    auto handle_ = AssetRegistry::singleton().findByGuid(p_guid);

    AssetType type = AssetType::All;
    const AssetMetaData* meta = nullptr;
    const IAsset* asset = nullptr;

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, ui::kDefaultColumnWidth);
    ImGui::Text(ICON_FA_CUBE "  %s", p_name);
    ImGui::NextColumn();

    if (handle_.is_some()) {
        AssetHandle handle = handle_.unwrap_unchecked();
        meta = handle.meta();
        DEV_ASSERT(meta);
        asset = handle.get();
        type = meta->type;
    }

    ImGui::Text(" %s ", meta ? meta->name.c_str() : "not set");

    bool dirty = false;

    const bool hovered = ImGui::IsItemHovered();
    if (auto _handle = DragDropTarget(type); _handle.is_some()) {
        if constexpr (HasSetResourceGuid<T>) {
            if (p_component) {
                dirty = p_component->SetResourceGuid(_handle.unwrap_unchecked().guid());
            }
        }
    }

    ImGui::Columns(1);
    if (hovered && meta) {
        ShowAssetToolTip(p_context.thumbnail, handle_.unwrap_unchecked());
    }
    return dirty;
};

template<typename ComponentT, typename ValueT, typename UiFn>
bool EditAndSubmit(const DrawComponentCtx& p_ctx,
                   ComponentT* p_component,
                   const FieldMetaBase* p_field,
                   UiFn&& p_ui_fn) {
    static_assert(std::is_trivially_copyable_v<ValueT>);

    ValueT old_v = p_field->template GetData<ValueT>(p_component);
    ValueT new_v = old_v;
    if (!p_ui_fn(p_field->name, new_v)) {
        return false;
    }

    auto cmd = std::make_unique<ChangePropertyCmd>(
        p_ctx.app.services().sceneRegistry(),
        p_ctx.entity,
        p_component->GetId(),
        p_field->id,
        old_v,
        new_v);
    p_ctx.edit.submit(p_ctx.doc_id, std::move(cmd));
    return true;
}

template<typename T>
bool DrawPropertyAuto(const FieldMetaBase* p_property,
                      T* p_component,
                      const DrawComponentCtx& p_ctx) {
    switch (p_property->editor_hint) {
        case EditorHint::EnumDropDown:
            return p_property->DrawEditor(p_component, ui::kDefaultColumnWidth);
        case EditorHint::Toggle:
            return EditAndSubmit<T, bool>(
                p_ctx, p_component, p_property,
                [](const char* p_label, bool& p_value) {
                    return ui::CheckBox(p_label, p_value);
                });
        case EditorHint::InputInt:
            return (int)EditAndSubmit<T, int>(
                p_ctx, p_component, p_property,
                [](const char* p_label, int& p_value) {
                    return ui::InputInt(p_label, p_value);
                });
        case EditorHint::InputFloat:
            return EditAndSubmit<T, float>(
                p_ctx, p_component, p_property,
                [](const char* p_label, float& p_value) {
                    return ui::InputFloat(p_label, p_value);
                });
        case EditorHint::DragInt:
            BreakIfDebug();
            return false;
        case EditorHint::DragFloat:
            return EditAndSubmit<T, float>(
                p_ctx, p_component, p_property,
                [&](const char* p_label, float& p_value) {
                    return ui::DragFloat(p_label,
                                         p_value,
                                         0.01f,
                                         p_property->v_min,
                                         p_property->v_max);
                });
        case EditorHint::Color:
            return EditAndSubmit<T, Vec4f>(
                p_ctx, p_component, p_property,
                [](const char* p_label, Vec4f& p_value) {
                    return ui::ColorPicker4(p_label, p_value);
                });
        case EditorHint::Translation:
            return EditAndSubmit<T, Vec3f>(
                p_ctx, p_component, p_property,
                [](const char* p_label, Vec3f& p_value) {
                    return ui::Float3(p_label, p_value, 0.0f);
                });
        case EditorHint::Scale:
            return EditAndSubmit<T, Vec3f>(
                p_ctx, p_component, p_property,
                [](const char* p_label, Vec3f& p_value) {
                    return ui::Float3(p_label, p_value, 1.0f);
                });
        case EditorHint::Rotation: {
            // @TODO: fix this
            Vec4f& q = p_property->template GetData<Vec4f>(p_component);
            glm::vec3 euler_ = glm::eulerAngles(glm::quat(q.w, q.x, q.y, q.z));
            Vec3f euler = *reinterpret_cast<Vec3f*>(&euler_);
            constexpr float RAD_TO_DEG = 180.0f / glm::pi<float>();
            constexpr float DEG_TO_RAD = glm::pi<float>() / 180.0f;
            euler *= RAD_TO_DEG;

            if (!ui::Float3(p_property->name, euler, 0.0f)) {
                return false;
            }

            euler *= DEG_TO_RAD;
            glm::quat q2 = glm::quat(reinterpret_cast<glm::vec3&>(euler));

            Vec4f old_v = q;
            Vec4f new_v{ q2.x, q2.y, q2.z, q2.w };

            auto cmd = std::make_unique<ChangePropertyCmd>(
                p_ctx.app.services().sceneRegistry(),
                p_ctx.entity,
                p_component->GetId(),
                p_property->id,
                old_v,
                new_v);
            p_ctx.edit.submit(p_ctx.doc_id, std::move(cmd));
            return true;
        } break;
        case EditorHint::Asset: {
            const Guid& guid = p_property->template GetData<Guid>(p_component);
            return DrawAsset(p_property->name, guid, p_component, p_ctx);
        } break;
        default:
            return false;
    }
}

template<typename T>
bool DrawComponentAuto(T* p_component, const DrawComponentCtx& p_ctx) {
    const MetaTableFields& meta_table = MetaDataTable<T>::GetFields();

    int dirty = 0;
    for (const auto& field : meta_table) {
        dirty |= (int)DrawPropertyAuto(field, p_component, p_ctx);
    }
    return (int)dirty;
}

void PropertyPanel::drawUIImpl() {
    CAVE_PROFILE_EVENT();

    PreviewScene preview = editor_services_.workspace().focusedPreviewScene();
    if (!preview.scene) {
        return;
    }

    SelectionKey selection = editor_services_.selection().Primary(preview.doc_id);

    ecs::Entity id = selection.entity;
    if (!id.IsValid()) {
        return;
    }

    Scene& scene = *preview.scene;
    DocId doc_id = preview.doc_id;

    NameComponent* name_component = scene.component<NameComponent>(id);
    if (!name_component) {
        return;
    }

    EditService& edit_service = editor_services_.edit();

    const DrawComponentCtx ctx{
        .app = m_editor.app(),
        .edit = edit_service,
        .thumbnail = editor_services_.thumbnail(),
        .scene = &scene,
        .entity = id,
        .doc_id = doc_id,
    };

    {
        FixedString<64> name = name_component->GetNameRef();
        if (ui::TextBox("Name", name.data(), name.capacity())) {
            auto cmd = std::make_unique<ChangePropertyCmd>(
                engine_services_.sceneRegistry(),
                id,
                NameComponent_Id,
                "name"_sid,
                name_component->GetNameRef(),
                name);
            edit_service.submit(doc_id, std::move(cmd));
        }
    }

    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (ImGui::Button("+")) {
        ImGui::OpenPopup("AddComponentPopup");
    }

    auto create_component = [&](BuiltinComponentId cid) {
        if (scene.storage().Has(id, cid)) {
            LOG_ERROR("object {} already has component {}",
                      name_component->GetName(),
                      std::to_underlying(cid));
            return;
        }
        auto cmd = std::make_unique<AddComponentCmd>(
            engine_services_.sceneRegistry(),
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
    LuaScriptComponent* lua_script = scene.component<LuaScriptComponent>(id);
    NativeScriptComponent* native_script = scene.component<NativeScriptComponent>(id);
    CameraComponent* camera = scene.component<CameraComponent>(id);
    PrefabInstanceComponent* prefab = scene.component<PrefabInstanceComponent>(id);
    FacingComponent* facing = scene.component<FacingComponent>(id);
    VelocityComponent* velocity = scene.component<VelocityComponent>(id);
    SpriteAnimatorComponent* sprite_animator = scene.component<SpriteAnimatorComponent>(id);

#define DRAW_COMPONENT_ARGS(DISPLAY) DISPLAY, ctx

    DrawComponent(DRAW_COMPONENT_ARGS("Transform"), transform, [&](TransformComponent& p_transform) {
        const math::Mat4f old_transform = p_transform.GetLocalMatrix();

        TransformComponent copy = p_transform;
        const bool dirty = DrawComponentAuto<TransformComponent>(&copy, ctx);
        if (dirty && camera) {
            camera->SetDirty();
        }
    });

    DrawComponent(DRAW_COMPONENT_ARGS("Light"), light, [&](LightComponent& p_light) {
        bool dirty = DrawComponentAuto<LightComponent>(&p_light, ctx);
        if (dirty) {
            p_light.SetDirty();
        }

        if (material) {
            DrawComponentAuto<MaterialComponent>(material, ctx);
        }
    });

    DrawComponent(DRAW_COMPONENT_ARGS("Lua Script"), lua_script, [&](LuaScriptComponent& script) {
        FixedString<32>& name = script.GetClassNameRef();
        ui::TextBox("class_name", name.data(), name.capacity(), false);

        DrawComponentAuto<LuaScriptComponent>(&script, ctx);
    });

    DrawComponent(DRAW_COMPONENT_ARGS("Native Script"), native_script, [&](NativeScriptComponent& script) {
        FixedString<32>& name = script.name;
        ui::TextBox("class_name", name.data(), name.capacity(), false);

        DrawComponentAuto<NativeScriptComponent>(&script, ctx);
    });

    DrawComponent(DRAW_COMPONENT_ARGS("Prefab"), prefab, [&](PrefabInstanceComponent& prefab) {
        const Guid old_guid = prefab.prefabGuid();
        const bool dirty = DrawComponentAuto<PrefabInstanceComponent>(&prefab, ctx);
        const Guid new_guid = prefab.prefabGuid();

        // @NOTE: can only instantiate once
        if (old_guid.IsNull() && !new_guid.IsNull()) {
            scene.instantiatePrefab(prefab, id);
        }

        if (dirty) {
            ecs::Entity child = prefab.child();
            TransformComponent* transform = scene.component<TransformComponent>(child);
            transform->SetTranslation(prefab.translation());
        }
    });

    DrawComponent(DRAW_COMPONENT_ARGS("Collider"), collider, [&](ColliderComponent& collider) {
        DrawComponentAuto(&collider, ctx);

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

    DrawComponent(DRAW_COMPONENT_ARGS("Facing"), facing, [&ctx](FacingComponent& comp) {
        DrawComponentAuto(&comp, ctx);
    });

    DrawComponent(DRAW_COMPONENT_ARGS("Velocity"), velocity, [&ctx](VelocityComponent& comp) {
        DrawComponentAuto(&comp, ctx);
    });

    DrawComponent(
        DRAW_COMPONENT_ARGS("SpriteAnimator"), sprite_animator,
        [&](SpriteAnimatorComponent& animator) {
            // @TODO: refactor this
            // @TODO: drop down
            const Guid& guid = animator.GetResourceGuid();
            if (auto handle = AssetRegistry::singleton().findByGuid<SpriteAnimationAsset>(guid);
                handle.is_some()) {
                // SpriteAnimationAsset* asset = handle.unwrap_unchecked().get();
                // std::string clip_name = animator.currentClip();
                // if (ui::TextBox("clip", clip_name)) {
                //    const SpriteAnimationClip* clip = asset->GetClip(clip_name);
                //    if (clip) {
                //        p_animator.SetClip(clip_name);
                //    }
                //}
            }

            DrawComponentAuto<SpriteAnimatorComponent>(&animator, ctx);
        });

    DrawComponent(
        DRAW_COMPONENT_ARGS("SkeletalAnimation"),
        scene.component<SkeletalAnimationComponent>(id),
        [&](SkeletalAnimationComponent& p_anim) {
            DrawComponentAuto<SkeletalAnimationComponent>(&p_anim, ctx);
            ImGui::Separator();
            const float start = p_anim.GetStart();
            const float end = p_anim.GetEnd();
            float timer = p_anim.GetTimer();

            if (ImGui::SliderFloat("Frame", &timer, start, end)) {
                p_anim.SetPlaying();
                p_anim.SetTimer(timer);
            }
        });

    DrawComponent(DRAW_COMPONENT_ARGS("SpriteRenderer"),
                  scene.component<SpriteRendererComponent>(id),
                  [&](SpriteRendererComponent& p_renderer) {
                      DrawComponentAuto<SpriteRendererComponent>(&p_renderer, ctx);
                  });

    DrawComponent(DRAW_COMPONENT_ARGS("TileMapRenderer"),
                  scene.component<TileMapInstanceComponent>(id),
                  [&](TileMapInstanceComponent& p_renderer) {
                      DrawComponentAuto<TileMapInstanceComponent>(&p_renderer, ctx);
                  });

    MeshRendererComponent* mesh_renderer = scene.component<MeshRendererComponent>(id);
    DrawComponent(DRAW_COMPONENT_ARGS("MeshRenderer"), mesh_renderer, [&](MeshRendererComponent& p_render) {
        DrawComponentAuto<MeshRendererComponent>(&p_render, ctx);

        if (ImGui::Button("+")) {
            // @TODO: command
            DEV_ASSERT(0 && "should go through cb");
#if 0
            auto& materials = p_render.GetMaterialInstances();
            auto name = std::format("mat_{}", materials.size());
            auto mat_id = EntityFactory::CreateNameEntity(scene, name);
            scene.Create<MaterialComponent>(mat_id);
            p_render.AddMaterial(mat_id);
#endif
        }

        for (ecs::Entity id : p_render.GetMaterialInstances()) {
            if (MaterialComponent* material = scene.component<MaterialComponent>(id); material) {
                DrawComponentCtx copy_ctx = ctx;
                copy_ctx.entity = id;
                DrawComponentAuto<MaterialComponent>(material, copy_ctx);
            }
        }
    });

    DrawComponent(DRAW_COMPONENT_ARGS("Camera"), camera, [&](CameraComponent& p_camera) {
        if (DrawComponentAuto<CameraComponent>(&p_camera, ctx)) {
            p_camera.SetDirty();
        }
    });

#if 0
    DrawComponent("ParticleEmitter", particle_emitter_component, [](ParticleEmitterComponent& p_emitter) {
        const float width = 100.0f;
        ImGui::Checkbox("Gravity", &p_emitter.gravity);
        DrawVec3Control("Velocity", p_emitter.startingVelocity, 0.0f, width);
        DrawDragInt("Max count", p_emitter.maxParticleCount, 1000.f, 1000, MAX_PARTICLE_COUNT, width);
        DrawDragInt("Emit per frame", p_emitter.particlesPerFrame, 10.0f, 1, 10000, width);
        DrawDragFloat("Scaling", p_emitter.particleScale, 0.01f, 0.01f, 10.0f, width);
        DrawDragFloat("Life span", p_emitter.particleLifeSpan, 0.1f, 0.1f, 10.0f, width);
        ImGui::Separator();
        DrawColorPicker3("base color", &p_emitter.color.x, width);
        DrawInputText("texture", p_emitter.texture, width);
    });

    DrawComponent("MeshEmitter", mesh_emitter_component, [](MeshEmitterComponent& p_emitter) {
        // const float width = 100.0f;
        if (ImGui::Button("reset")) {
            p_emitter.Reset();
        }

        DrawCheckBoxBitflag("run", p_emitter.flags, MeshEmitterComponent::RUNNING);
        DrawCheckBoxBitflag("recycle", p_emitter.flags, MeshEmitterComponent::RECYCLE);
    });
#endif
}

}  // namespace cave
