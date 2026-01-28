#include "ViewerTab.h"

#include "engine/private/runtime/string/StringUtils.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/renderer/graphics_manager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/InputSystem.h"
#include "engine/private/runtime/framework/RuntimeHost.h"
#include "engine/private/runtime/scene/EntityFactory.h"

#include "editor/EditorState.h"
#include "editor/panels/AssetInspector.h"
#include "editor/viewer/Viewer.h"

// @TODO: refactor
#include "engine/private/runtime/scene/ISceneRegistry.h"

namespace cave {

#if 0
using ecs::Entity;

ViewerTab::ViewerTab(EditorState& p_editor,
                     DocId p_doc_id,
                     Viewer& p_viewer,
                     Dimension p_dimension)
    : m_id(ViewerTabId::Next())
    , m_doc_id(p_doc_id)
    , m_dimension(p_dimension)
    , m_editor(p_editor)
    , m_viewer(p_viewer)
    , m_scene_manager(*p_editor.GetApp().GetSceneRegistry()) {
}

ViewerTab::~ViewerTab() {
}

Scene* ViewerTab::GetResolvedScene() {
    return m_scene_manager.Resolve(GetSceneId());
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

    IApplication& app = m_editor.GetApp();

    app.GetInputSystem()->Router().Register(this);
    app.GetSceneScheduler().Register(this);
}

void ViewerTab::OnDeactivate() {
    IApplication& app = m_editor.GetApp();

    app.GetSceneScheduler().Unregister(this);
    app.GetInputSystem()->Router().Unregister(this);

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

void ViewerTab::BuildViewsImpl(SceneId p_scene_id,
                               ecs::Entity p_camera,
                               std::vector<SceneView>& p_out_views,
                               bool p_is_opengl) {
    // @TODO: refactor scene view API
    if (m_editor.IsPlaying()) {
        SceneView scene_view;
        scene_view.scene_id = m_editor.GetRuntimeHost().GetSceneId();
        scene_view.scene_manager = m_editor.GetApp().GetSceneRegistry();

        Scene* scene = scene_view.ResolveScene();

        // @HACK: find the first non-editor camera
        for (auto [id, camera] : scene->View<CameraComponent>()) {
            if (scene->Contains<NoSaveTag>(id)) {
                continue;
            }

            ViewInfo::FromCamera(camera,
                                 scene_view.view_info,
                                 p_is_opengl);

            p_out_views.push_back(scene_view);
            break;
        }
        return;
    }

    // @HACK: force update
    SceneView scene_view;
    scene_view.scene_id = p_scene_id;
    scene_view.scene_manager = m_editor.GetApp().GetSceneRegistry();

    Scene* scene = scene_view.ResolveScene();
    const CameraComponent* cam = scene->GetComponent<CameraComponent>(p_camera);

    if (DEV_VERIFY(cam)) {
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

    BuildViewsImpl(GetSceneId(), m_camera, p_out_views, p_is_opengl);
}
#endif

}  // namespace cave
