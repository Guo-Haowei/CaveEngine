#include "ViewTabBase.h"

#include "engine/private/runtime/view/ViewManager.h"

#include "editor/services/SelectionService.h"

// @TODO: remove
#include "engine/private/renderer/sampler.h"
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/renderer/gpu_resource.h"

namespace cave {

using namespace ::cave::math;

#if 1
static constexpr uint32_t kTextureWidth = 1920;
static constexpr uint32_t kTextureHeight = 1080;
#else
static constexpr uint32_t kTextureWidth = 640;
static constexpr uint32_t kTextureHeight = 480;
#endif

ViewTabBase::ViewTabBase(EditorState& editor,
                         DocId doc_id,
                         SceneId scene_id,
                         ViewDimension dim)
    : Tab(editor, doc_id)
    , view_manager_(app_services_.viewManager())
    , dim_(dim)
    , preview_scene_id_(scene_id) {

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
    texture_ = m_editor.app().GetRenderDevice()->CreateTexture(
        desc,
        PointClampSampler());
}

void ViewTabBase::onCreate() {
    camera_.SetAspect((float)kTextureWidth / (float)kTextureHeight);
    camera_.SetDirty();
    switch (dim_) {
        case ViewDimension::Dim2: {
            camera_.SetProjection(ProjectionType::Orthographic);
            camera_transform_.Translate(Vector3f(0, 0, 4));
            camera_controller_ = std::make_unique<CameraController2DEditor>(camera_, camera_transform_);
        } break;
        case ViewDimension::Dim3: {
            camera_transform_.Translate(Vector3f(0, 4, 8));
            camera_controller_ = std::make_unique<CameraControllerFPS>(camera_, camera_transform_);
        } break;
    }

    camera_transform_.UpdateTransform();
    camera_.Update(camera_transform_.GetWorldMatrix());

    view_id_ = view_manager_.createView(
        "SceneView",
        { 0, 0, kTextureWidth, kTextureHeight });

    app_services_.sceneScheduler().add(this);
}

void ViewTabBase::onDestroy() {
    view_manager_.destroyView(view_id_);

    app_services_.sceneScheduler().remove(this);
}

void ViewTabBase::collectSceneTicks(std::vector<SceneTickRequest>& out_requests) {
    if (!m_editor.IsPlaying()) {
        out_requests.push_back(SceneTickRequest{
            SceneTickMode::Editor,
            preview_scene_id_,
        });
    }
}

void ViewTabBase::submitView(bool support_pie) {
    using namespace render;
    ViewDesc view;
    view.view_id = view_id_;
    view.viewport_px = { 0, 0, kTextureWidth, kTextureHeight };
    if (support_pie && m_editor.IsPlaying()) {
        view.scene_id = m_editor.PIE().getPIESceneId();
        view.camera_source = CameraSource::FirstCamera();
    } else {
        view.scene_id = preview_scene_id_;
        view.camera_source = CameraSource::External(camera_);

        SelectionKey key = editor_services_.selection().Primary(doc_id_);
        if (key.scene == preview_scene_id_ && key.entity.IsValid()) {
            view.highlight.entities.insert(key.entity);
        }
    }
    view.output = texture_;
    view_manager_.submit(view);
}

// @TODO: refactor
static void fitAspect(float aspect, float& w, float& h) {
    if (aspect * h > w) {
        h = w / aspect;
    } else {
        w = h * aspect;
    }
}

void ViewTabBase::updateRect(math::FloatRect& out_rect) {
    ImVec2 cursor_pos = ImGui::GetCursorPos();  // cursor to screen pos
    ImVec2 cursor_screen_pos = ImGui::GetCursorScreenPos();
    ImVec2 size = ImGui::GetWindowSize();
    {
        size.x -= 2 * cursor_pos.x;
        size.y -= 1.2f * cursor_pos.y;

        const float aspect = camera_.GetAspect();
        fitAspect(aspect, size.x, size.y);
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
    uint64_t handle = texture_->GetHandle();
    // add image for drawing
    switch (m_editor.app().GetBackend()) {
        case Backend::Direct3D11:
        case Backend::Direct3D12: {
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)handle, min, max);
        } break;
        case Backend::OpenGL: {
            // @TODO: add p_flip
            ImVec2 uv_min = ImVec2(0, 1);
            ImVec2 uv_max = ImVec2(1, 0);
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)handle, min, max, uv_min, uv_max);
        } break;
        case Backend::Vulkan:
        case Backend::Metal: {
        } break;
        default:
            CRASH_NOW();
            break;
    }

    // @TODO: drop target
    ImGui::Dummy({ rect.w, rect.h });
    // ImGui::InvisibleButton("###DropTarget", size);
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CAVE/Asset")) {
        }
        ImGui::EndDragDropTarget();
    }
}

}  // namespace cave
