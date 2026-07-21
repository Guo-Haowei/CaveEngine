#if 0
#include "TileMapEditor.h"

#include "TileMapLayerPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/core/algorithm/Graph.h"
#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/display/DisplayService.h"
#include "cave/runtime/display/ICanvas.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "editor/inspector/PropertyEditors.h"
#include "editor/panels/AssetInspector.h"
#include "editor/services/DocumentService.h"
#include "editor/services/EditorServices.h"
#include "editor/services/EditService.h"
#include "editor/tile_map/SetTileCommand.h"

// @TODO: remove
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/view/ViewManager.h"
#include "editor/utility/ImGuizmo.h"

namespace cave {

using namespace ::cave::math;

TileMapEditor::TileMapEditor(EditorState& editor,
                             DocId doc_id,
                             SceneId scene_id)
    : ViewTabBase(editor, doc_id, scene_id, ViewDimension::Dim2)
    , m_canvas(m_engine_services.canvas())
    , m_debug_id(MakeDebugId(this)) {

}

}  // namespace cave
#endif
