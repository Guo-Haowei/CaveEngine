#include "property_panel.h"

#include <glm/gtc/quaternion.hpp>
#include <ImGuizmo/ImGuizmo.h>
#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "engine/assets/sprite_animation_asset.h"
#include "engine/debugger/profiler.h"
#include "engine/core/string/string_utils.h"
#include "engine/reflection/meta_editor.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/runtime/asset_registry.h"

#include "editor/editor_command.h"
#include "editor/editor_layer.h"
#include "editor/scene_editor/scene_document.h"
#include "editor/utility/content_entry.h"
#include "editor/viewer/viewer.h"
#include "editor/viewer/viewer_tab.h"
#include "editor/widgets/drag_drop.h"
#include "editor/widgets/widget.h"

namespace cave {

template<ComponentType T, typename UIFunction>
static void DrawComponent(const std::string& p_name,
                          Scene* p_scene,
                          ecs::Entity p_entity,
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

        bool should_remove_component = false;
        if (ImGui::BeginPopup("ComponentSettings")) {
            if (ImGui::MenuItem("remove component")) {
                should_remove_component = true;
            }

            ImGui::EndPopup();
        }

        if (open) {
            p_function(*p_component);
            ImGui::TreePop();
        }

        if (should_remove_component) {
            p_scene->Get<T>().Remove(p_entity);
        }
    }
}

template<typename T>
concept HasSetResourceGuid = requires(T& t, const Guid& guid) {
    { t.SetResourceGuid(guid) } -> std::same_as<bool>;
};

static_assert(HasSetResourceGuid<LuaScriptComponent>);

template<typename T>
bool DrawAsset(const char* p_name, const Guid& p_guid, T* p_component) {
    auto handle_ = AssetRegistry::GetSingleton().FindByGuid(p_guid);

    AssetType type = AssetType::All;
    const AssetMetaData* meta = nullptr;
    const IAsset* asset = nullptr;

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, DEFAULT_COLUMN_WIDTH);
    ImGui::Text(ICON_FA_CUBE "  %s", p_name);
    ImGui::NextColumn();

    if (handle_.is_some()) {
        AssetHandle handle = handle_.unwrap_unchecked();
        meta = handle.GetMeta();
        DEV_ASSERT(meta);
        asset = handle.Get();
        type = meta->type;
    }

    ImGui::Text(" %s ", meta ? meta->name.c_str() : "not set");

    bool dirty = false;

    const bool hovered = ImGui::IsItemHovered();
    if (auto _handle = DragDropTarget(type); _handle.is_some()) {
        if constexpr (HasSetResourceGuid<T>) {
            if (p_component) {
                dirty = p_component->SetResourceGuid(_handle.unwrap_unchecked().GetGuid());
            }
        }
    }

    ImGui::Columns(1);
    if (hovered && meta) {
        ShowAssetToolTip(*meta, asset);
    }
    return dirty;
};

static bool DrawCheckBox(const char* p_name,
                         bool& p_val,
                         float p_column_width = DEFAULT_COLUMN_WIDTH) {
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, p_column_width);
    ImGui::Text("%s", p_name);
    ImGui::NextColumn();

    auto string_id = std::format("##{}", p_name);
    const bool dirty = ImGui::Checkbox(string_id.c_str(), &p_val);

    ImGui::Columns(1);
    return dirty;
}

