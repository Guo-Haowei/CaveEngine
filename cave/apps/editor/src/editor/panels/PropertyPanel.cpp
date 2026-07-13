#include "PropertyPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "cave/core/diagnostics/Profiler.h"

#include "editor/inspector/PropertyEditors.h"
#include "editor/EditorState.h"
#include "editor/utility/ContentEntry.h"
#include "editor/services/DragDropService.h"
#include "editor/services/SelectionService.h"
#include "editor/services/Workspace.h"

// @TODO: refactor
#include "engine/private/core/reflection/MetaEditor.h"
#include "engine/private/runtime/assets/SpriteAnimationAsset.h"
#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/ui/layout.h"

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
    COMPONENT_DECL(MeshRenderer)   \
    COMPONENT_DECL(SpriteRenderer) \
    COMPONENT_DECL(Facing)         \
    COMPONENT_DECL(TileMapInstance)

bool DrawPropertyAuto(const FieldMetaBase* property,
                      void* component,
                      const DrawComponentCtx& ctx) {
    switch (property->editor_hint) {
        case EditorHint::EnumDropDown:
            return property->DrawEditor(component, ui::kDefaultColumnWidth);
        case EditorHint::Toggle:
            return EditAndSubmit<bool>(
                ctx, component, property,
                [](const char* label, bool& value) {
                    return ui::CheckBox(label, value);
                });
        case EditorHint::InputInt:
            return EditAndSubmit<int>(
                ctx, component, property,
                [](const char* label, int& value) {
                    return ui::InputInt(label, value);
                });
        case EditorHint::InputFloat:
            return EditAndSubmit<float>(
                ctx, component, property,
                [](const char* label, float& value) {
                    return ui::InputFloat(label, value);
                });
        case EditorHint::BitMask: {
            return EditAndSubmit<uint32_t>(
                ctx, component, property,
                [](const char* label, uint32_t& value) {
                    return ui::DrawBitMask32(label, value);
                });
        } break;
        case EditorHint::DragInt:
            BreakIfDebug();
            return false;
        case EditorHint::DragFloat:
            return EditAndSubmit<float>(
                ctx, component, property,
                [&](const char* label, float& value) {
                    return ui::DragFloat(label,
                                         value,
                                         0.01f,
                                         property->v_min,
                                         property->v_max);
                });
        case EditorHint::Color:
            return EditAndSubmit<Vec4f>(
                ctx, component, property,
                [](const char* label, Vec4f& value) {
                    return ui::ColorPicker4(label, value);
                });
        case EditorHint::Translation:
            return EditAndSubmit<Vec3f>(
                ctx, component, property,
                [](const char* label, Vec3f& value) {
                    return ui::Float3(label, value, 0.0f);
                });
        case EditorHint::Scale:
            return EditAndSubmit<Vec3f>(
                ctx, component, property,
                [](const char* label, Vec3f& value) {
                    return ui::Float3(label, value, 1.0f);
                });
        case EditorHint::Rotation: {
            // @TODO: fix this
            Vec4f& q = property->template GetData<Vec4f>(component);
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
                ctx.entity,
                ctx.cid,
                property->id,
                old_v,
                new_v);
            ctx.editor_services.edit().submit(ctx.doc_id, std::move(cmd));
            return true;
        } break;
        case EditorHint::Asset: {
            return EditAndSubmit<Guid>(
                ctx, component, property,
                [&ctx](const char* label, Guid& guid) {
                    return DrawAsset(ctx, label, guid);
                });
        } break;
        case EditorHint::VariantMap: {
            return EditAndSubmit<VariantMap>(
                ctx, component, property,
                [](const char* label, VariantMap& map) {
                    return DrawVariantMap(label, map);
                });
        } break;
        default:
            return false;
    }
}

template<typename T>
bool DrawComponentAuto(T* component, const DrawComponentCtx& ctx) {
    const MetaTableFields& meta_table = MetaDataTable<T>::GetFields();
    DrawComponentCtx ctx2 = ctx;
    ctx2.cid = T::kId;

    int dirty = 0;
    for (const auto& field : meta_table) {
        dirty |= (int)DrawPropertyAuto(field, component, ctx2);
    }
    return (int)dirty;
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

    const DrawComponentCtx ctx{
        .engine_services = m_engine_services,
        .editor_services = m_editor_services,
        .scene = &scene,
        .entity = id,
        .doc_id = doc_id,
    };

    {
        FixedString<64> name = name_component->nameRef();
        if (ui::TextBox("Name", name)) {
            auto cmd = MakeOwner<ChangePropertyCmd>(
                m_engine_services.sceneRegistry(),
                id,
                NameComponent_Id,
                "name"_sid,
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

    auto create_component = [&](BuiltinComponentId cid) {
        if (scene.storage().has(cid, id)) {
            LOG_ERROR("object {} already has component {}",
                      name_component->name(),
                      std::to_underlying(cid));
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
    FacingComponent* facing = scene.component<FacingComponent>(id);
    VelocityComponent* velocity = scene.component<VelocityComponent>(id);
    MotorComponent* motor = scene.component<MotorComponent>(id);
    SpriteAnimatorComponent* sprite_animator = scene.component<SpriteAnimatorComponent>(id);

#define DRAW_COMPONENT_ARGS(DISPLAY) DISPLAY, ctx

    DrawComponent(DRAW_COMPONENT_ARGS("Transform"), transform, [&](TransformComponent& p_transform) {
        const math::Mat4f old_transform = p_transform.localMatrix();

        TransformComponent copy = p_transform;
        const bool dirty = DrawComponentAuto<TransformComponent>(&copy, ctx);
        if (dirty && camera) {
            camera->setDirty();
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

    DrawComponent(DRAW_COMPONENT_ARGS("Native Script"), native_script, [&](NativeScriptComponent& script) {
        // @TODO: fix this
        FixedString<32>& name = script.name;
        ui::TextBox("class_name", name);

        DrawComponentAuto<NativeScriptComponent>(&script, ctx);
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

    DrawComponent(DRAW_COMPONENT_ARGS("Motor"), motor, [&ctx](MotorComponent& comp) {
        DrawComponentAuto(&comp, ctx);
    });

    DrawComponent(
        DRAW_COMPONENT_ARGS("SpriteAnimator"), sprite_animator,
        [&](SpriteAnimatorComponent& animator) {
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
            scene.create<MaterialComponent>(mat_id);
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
            p_camera.setDirty();
        }
    });
}

}  // namespace cave
