#include "ViewerTab.h"

#include "engine/private/runtime/string/StringUtils.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/renderer/graphics_manager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/InputSystem.h"
#include "engine/private/runtime/scene/EntityFactory.h"

#include "editor/document/document.h"
#include "editor/EditorState.h"
#include "editor/panels/AssetInspector.h"
#include "editor/viewer/Viewer.h"

// @TODO: refactor
#include "engine/private/runtime/scene/ISceneManager.h"

namespace cave {

using ecs::Entity;

static const char EDITOR_CAMERA_NAME[] = "_editor_cam";

const Guid& ViewerTab::GetGuid() const {
    return GetDocument().GetGuid();
}

ViewerTab::ViewerTab(EditorState& p_editor, Viewer& p_viewer, Dimension p_dimension)
    : m_id(TabId::Next())
    , m_dimension(p_dimension)
    , m_editor(p_editor)
    , m_viewer(p_viewer)
    , m_scene_manager(*p_editor.GetApp().GetSceneManager()) {

    InputRouter& router = m_editor.GetApp().GetInputSystem()->Router();
    router.Register(this);
}

ViewerTab::~ViewerTab() {
    InputRouter& router = m_editor.GetApp().GetInputSystem()->Router();
    router.Unregister(this);
}

void ViewerTab::SetSelectedEntity(ecs::Entity p_selected) {
    m_selected = p_selected;
    if (Scene* scene = GetResolvedScene(); scene) {
        scene->m_selected = m_selected;
    }
}

Scene* ViewerTab::GetResolvedScene() {
    return m_scene_manager.Resolve(GetSceneId());
}

void ViewerTab::SetCopiedEntity(ecs::Entity p_copied) {
    m_copied = p_copied;
}

void ViewerTab::OnCreate(const Guid& p_guid) {
    auto handle = AssetRegistry::GetSingleton().FindByGuid(p_guid).unwrap();
    auto meta = handle.GetMeta();
    DEV_ASSERT(meta);

    m_title = std::format("{}###{}", meta->name, handle.GetGuid().ToString());

    LOG_OK("ViewerTab '{}' created", m_title);

    OnCreateInternal(p_guid);

    switch (m_dimension) {
        case DIMENSION_2: {
            SetupDefault2DCamera();
        } break;
        case DIMENSION_3: {
            SetupDefault3DCamera();
        } break;
    }
}

void ViewerTab::SetupDefault2DCamera() {
    Scene* scene = GetResolvedScene();
    DEV_ASSERT(scene);

    Entity cam = scene->FindEntityByName(EDITOR_CAMERA_NAME);
    if (!cam.IsValid()) {
        cam = EntityFactory::CreateCameraEntity(*scene, EDITOR_CAMERA_NAME);
        scene->Create<NoSaveTag>(cam);
        scene->AttachChild(cam);
        CameraComponent* camera = scene->GetComponent<CameraComponent>(cam);
        camera->SetProjection(ProjectionType::Orthographic);
        TransformComponent* transform = scene->GetComponent<TransformComponent>(cam);
        transform->SetTranslation(Vector3f(0, 0, 10));
    }

    m_camera = cam;
    m_camera_controller = std::make_shared<CameraController2DEditor>(scene, cam);
}

void ViewerTab::SetupDefault3DCamera() {
    Scene* scene = GetResolvedScene();
    DEV_ASSERT(scene);

    Entity cam = scene->FindEntityByName(EDITOR_CAMERA_NAME);
    Entity cam_y = scene->FindEntityByName("_editor_cam_y");
    Entity cam_root = scene->FindEntityByName("_editor_cam_root");

    if (!cam.IsValid()) {
        cam = EntityFactory::CreateCameraEntity(*scene, EDITOR_CAMERA_NAME);
        cam_y = EntityFactory::CreateTransformEntity(*scene, "_editor_cam_y");
        cam_root = EntityFactory::CreateTransformEntity(*scene, "_editor_cam_root");

        scene->Create<NoSaveTag>(cam);
        scene->Create<NoSaveTag>(cam_y);
        scene->Create<NoSaveTag>(cam_root);

        scene->AttachChild(cam_root);
        scene->AttachChild(cam_y, cam_root);
        scene->AttachChild(cam, cam_y);
    }

    m_camera = cam;
    m_camera_controller = std::make_shared<CameraControllerFPS>(scene, cam_root, cam_y, cam);
}

void ViewerTab::OnActivate() {
    m_active = true;
    OnActivateInternal();
}

void ViewerTab::OnDeactivate() {
    OnDeactivateInternal();
    m_active = false;
}

void ViewerTab::DrawAssetInspector() {
    m_editor.GetAssetInspector().DrawContentBrowser();
}

void ViewerTab::DrawMainView(const CameraComponent&) {
    const auto canvas_min = m_viewer.GetCanvasMin();
    const auto canvas_max = canvas_min + m_viewer.GetCanvasSize();

    ImVec2 top_left(canvas_min.x, canvas_min.y);
    ImVec2 bottom_right(canvas_max.x, canvas_max.y);

    // @TODO: fix this
    const auto& gm = *m_editor.GetApp().GetGraphicsManager();
    uint64_t handle = gm.GetFinalImage();
    // add image for drawing
    switch (gm.GetBackend()) {
        case Backend::D3D11:
        case Backend::D3D12: {
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)handle, top_left, bottom_right);
        } break;
        case Backend::OPENGL: {
            ImVec2 uv_min = ImVec2(0, 1);
            ImVec2 uv_max = ImVec2(1, 0);
            if (gm.GetActiveRenderGraphName() == RenderGraphName::PATHTRACER) {
                uv_min = ImVec2(0, 0);
                uv_max = ImVec2(1, 1);
            }
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)handle, top_left, bottom_right, uv_min, uv_max);
        } break;
        case Backend::VULKAN:
        case Backend::METAL: {
        } break;
        default:
            CRASH_NOW();
            break;
    }
}