template<typename T>
bool DrawComponentAuto(T* p_component) {
    const auto& meta_table = MetaDataTable<T>::GetFields();

    int dirty = 0;
    for (const auto& field : meta_table) {
        switch (field->editor_hint) {
            case EditorHint::EnumDropDown: {
                dirty |= (int)field->DrawEditor(p_component, DEFAULT_COLUMN_WIDTH);
            } break;
            case EditorHint::Toggle: {
                bool& toggle = field->template GetData<bool>(p_component);
                dirty |= (int)DrawCheckBox(field->name, toggle);
            } break;
            case EditorHint::Color: {
                Vector4f& color = field->template GetData<Vector4f>(p_component);
                dirty |= (int)DrawColorPicker4(field->name, &color.r);
            } break;
            case EditorHint::Asset: {
                const Guid& guid = field->template GetData<Guid>(p_component);
                dirty |= (int)DrawAsset(field->name, guid, p_component);
            } break;
            case EditorHint::Translation: {
                Vector3f& translation = field->template GetData<Vector3f>(p_component);
                dirty |= (int)DrawVec3Control(field->name, translation, 0.0f);
            } break;
            case EditorHint::Scale: {
                Vector3f& scale = field->template GetData<Vector3f>(p_component);
                dirty |= (int)DrawVec3Control(field->name, scale, 1.0f);
            } break;
            case EditorHint::Rotation: {
                Vector4f& q = field->template GetData<Vector4f>(p_component);
                glm::vec3 euler_ = glm::eulerAngles(glm::quat(q.w, q.x, q.y, q.z));
                Vector3f euler = *reinterpret_cast<Vector3f*>(&euler_);
                constexpr float RAD_TO_DEG = 180.0f / glm::pi<float>();
                constexpr float DEG_TO_RAD = glm::pi<float>() / 180.0f;
                euler *= RAD_TO_DEG;

                if (DrawVec3Control(field->name,
                                    euler,
                                    0.0f)) {
                    euler *= DEG_TO_RAD;
                    glm::quat q2 = glm::quat(reinterpret_cast<glm::vec3&>(euler));
                    q = Vector4f(q2.x, q2.y, q2.z, q2.w);
                    dirty |= 1;
                }
            } break;
            case EditorHint::DragFloat: {
                float& f = field->template GetData<float>(p_component);
                dirty |= (int)DrawDragFloat(field->name,
                                            f,
                                            0.1f,          // speed
                                            field->v_min,  // min
                                            field->v_max   // max
                );
            } break;
            default:
                break;
        }
    }
    return (bool)dirty;
}

