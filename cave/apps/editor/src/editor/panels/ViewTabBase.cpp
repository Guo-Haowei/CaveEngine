#include "ViewTabBase.h"

#include "cave/runtime/framework/EngineServices.h"

#include "editor/services/EditorServices.h"
#include "editor/services/DocumentService.h"
#include "editor/services/SelectionService.h"
#include "editor/services/Workspace.h"
#include "editor/widgets/DragDrop.h"
#include "editor/play/PIESession.h"

// @TODO: remove
#include "editor/EditorState.h"
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/scene/SceneScheduler.h"
#include "engine/private/runtime/view/ViewManager.h"

#include "engine/private/renderer/gpu_resource.h"
#include "engine/private/renderer/sampler.h"

namespace cave {

using namespace ::cave::math;

namespace {

#if 1
constexpr uint32_t kTextureWidth = 1920;
constexpr uint32_t kTextureHeight = 1080;
#else
constexpr uint32_t kTextureWidth = 640;
constexpr uint32_t kTextureHeight = 480;
#endif

// @TODO: refactor
void FitAspect(float aspect, float& w, float& h) {
    if (aspect * h > w) {
        h = w / aspect;
    } else {
        w = h * aspect;
    }
}

}  // namespace

ViewTabBase::ViewTabBase(EditorState& editor,
                         DocId doc_id,
                         SceneId scene_id,
                         ViewDimension dim)
    : Tab(editor, doc_id)
    , m_editor(editor)
    , m_view_manager(m_engine_services.viewManager())
    , m_dim(dim)
    , m_preview_scene_id(scene_id) {

    // @TODO: move it to somewhere else
    GpuTextureDesc desc{
        .type = AttachmentType::COLOR_2D,
        .dimension = Dimension::TEXTURE_2D,
        .width = kTextureWidth,
        .height = kTextureHeight,
        .depth = 1,
        .mipLevels = 0,
        .arraySize = 1,
        .format = PixelFormat::R16G16B16A16_FLOAT,
        .bindFlags = BIND_RENDER_TARGET | BIND_SHADER_RESOURCE,
        .miscFlags = RESOURCE_MISC_NONE,
    };
    m_texture = m_engine_services.renderDevice().CreateTexture(
        desc,
        PointClampSampler());
}

void ViewTabBase::onCreate() {
    m_camera.setAspect((float)kTextureWidth / (float)kTextureHeight);
    m_camera.setDirty();
    switch (m_dim) {
        case ViewDimension::Dim2: {
            m_camera.setProjectionType(ProjectionType::Orthographic);
            m_camera_transform.translate(Vec3f(0, 0, 4));
            m_camera_controller = std::make_unique<CameraController2DEditor>(m_camera, m_camera_transform);
        } break;
        case ViewDimension::Dim3: {
            m_camera_transform.translate(Vec3f(0, 4, 8));
            m_camera_controller = std::make_unique<CameraControllerFPS>(m_camera, m_camera_transform);
        } break;
    }

    IDocument* doc = m_editor_services.document().resolve(m_doc_id);
    if (DEV_VERIFY(doc)) {
        const Guid guid = doc->guid();
        auto& tabs = m_editor_services.workspace().workspaceState().tabs;

        auto it = std::find_if(tabs.begin(), tabs.end(),
                               [&guid](const TabState& tab) {
                                   return tab.guid == guid;
                               });

        if (it != tabs.end()) {
            TabState& tab = *it;
            m_camera = tab.camera.unwrap_or(m_camera);
            m_camera_transform = tab.transform.unwrap_or(m_camera_transform);
        }
    }

    m_camera_transform.updateTransform();
    m_camera.update(m_camera_transform.worldMatrix());

    m_view_id = m_view_manager.createView(
        "SceneView",
        { 0, 0, kTextureWidth, kTextureHeight });

    m_engine_services.sceneScheduler().add(this);
}

void ViewTabBase::onDestroy() {
    m_view_manager.destroyView(m_view_id);

    m_engine_services.sceneScheduler().remove(this);
}

void ViewTabBase::collectSceneTicks(std::vector<SceneTickRequest>& out_requests) {
    if (!m_editor.isPlaying()) {
        out_requests.push_back(SceneTickRequest{
            SceneTickDomain::Editor,
            m_preview_scene_id,
            m_view_id,
            *this,
        });
    }
}

void ViewTabBase::submitView(bool support_pie) {
    using namespace render;
    ViewDesc view;
    view.view_id = m_view_id;
    view.viewport_px = { 0, 0, kTextureWidth, kTextureHeight };
    if (support_pie && m_editor.isPlaying()) {
        view.scene_id = m_editor.PIE().getPIESceneId();
        view.camera_source = CameraSource::FirstCamera();
    } else {
        view.scene_id = m_preview_scene_id;
        view.camera_source = CameraSource::External(m_camera);

        SelectionKey key = m_editor_services.selection().Primary(m_doc_id);
        if (key.scene == m_preview_scene_id && key.entity.valid()) {
            view.highlight.entities.insert(key.entity);
        }
    }
    view.output = m_texture;
    m_view_manager.submit(view);
}

void ViewTabBase::updateRect(math::FloatRect& out_rect) {
    ImVec2 cursor_pos = ImGui::GetCursorPos();  // cursor to screen pos
    ImVec2 cursor_screen_pos = ImGui::GetCursorScreenPos();
    ImVec2 size = ImGui::GetWindowSize();
    {
        size.x -= 2 * cursor_pos.x;
        size.y -= 1.2f * cursor_pos.y;

        const float aspect = m_camera.aspect();
        FitAspect(aspect, size.x, size.y);
    }

    out_rect = math::FloatRect::FromMinMax(
        cursor_screen_pos.x,
        cursor_screen_pos.y,
        cursor_screen_pos.x + size.x,
        cursor_screen_pos.y + size.y);
}

void ViewTabBase::drawMainView(const math::FloatRect& rect) {
    using rhi::Backend;

    const ImVec2 min{ rect.x, rect.y };
    const ImVec2 max{ rect.Right(), rect.Bottom() };

    // @TODO: move it somewhere else
    uint64_t tex = m_texture->GetHandle();
    // add image for drawing
    switch (m_editor.app().backend()) {
        case Backend::Direct3D11:
        case Backend::Direct3D12: {
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)tex, min, max);
        } break;
        case Backend::OpenGL: {
            // @TODO: add p_flip
            ImVec2 uv_min = ImVec2(0, 1);
            ImVec2 uv_max = ImVec2(1, 0);
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)tex, min, max, uv_min, uv_max);
        } break;
        case Backend::Vulkan:
        case Backend::Metal: {
        } break;
        default:
            CRASH_NOW();
            break;
    }

    ImGui::Dummy({ rect.w, rect.h });
    if (auto handle_opt = DragDropTarget(AssetType::All)) {
        auto handle = handle_opt.unwrap_unchecked();
        if (!onAssetDropped(std::move(handle))) {
            const AssetMetaData* meta = handle.meta();
            LOG_ERROR(LogChannel::Asset, "asset '{}' not accepted", meta->name);
        }
    }
}

void ViewTabBase::commitSceneReload() {
    DocId doc_id = docId();
    IDocument* doc = m_editor_services.document().resolve(doc_id);
    if (!doc) {
        return;
    }

    doc->reloadPreviewScene();
}

bool ViewTabBase::onAssetDropped(AssetHandle handle) {
    const AssetMetaData* meta = handle.meta();
    DEV_ASSERT(meta);
    if (meta->type == AssetType::Scene) {
        m_editor.services().document().openDoc({
            handle.guid(),
            meta->type,
            true,
        });
        return true;
    }
    return false;
}

bool ViewTabBase::tabState(TabState& out) const {
    if (!Tab::tabState(out)) {
        return false;
    }

    out.camera = Some(m_camera);
    out.transform = Some(m_camera_transform);
    return true;
}

}  // namespace cave
