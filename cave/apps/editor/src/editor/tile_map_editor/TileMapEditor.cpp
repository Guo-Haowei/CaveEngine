#include "TileMapEditor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/core/diagnostics/DebugIdAllocator.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/assets/TileSetAsset.h"

#include "editor/EditorState.h"
#include "editor/widgets/DragDrop.h"
#include "editor/widgets/Image.h"
#include "engine/private/ui/inputs.h"
#include "engine/private/ui/layout.h"
#include "editor/utility/ImGuizmo.h"

// @TODO: remove
#include "engine/private/renderer/sampler.h"
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/view/ViewManager.h"
#include "editor/services/PickingService.h"

namespace cave {

using namespace ::cave::math;

// @TODO: refactor this
#if 1
static constexpr uint32_t kTextureWidth = 1920;
static constexpr uint32_t kTextureHeight = 1080;
#else
static constexpr uint32_t kTextureWidth = 640;
static constexpr uint32_t kTextureHeight = 480;
#endif

TileMapEditor::TileMapEditor(EditorState& editor,
                             DocId doc_id,
                             SceneId scene_id)
    : Tab(editor, doc_id)
    , view_manager_(editor.app().services().viewManager())
    , debug_id_(MakeDebugId(this))
    , preview_scene_id_(scene_id)
    , m_sprite_selector(SpriteSelector::SelectionMode::Single) {

    m_brush_desc = ToolBarButtonDesc{ ICON_FA_BRUSH, "TileMap editor mode",
                                      [&]() {
                                          LOG_WARN("TODO");
                                      } };

    {
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
}

TileMapEditor::~TileMapEditor() = default;

void TileMapEditor::submitView() {
    using namespace render;
    ViewDesc view;
    view.view_id = view_id_;
    view.viewport_px = { 0, 0, kTextureWidth, kTextureHeight };
    view.scene_id = preview_scene_id_;
    view.camera_source = CameraSource::External(camera_);

    view.output = texture_;
    view_manager_.submit(view);
}

void TileMapEditor::onCreate() {
    camera_.SetAspect((float)kTextureWidth / (float)kTextureHeight);
    camera_.SetDirty();
    camera_.SetProjection(ProjectionType::Orthographic);
    camera_transform_.Translate(Vector3f(0, 0, 4));
    camera_controller_ = std::make_unique<CameraController2DEditor>(camera_, camera_transform_);

    camera_transform_.UpdateTransform();
    camera_.Update(camera_transform_.GetWorldMatrix());

    IApplication& app = m_editor.app();

    app.services().sceneScheduler().add(this);
    m_editor.PickingService().Register(this);

    view_id_ = view_manager_.createView(
        "SceneView",
        { 0, 0, kTextureWidth, kTextureHeight });
}

void TileMapEditor::onDestroy() {
    IApplication& app = m_editor.app();

    view_manager_.destroyView(view_id_);
    m_editor.PickingService().Register(this);
    app.services().sceneScheduler().remove(this);
}

Option<PickData> TileMapEditor::GetPickData(const Vector2f& pointer_os) {
    unused(pointer_os);

    return None();

    // if (!IsVisible()) return None();

    // const ViewRecord* view = view_manager_.resolve(view_id_);
    // if (!view->display_rect_os.Contains(pointer_os.x, pointer_os.y)) {
    //     return None();
    // }

    // return Some(PickData{
    //     .proj_view = camera_.GetProjectionViewMatrix(),
    //     .cursor_ndc = view->screenToNDC(pointer_os),
    //     .scene_id = preview_scene_id_,
    //     .doc_id = doc_id_,
    // });
}

void TileMapEditor::collectSceneTicks(std::vector<SceneTickRequest>& out_requests) {
    if (!m_editor.IsPlaying()) {
        out_requests.push_back(SceneTickRequest{
            SceneTickMode::Editor,
            preview_scene_id_,
        });
    }
}

void TileMapEditor::onInputEvents(const InputFrame& input) {
    if (!IsHovered()) {
        return;
    }

    unused(input);
    // @TODO: impl
}

void TileMapEditor::drawUIImpl() {
    ViewRecord* view = view_manager_.resolve(view_id_);
    DEV_ASSERT(view);

    updateRect(view->display_rect_os);
    drawMainView(view->display_rect_os);

    submitView();
}

// @TODO: instead of asking for image, provide an image to renderer
void TileMapEditor::drawMainView(const math::FloatRect& rect) {
    const ImVec2 min{ rect.x, rect.y };
    const ImVec2 max{ rect.Right(), rect.Bottom() };

    using rhi::Backend;

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

// @TODO: refactor
static void fitAspect(float aspect, float& w, float& h) {
    if (aspect * h > w) {
        h = w / aspect;
    } else {
        w = h * aspect;
    }
}

void TileMapEditor::updateRect(math::FloatRect& out_rect) {
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
#if 0
void TileMapEditor::OnCreateInternal(const Guid& p_guid) {

    auto scene_manager = static_cast<EditorSceneManager*>(SceneManager::GetSingletonPtr());
    DEV_ASSERT(scene_manager);

    m_tmp_scene = scene_manager->CreateTempScene(p_guid, [&]() {
        auto scene = std::make_shared<Scene>();
        auto root = EntityFactory::CreateTransformEntity(*scene, "tile_map_test_scene");
        scene->m_root = root;

        auto id = EntityFactory::CreateTileMapEntity(*scene, "tile_map");
        scene->AttachChild(id);

        TileMapRendererComponent* tile_map_renderer = scene->GetComponent<TileMapRendererComponent>(id);
        tile_map_renderer->SetResourceGuid(p_guid);
        return scene;
    });
}

void TileMapEditor::DrawAssetInspector() {
    TileMapAsset* tile_map = m_document->GetHandle<TileMapAsset>().Get();
    TileSetAsset* tile_set = tile_map->GetTileSetHandle().Get();

    std::vector<AssetChildPanel> descs = {
        {
            "LayerOverview",
            720,
            [&]() {
                if (ImGui::BeginTabBar("##MyTabs1")) {
                    if (ImGui::BeginTabItem("Layer")) {
                        TileMapLayerOverview(*tile_map);
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            },
        },
        {
            "PaintTab",
            0,
            [&]() {
                if (tile_set) {
                    auto handle = tile_set->GetHandle();
                    const int column = tile_set->GetCol();
                    const int row = tile_set->GetRow();
                    if (auto image = handle.Get(); image) {
                        m_sprite_selector.SelectSprite(*image, &column, &row);
                    }
                }
            },
        }
    };

    const float full_width = ImGui::GetContentRegionAvail().x;

    ui::DrawContents(full_width, descs);
}

bool TileMapEditor::CursorToTile(const Vector2f& p_in, TileIndex& p_out) const {
    auto res = m_viewer.CursorToNDC(p_in);
    if (res.is_none()) {
        return false;
    }

    auto ndc_2 = res.unwrap_unchecked();
    Vector4f ndc{ ndc_2.x, ndc_2.y, 0.0f, 1.0f };

    DEV_ASSERT(0);
    CameraComponent cam;
    const auto inv_proj_view = glm::inverse(cam.GetProjectionViewMatrix());

    Vector4f position = inv_proj_view * ndc;
    position /= position.w;

    p_out.x = static_cast<int16_t>(std::floor(position.x));
    p_out.y = static_cast<int16_t>(std::floor(position.y));

    return true;
}

#if 0
bool TileMapEditor::HandleInput(const OldInputEvent* p_input_event) {
    DEV_ASSERT(0);
    unused(p_input_event);
    if (auto e = dynamic_cast<const InputEventMouse*>(p_input_event); e) {
        if (!e->IsModiferPressed()) {
            if (e->IsButtonDown(MouseButton::LEFT)) {
                auto selections = m_sprite_selector.GetSelections();
                if (!selections.empty()) {
                    // @TODO: support multi tile editing
                    auto [x, y] = selections[0];
                    if (x >= 0 && y >= 0) {
                        TileMapAsset* tile_map = m_document->GetHandle<TileMapAsset>().Get();
                        TileSetAsset* tile_set = tile_map->GetTileSetHandle().Get();
                        uint32_t idx = y * tile_set->GetCol() + x;
                        m_document->RequestAdd(e->GetPos(), TileId(idx));
                    }
                }
                return true;
            }
            if (e->IsButtonDown(MouseButton::RIGHT)) {
                m_document->RequestErase(e->GetPos());
                return true;
            }
        }
    }

    return false;
}
#endif

void TileMapEditor::TileMapLayerOverview(TileMapAsset& p_tile_map) {
    if (ImGui::Button(ICON_FA_SQUARE_PLUS " Add Layer")) {
        // p_tile_map.AddLayer("untitled layer");
    }
    ImGui::Separator();

    auto tool = dynamic_cast<TileMapEditor*>(m_editor.GetViewer().GetActiveTab());
    DEV_ASSERT(tool);

    for (int layer_id = 0; layer_id < 1; ++layer_id) {
        TileMapAsset& layer = p_tile_map;
        const bool is_layer_selected = true;

        ImGui::PushID(layer_id);

        if (is_layer_selected) {
            auto& style = ImGui::GetStyle();
            auto& colors = style.Colors;
            ImGui::PushStyleColor(ImGuiCol_ChildBg, colors[ImGuiCol_FrameBgHovered]);
        }

        ImGui::BeginGroup();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 0));

        ImGui::BeginGroup();

        ImGui::Dummy(ImVec2(8, 8));

        if (ui::TextBox("layer", layer.GetName())) {
            // @TODO: notify dirty
        }

        ImGui::SameLine();

        const bool is_visible = layer.IsVisible();
        const char* label = is_visible ? ICON_FA_EYE : ICON_FA_EYE_SLASH;
        if (ImGui::Button(label)) {
            layer.SetVisible(!is_visible);
        }

        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_TRASH_CAN)) {
            LOG_WARN("TODO: DELETE");
        }

        // next line

        {

            const ImageAsset* image = nullptr;
            if (auto image_handle = layer.GetTileSetHandle().Get(); image_handle) {
                image = image_handle->GetHandle().Get();
            }

            auto checkerboard = m_editor.context.checkerboard;
            DEV_ASSERT(checkerboard && checkerboard->gpu_texture);

            Vector2f region_size(128, 128);
            ui::CenteredImage(image, region_size, checkerboard->gpu_texture->GetHandle());

            if (ImGui::IsItemClicked()) {
                // tool->SetActiveLayer(layer_id);
            }

            // @TODO: make an asset drop region
            // accept same type of assets, show tooltips, etc
            if (auto _handle = DragDropTarget(AssetType::TileSet); _handle.is_some()) {
                layer.SetTileSetGuid(_handle.unwrap_unchecked().GetGuid());
            }
        }

        ImGui::Dummy(ImVec2(8, 8));

        ImGui::EndGroup();
        ImGui::Separator();

        ImGui::PopStyleVar(2);
        ImGui::PopID();
        ImGui::EndGroup();

        if (is_layer_selected) {
            ImGui::PopStyleColor();
        }
    }
}

const std::vector<const ToolBarButtonDesc*> TileMapEditor::GetToolBarButtons() const {
    return { &m_brush_desc };
}
#endif

}  // namespace cave
