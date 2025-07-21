#include "sprite_animation_editor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "engine/assets/image_asset.h"
#include "engine/input/input_event.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/entity_factory.h"

#include "editor/document/document.h"
#include "editor/editor_layer.h"
#include "editor/editor_scene_manager.h"
#include "editor/widgets/widget.h"
#include "editor/viewer/viewer.h"
#include "editor/utility/imguizmo.h"

namespace cave {

SpriteAnimationEditor::SpriteAnimationEditor(EditorLayer& p_editor, Viewer& p_viewer)
    : ViewerTab(p_editor, p_viewer) {

    m_camera = std::make_unique<CameraComponent>();
    ViewerTab::CreateDefaultCamera2D(*m_camera.get());
}

void SpriteAnimationEditor::OnCreate(const Guid& p_guid) {
    ViewerTab::OnCreate(p_guid);
    m_document = std::make_shared<SpriteAnimationDocument>(p_guid);

    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);

    m_tmp_scene = scene_manager->OpenTemporaryScene(p_guid, [&]() {
        auto scene = std::make_shared<Scene>();
        auto root = EntityFactory::CreateTransformEntity(*scene, "sprite_animation_test_scene");
        scene->m_root = root;

        auto id = EntityFactory::CreateTransformEntity(*scene, "test_sprite");
        scene->AttachChild(id);

        auto test_image = AssetRegistry::GetSingleton().FindByPath<ImageAsset>("@res://player/player.png").unwrap();

        SpriteRenderer& sprite_renderer = scene->Create<SpriteRenderer>(id);
        sprite_renderer.SetImage(test_image.GetGuid());
        return scene;
    });
}

void SpriteAnimationEditor::OnDestroy() {
    m_tmp_scene = nullptr;  // decrease ref count
}

void SpriteAnimationEditor::OnActivate() {
    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);
    scene_manager->SetTmpScene(m_tmp_scene);
}

void SpriteAnimationEditor::DrawMainView(const CameraComponent& p_camera) {
    ViewerTab::DrawMainView(p_camera);

    const Matrix4x4f proj_view = p_camera.GetProjectionViewMatrix();

    const Vector2f& canvas_min = m_viewer.GetCanvasMin();
    const Vector2f& canvas_size = m_viewer.GetCanvasSize();

    ImGuizmo::SetOrthographic(true);
    ImGuizmo::BeginFrame();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(canvas_min.x, canvas_min.y, canvas_size.x, canvas_size.y);

    ImGuizmo::DrawAxes(proj_view);

    // m_document->FlushCommands();
}

void SpriteAnimationEditor::ImageSourceDropTarget() {
    auto asset = m_document->GetHandle<SpriteAnimationAsset>().Get();
    DEV_ASSERT(asset);

    ImGui::Text("Source Image");

    ImVec2 region_size(128, 128);

    auto image_handle = asset->GetImageHandle();
    ImageAsset* image = image_handle.Get();

    auto checkerboard = m_editor.context.checkerboard_handle.Get();

    CenteredImage(image, region_size, checkerboard->gpu_texture->GetHandle());

    DragDropTarget(AssetType::Image, [&](AssetHandle& p_handle) {
        DEV_ASSERT(p_handle.GetMeta()->type == AssetType::Image);

        asset->SetGuid(p_handle.GetGuid());
    });
}

void SpriteAnimationEditor::DrawFrameSelector(ImageAsset& p_image_asset) {
    // @TODO: refactor this, this is the same as ViewerTab::DrawToolBar
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    auto& colors = ImGui::GetStyle().Colors;
    const auto& button_hovered = colors[ImGuiCol_ButtonHovered];
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(button_hovered.x, button_hovered.y, button_hovered.z, 0.5f));
    const auto& button_active = colors[ImGuiCol_ButtonActive];
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(button_active.x, button_active.y, button_active.z, 0.5f));

    std::string output;
    DrawInputText("name", output, 80, 160);
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_SQUARE_PLUS "  Add Animation")) {
    }

    ImGui::PopStyleColor(3);
    // -------------

    ImGui::BeginGroup();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 0));

    ImGui::Dummy(ImVec2(8, 8));

    m_sprite_selector.SelectSprite(p_image_asset, nullptr, nullptr);

    ImGui::PopStyleVar(2);

    ImGui::EndGroup();
}

void SpriteAnimationEditor::DrawTimeLine() {
    constexpr int width = 300;
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, width);
    ImGui::SetColumnWidth(1, width);
    {
        std::vector<const char*> clips = { "idle", "walk", "jump" };
        static int current_clip = 0;
        const int clip_count = static_cast<int>(clips.size());
        if (ImGui::BeginCombo("Clips", clips[current_clip])) {
            for (int n = 0; n < clip_count; ++n) {
                const bool is_selected = (current_clip == n);
                if (ImGui::Selectable(clips[n], is_selected)) {
                    current_clip = n;
                }

                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }

    ImGui::NextColumn();

    {
        std::vector<const char*> fps = { "1fps", "2fps", "3fps" };
        static int current = 0;
        const int count = static_cast<int>(fps.size());
        if (ImGui::BeginCombo("FPS", fps[current])) {
            for (int n = 0; n < count; ++n) {
                const bool is_selected = (current == n);
                if (ImGui::Selectable(fps[n], is_selected)) {
                    current = n;
                }

                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }
    ImGui::Columns(2);
}

void SpriteAnimationEditor::DrawAssetInspector() {
    auto sprite_animation = m_document->GetHandle<SpriteAnimationAsset>().Get();
    DEV_ASSERT(sprite_animation);

    auto image_handle = sprite_animation->GetImageHandle();

    std::vector<AssetChildPanel> descs = {
        {
            "LayerOverview",
            360,
            [&]() {
                if (ImGui::BeginTabBar("##MyTabs1")) {
                    if (ImGui::BeginTabItem("Animation")) {
                        ImageSourceDropTarget();
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            },
        },
        {
            "SpriteTab",
            360,
            [&]() {
                m_sprite_selector.EditSprite(nullptr, nullptr);
            },
        },
        {
            "PaintTab",
            600,
            [&]() {
                ImageAsset* image = image_handle.Get();
                if (image) {
                    DrawFrameSelector(*image);
                }
            },
        },
        {
            "TileLine",
            0,
            [&]() {
                DrawTimeLine();
            },
        },
    };

    const float full_width = ImGui::GetContentRegionAvail().x;

    DrawContents(full_width, descs);
}

Document& SpriteAnimationEditor::GetDocument() const {
    return *m_document.get();
}

bool SpriteAnimationEditor::HandleInput(const InputEvent* p_input_event) {
    unused(p_input_event);
    return false;
}

const CameraComponent& SpriteAnimationEditor::GetActiveCameraInternal() const {
    return *m_camera.get();
}

}  // namespace cave