CameraInputState ViewerTab::CreateCameraInputState2D(const std::vector<InputEvent>& p_events, const KeyState&) {
    CameraInputState state{};

    float dx = 0.0f;
    float dy = 0.0f;
    bool mmb = false;

    for (const InputEvent& e : p_events) {
        if (e.consumed) {
            continue;
        }

        switch (e.type) {
            case InputEventType::MouseWheel: {
                e.consumed = true;
                state.zoom_delta = -e.dy;
            } break;
            case InputEventType::MouseMove: {
                e.consumed = true;
                dx = -e.dx;
                dy = e.dy;
            } break;
            case InputEventType::ButtonDown:
                if (e.code == std::to_underlying(Key::MMB)) {
                    e.consumed = true;
                    mmb = true;
                }
                break;
            default:
                break;
        }

        if (mmb) {
            state.move = Vector3f(dx, dy, 0.0f);
        }
    }

    return state;
}

CameraInputState ViewerTab::CreateCameraInputState3D(const std::vector<InputEvent>& p_events, const KeyState& p_st) {
    Vector2f rotation = Vector2f::Zero;

    const InputDeviceId id{ 0 };
    const bool mmb = p_st.Down(id, Key::MMB);
    const int dx = p_st.Down(id, Key::D) - p_st.Down(id, Key::A);
    const int dy = p_st.Down(id, Key::E) - p_st.Down(id, Key::Q);
    const int dz = p_st.Down(id, Key::W) - p_st.Down(id, Key::S);

    CameraInputState state{};

    for (const InputEvent& e : p_events) {
        if (e.consumed) {
            continue;
        }
        switch (e.type) {
            case InputEventType::MouseWheel: {
                e.consumed = true;
                state.zoom_delta = 3.0f * e.dy;
            } break;
            case InputEventType::MouseMove: {
                if (mmb) {
                    e.consumed = true;
                    state.rotation.x = e.dx;
                    state.rotation.y = e.dy;
                }
            } break;
            default:
                break;
        }
    }

    state.move = Vector3f(dx, dy, dz);
    return state;
}

void ViewerTab::Update(float p_timestep) {
    m_camera_state.move *= p_timestep;
    m_camera_state.zoom_delta *= p_timestep;
    m_camera_state.rotation *= p_timestep;

    m_camera_controller->Update(m_camera_state);
}

void ViewerTab::OnEvents(const std::vector<InputEvent>& p_events) {
    if (!m_viewer.IsHovered()) {
        return;
    }

    if (m_editor.IsPlaying()) {
        return;
    }

    const KeyState& st = m_editor.GetApp().GetInputSystem()->GetKeyState();
    if (st.AnyAltDown() || st.AnyCtrlDown() || st.AnyShiftDown()) {
        m_camera_state = {};
        return;
    }

    switch (m_dimension) {
        case DIMENSION_2: {
            m_camera_state = CreateCameraInputState2D(p_events, st);
        } break;
        case DIMENSION_3: {
            m_camera_state = CreateCameraInputState3D(p_events, st);
        } break;
        default: {
            CRASH_NOW_MSG("invalid dimension");
        } break;
    }
}

void ViewerTab::BuildViewsImpl(Scene* p_scene,
                               ecs::Entity p_camera,
                               std::vector<SceneView>& p_out_views,
                               bool p_is_opengl) {
    if (m_editor.IsPlaying()) {
        DEV_ASSERT(0);
        std::shared_ptr<Scene> scene = nullptr;

        // @HACK: find the first non-editor camera
        for (auto [id, camera] : scene->View<CameraComponent>()) {
            if (scene->Contains<NoSaveTag>(id)) {
                continue;
            }

            SceneView scene_view;
            scene_view.scene = scene.get();

            ViewInfo::FromCamera(camera,
                                 scene_view.view_info,
                                 p_is_opengl);

            p_out_views.push_back(scene_view);
            break;
        }
        return;
    }

    const CameraComponent* cam = p_scene->GetComponent<CameraComponent>(p_camera);

    if (DEV_VERIFY(cam)) {
        SceneView scene_view;
        scene_view.scene = p_scene;

        ViewInfo::FromCamera(*cam,
                             scene_view.view_info,
                             p_is_opengl);

        p_out_views.push_back(scene_view);
    }
}

void ViewerTab::BuildViews(std::vector<SceneView>& p_out_views, bool p_is_opengl) {
    if (!m_active) {
        return;
    }

    BuildViewsImpl(GetResolvedScene(), m_camera, p_out_views, p_is_opengl);
}

}  // namespace cave
