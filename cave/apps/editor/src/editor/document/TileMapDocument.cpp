#include "TileMapDocument.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

#include "editor/tile_map_editor/TileMapEditor.h"

// @TODO: remove private #include
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/ui/inputs.h"
#include "engine/private/ui/layout.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/assets/TileSetAsset.h"

#include "editor/EditorState.h"

namespace cave {

using namespace ::cave::literals;
using ecs::Entity;

TileMapDocument::TileMapDocument(AppServices& services, const Guid& guid)
    : DocumentBase(services, guid) {

    SceneCommandWriter cb(services.assetRegistry());
    Entity root = cb.CreateRootObject();

    Entity ent = cb.CreateTileMapObject("tilemap");
    cb.AttachChild(ent, root);
    cb.SetProperty(ent, TileMapRendererComponent_Id, "tile_map_id"_sid, guid);

    auto scene = std::make_unique<Scene>(std::format("preview-tile-map-{}", guid.ToString()));

    SceneCommandExecutor executor(*scene);
    EntityMap map(cb.GetAllocationCount());
    SceneCommandPlayback::Play(cb, executor, { map, *scene });
    scene->m_root = map.Resolve(root);
    scene->Update(0.0f);

    preview_scene_ = scene_reg_.registerScene(std::move(scene));
}

void TileMapDocument::tileMapLayerOverview(TileMapAsset& p_tile_map) {
    if (ImGui::Button(ICON_FA_SQUARE_PLUS " Add Layer")) {
        // p_tile_map.AddLayer("untitled layer");
    }
    ImGui::Separator();

    //auto tool = dynamic_cast<TileMapEditor*>(m_editor.GetViewer().GetActiveTab());
    //DEV_ASSERT(tool);

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

        // if (ui::TextBox("layer", layer.GetName())) {
        //     // @TODO: notify dirty
        // }

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

            //auto checkerboard = m_editor.context.checkerboard;
            //DEV_ASSERT(checkerboard && checkerboard->gpu_texture);

            //Vector2f region_size(128, 128);
            //ui::CenteredImage(image, region_size, checkerboard->gpu_texture->GetHandle());

            if (ImGui::IsItemClicked()) {
                // tool->SetActiveLayer(layer_id);
            }

            // @TODO: make an asset drop region
            // accept same type of assets, show tooltips, etc
            //if (auto _handle = DragDropTarget(AssetType::TileSet); _handle.is_some()) {
            //    layer.SetTileSetGuid(_handle.unwrap_unchecked().GetGuid());
            //}
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
void TileMapDocument::drawAssetInspector() {
    TileMapAsset* tile_map = handle<TileMapAsset>().Get();

    TileSetAsset* tile_set = tile_map->GetTileSetHandle().Get();

    std::vector<AssetChildPanel> descs = {
        {
            "LayerOverview",
            720,
            [&]() {
                if (ImGui::BeginTabBar("##MyTabs1")) {
                    if (ImGui::BeginTabItem("Layer")) {
                        tileMapLayerOverview(*tile_map);
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
                    //auto handle = tile_set->GetHandle();
                    //const int column = tile_set->GetCol();
                    //const int row = tile_set->GetRow();
                    //if (auto image = handle.Get(); image) {
                    //    m_sprite_selector.SelectSprite(*image, &column, &row);
                    //}
                }
            },
        }
    };

    const float full_width = ImGui::GetContentRegionAvail().x;

    ui::DrawContents(full_width, descs);
}

#if 0
// @TODO: abstract brush class
class SetTileCommand : public UndoCommand {
public:
    bool Undo() override {
        return SetTile(m_old_tile);
    }

    bool Redo() override {
        return SetTile(m_new_tile);
    }

    static std::unique_ptr<SetTileCommand> AddTile(TileMapAsset& p_tile_map, TileIndex p_index, TileId p_tile);

    static std::unique_ptr<SetTileCommand> RemoveTile(TileMapAsset& p_tile_map, TileIndex p_index);

    bool MergeCommand(const UndoCommand* p_other) override;

    void SetHandle(Handle<TileMapAsset>&& p_handle) {
        m_handle = std::move(p_handle);
    }

private:
    bool SetTile(Option<TileId> p_new_tile);

    Handle<TileMapAsset> m_handle;
    TileIndex m_index;

    Option<TileId> m_old_tile{ None() };
    Option<TileId> m_new_tile{ None() };
};

std::unique_ptr<SetTileCommand> SetTileCommand::AddTile(TileMapAsset& p_tile_map, TileIndex p_index, TileId p_tile) {
    Option<TileId> old_tile = p_tile_map.GetTile(p_index);

    if (!p_tile_map.AddTile(p_index, p_tile)) {
        return nullptr;
    }

    p_tile_map.IncRevision();

    auto cmd = std::make_unique<SetTileCommand>();
    cmd->m_old_tile = old_tile;
    cmd->m_new_tile = Some(p_tile);
    cmd->m_index = p_index;
    return cmd;
}

std::unique_ptr<SetTileCommand> SetTileCommand::RemoveTile(TileMapAsset& p_tile_map, TileIndex p_index) {
    Option<TileId> old_tile = p_tile_map.GetTile(p_index);

    if (!p_tile_map.RemoveTile(p_index)) {
        return nullptr;
    }

    p_tile_map.IncRevision();

    auto cmd = std::make_unique<SetTileCommand>();
    cmd->m_old_tile = old_tile;
    cmd->m_new_tile = None();
    cmd->m_index = p_index;
    return cmd;
}

bool SetTileCommand::MergeCommand(const UndoCommand* p_other) {
    if (auto other = dynamic_cast<const SetTileCommand*>(p_other); other) {
        return other->m_index == m_index &&
               other->m_new_tile == m_new_tile &&
               other->m_old_tile == m_old_tile &&
               other->m_handle.GetGuid() == m_handle.GetGuid();
    }
    return false;
}

bool SetTileCommand::SetTile(Option<TileId> p_new_tile) {
    TileMapAsset* tile_map = m_handle.Get();
    if (!tile_map) {
        return false;
    }
    const bool ok = p_new_tile.is_none() ? tile_map->RemoveTile(m_index) : tile_map->AddTile(m_index, p_new_tile.unwrap_unchecked());
    DEV_ASSERT(ok);
    tile_map->IncRevision();
    return ok;
}

void TileMapDocument::RequestAdd(const Vector2f& p_cursor, const TileId& p_id) {
    TileIndex tile;
    if (m_tile_map_editor.CursorToTile(p_cursor, tile)) {
        m_commands.emplace_back(CommandAddTile{ tile, p_id });
    }
}

void TileMapDocument::RequestErase(const Vector2f& p_cursor) {
    TileIndex tile;
    if (m_tile_map_editor.CursorToTile(p_cursor, tile)) {
        m_commands.emplace_back(CommandEraseTile{ tile });
    }
}

void TileMapDocument::FlushCommands() {
    auto handle = GetHandle<TileMapAsset>();
    TileMapAsset* tile_map = handle.Get();
    DEV_ASSERT(tile_map);
    if (!tile_map) return;

    // process commands
    for (const auto& command : m_commands) {
        std::visit([&](auto&& p_cmd) {
            using T = std::decay_t<decltype(p_cmd)>;
            if constexpr (std::is_same_v<T, CommandAddTile>) {
                if (auto cmd = SetTileCommand::AddTile(*tile_map, p_cmd.tile, p_cmd.id); cmd) {
                    m_dirty = true;
                    cmd->SetHandle(std::move(handle));
                    m_undo_stack->Submit(std::move(cmd));
                }
            } else if constexpr (std::is_same_v<T, CommandEraseTile>) {
                if (auto cmd = SetTileCommand::RemoveTile(*tile_map, p_cmd.tile); cmd) {
                    m_dirty = true;
                    cmd->SetHandle(std::move(handle));
                    m_undo_stack->Submit(std::move(cmd));
                }
            }
        },
                   command);
    }

    m_commands.clear();
}
#endif

}  // namespace cave