void PropertyPanel::UpdateInternal() {
    CAVE_PROFILE_EVENT();

    ViewerTab* tab = m_editor.GetViewer().GetActiveTab();
    Scene* _scene = tab ? tab->GetScene() : nullptr;

    if (!_scene) {
        return;
    }

    ecs::Entity id = tab->GetSelectedEntity();

    if (!id.IsValid()) {
        return;
    }

    Scene& scene = *_scene;

    NameComponent* name_component = scene.GetComponent<NameComponent>(id);
    // @NOTE: when loading another scene, the selected entity will expire, thus don't have name
    if (!name_component) {
        // LOG_WARN("Entity {} does not have name", id.get_id());
        return;
    }

    DrawInputText("Name", name_component->GetNameRef());

    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (ImGui::Button("+")) {
        ImGui::OpenPopup("AddComponentPopup");
    }

    if (ImGui::BeginPopup("AddComponentPopup")) {
        if (ImGui::MenuItem("Rigid Body")) {
            LOG_ERROR("TODO: implement add component");
            ImGui::CloseCurrentPopup();
        }
#define COMPONENT_DECL(NAME)                                   \
    if (ImGui::MenuItem(#NAME)) {                              \
        m_editor.CommandAddComponent(ComponentName::NAME, id); \
    }
        COMPONENT_LIST
#undef COMPONENT_DECL

        ImGui::EndPopup();
    }

    // @TODO: see how much this can be done with meta table

    MeshRendererComponent* mesh_renderer = scene.GetComponent<MeshRendererComponent>(id);
    SpriteRendererComponent* sprite_renderer = scene.GetComponent<SpriteRendererComponent>(id);
    TileMapRendererComponent* tile_map_renderer = scene.GetComponent<TileMapRendererComponent>(id);
    SpriteAnimatorComponent* animator = scene.GetComponent<SpriteAnimatorComponent>(id);

    TransformComponent* transform = scene.GetComponent<TransformComponent>(id);
    LightComponent* light = scene.GetComponent<LightComponent>(id);
    MaterialComponent* material = scene.GetComponent<MaterialComponent>(id);
    ColliderComponent* collider = scene.GetComponent<ColliderComponent>(id);
    LuaScriptComponent* lua_script = scene.GetComponent<LuaScriptComponent>(id);
    CameraComponent* camera = scene.GetComponent<CameraComponent>(id);
    PrefabInstanceComponent* prefab = scene.GetComponent<PrefabInstanceComponent>(id);

    SkeletalAnimationComponent* animation_component = scene.GetComponent<SkeletalAnimationComponent>(id);
    RigidBodyComponent* rigid_body_component = scene.GetComponent<RigidBodyComponent>(id);

#if 0
    ParticleEmitterComponent* particle_emitter_component = scene.GetComponent<ParticleEmitterComponent>(id);
    MeshEmitterComponent* mesh_emitter_component = scene.GetComponent<MeshEmitterComponent>(id);
#endif

    SceneDocument& document = static_cast<SceneDocument&>(tab->GetDocument());
    const bool is_2d = m_editor.GetApplication()->IsWorld2D();

    // @TODO: limit this in scene editor
#define DRAW_COMPONENT_ARGS(DISPLAY) DISPLAY, _scene, id
    DrawComponent(DRAW_COMPONENT_ARGS("Transform"), transform, [&](TransformComponent& p_transform) {
        const Matrix4x4f old_transform = p_transform.GetLocalMatrix();
        const bool dirty = DrawComponentAuto<TransformComponent>(&p_transform);
        if (dirty) {
            Matrix4x4f new_transform = p_transform.GetLocalMatrix();
            // already moved, no need to move again
            document.RequestMove(id, old_transform, new_transform, false);
        }
    });

    DrawComponent(DRAW_COMPONENT_ARGS("Light"), light, [&](LightComponent& p_light) {
        bool dirty = DrawComponentAuto<LightComponent>(&p_light);
        if (dirty) {
            p_light.SetDirty();
        }

        if (material) {
            DrawComponentAuto<MaterialComponent>(material);
        }
    });

    DrawComponent(DRAW_COMPONENT_ARGS("Script"), lua_script, [](LuaScriptComponent& p_script) {
        DrawInputText("class_name", p_script.GetClassNameRef(), DEFAULT_COLUMN_WIDTH);

        DrawComponentAuto<LuaScriptComponent>(&p_script);
    });

    DrawComponent(DRAW_COMPONENT_ARGS("Prefab"), prefab, [&](PrefabInstanceComponent&) {
        const bool was_null = prefab->GetResourceGuid().IsNull();
        const bool dirty = DrawComponentAuto<PrefabInstanceComponent>(prefab);
        if (dirty) {
            // don't support remove instantiated entities yet
            DEV_ASSERT(was_null);
            scene.InstantiatePrefab(*prefab, id);
        }
    });

    DrawComponent(DRAW_COMPONENT_ARGS("Collider"), collider, [&](ColliderComponent& p_collider) {
        DrawComponentAuto<ColliderComponent>(&p_collider);

        Shape& shape = p_collider.GetShape();
        DrawEnumDropDown("shape", shape.type, DEFAULT_COLUMN_WIDTH);
        switch (shape.type) {
            case ShapeType::Round: {
                DrawVec1Control("radius", shape.data.radius, 0.5f);
            } break;
            case ShapeType::Box: {
                if (is_2d) {
                    DrawVec2Control("half", reinterpret_cast<Vector2f&>(shape.data.half), 0.5f);
                } else {
                    DrawVec3Control("half", shape.data.half, 0.5f);
                }
            } break;
            default:
                DrawVec3Control("placeholder", shape.data.half, 0.5f);
                break;
        }
    });

    DrawComponent(DRAW_COMPONENT_ARGS("Animator"), animator, [](SpriteAnimatorComponent& p_animator) {
        // @TODO: refactor this
        // @TODO: drop down
        const Guid& guid = p_animator.GetResourceGuid();
        if (auto handle = AssetRegistry::GetSingleton().FindByGuid<SpriteAnimationAsset>(guid);
            handle.is_some()) {
            SpriteAnimationAsset* asset = handle.unwrap_unchecked().Get();
            std::string clip_name = p_animator.GetCurrentClip();
            if (DrawInputText("clip", clip_name, DEFAULT_COLUMN_WIDTH)) {
                const SpriteAnimationClip* clip = asset->GetClip(clip_name);
                if (clip) {
                    p_animator.SetClip(clip_name);
                }
            }
        }

        DrawComponentAuto<SpriteAnimatorComponent>(&p_animator);
    });

    DrawComponent(DRAW_COMPONENT_ARGS("SpriteRenderer"), sprite_renderer, [](SpriteRendererComponent& p_sprite_renderer) {
        DrawComponentAuto<SpriteRendererComponent>(&p_sprite_renderer);
    });

    DrawComponent(DRAW_COMPONENT_ARGS("TileMapRenderer"), tile_map_renderer, [](TileMapRendererComponent& p_tile_map_renderer) {
        DrawComponentAuto<TileMapRendererComponent>(&p_tile_map_renderer);
    });

    DrawComponent(DRAW_COMPONENT_ARGS("MeshRenderer"), mesh_renderer, [&](MeshRendererComponent& p_mesh_renderer) {
        DrawComponentAuto<MeshRendererComponent>(&p_mesh_renderer);

        for (ecs::Entity id : p_mesh_renderer.GetMaterialInstances()) {
            if (MaterialComponent* material = scene.GetComponent<MaterialComponent>(id); material) {
                DrawComponentAuto<MaterialComponent>(material);
            }
        }
    });

    DrawComponent(DRAW_COMPONENT_ARGS("Camera"), camera, [&](CameraComponent& p_camera) {
        // @TODO: need a better way to do this
        bool is_ortho = p_camera.HasOrthoFlag();
        if (ToggleButton("ortho", is_ortho)) {
            p_camera.SetOrthoFlag(is_ortho);
            p_camera.SetDirtyFlag();
        }

        DrawComponentAuto<CameraComponent>(&p_camera);
    });

    DrawComponent(DRAW_COMPONENT_ARGS("RigidBody"), rigid_body_component, [](RigidBodyComponent& p_rigid_body) {
        const auto& size = p_rigid_body.size;
        switch (p_rigid_body.shape) {
            case RigidBodyComponent::SHAPE_CUBE: {
                ImGui::Text("shape: box");
                ImGui::Text("half size: %.2f, %.2f, %.2f", size.x, size.y, size.z);
            } break;
            case RigidBodyComponent::SHAPE_SPHERE: {
                ImGui::Text("shape: sphere");
                ImGui::Text("radius: %.2f", size.x);
            } break;
            default:
                break;
        }
    });

    // @TODO: refactor this
    DrawComponent(DRAW_COMPONENT_ARGS("Animation"), animation_component, [](SkeletalAnimationComponent& p_animation) {
        DrawComponentAuto<SkeletalAnimationComponent>(&p_animation);

        // if (!p_animation.IsPlaying()) {
        //     if (ImGui::Button("play")) {
        //         p_animation.flags |= SkeletalAnimationComponent::PLAYING;
        //     }
        // } else {
        //     if (ImGui::Button("stop")) {
        //         p_animation.flags &= ~SkeletalAnimationComponent::PLAYING;
        //     }
        // }
        if (ImGui::SliderFloat("Frame", &p_animation.timer, p_animation.start, p_animation.end)) {
            p_animation.SetPlaying();
        }
        ImGui::Separator();
    });

#if 0
    VoxelGiComponent* voxel_gi_component = scene.GetComponent<VoxelGiComponent>(id);
    EnvironmentComponent* environment_component = scene.GetComponent<EnvironmentComponent>(id);
    DrawComponent("Environment", environment_component, [](EnvironmentComponent& p_environment) {
        DrawInputText("texture", p_environment.sky.texturePath);
        ImGui::BeginDisabled(p_environment.sky.texturePath.empty());
        ImGui::EndDisabled();
        DrawColorPicker3("ambient", &p_environment.ambient.color.x);
    });

    DrawComponent("VoxelGi", voxel_gi_component, [](VoxelGiComponent& p_voxel_gi) {
        DrawCheckBoxBitflag("enabled", p_voxel_gi.flags, VoxelGiComponent::ENABLED);
        DrawCheckBoxBitflag("show_debug_box", p_voxel_gi.flags, VoxelGiComponent::SHOW_DEBUG_BOX);

        ImGui::Checkbox("debug", (bool*)(DVAR_GET_POINTER(gfx_debug_vxgi)));
        int value = DVAR_GET_INT(gfx_debug_vxgi_voxel);
        ImGui::RadioButton("lighting", &value, 0);
        ImGui::SameLine();
        ImGui::RadioButton("normal", &value, 1);
        DVAR_SET_INT(gfx_debug_vxgi_voxel, value);
    });

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
