#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/core/math/Rect.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/input/IInputConsumer.h"

#include "editor/document/DocId.h"
#include "editor/services/IPickConsumer.h"
#include "editor/services/EditorServices.h"

#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

class CameraComponent;

enum class SceneViewToolType : uint8_t {
    Select = 0,
    TilePaint,

    Count,
};

struct SceneToolContext {
    EngineServices& engine_services;
    EditorServices& editor_services;
    CameraComponent& camera;
    ViewId view_id;
    SceneId scene_id;
    DocId doc_id;
};

class ISceneViewTool {
public:
    ISceneViewTool(const SceneToolContext& ctx) noexcept
        : m_ctx(ctx) {}

    virtual ~ISceneViewTool() = default;

    virtual Option<PickData> getPickData(const math::Vec2f&) { return None(); }

    virtual void onInputEvents(const InputFrame& input) = 0;

    virtual void draw(const math::FloatRect& rect) = 0;

    Scene* getResolvedScene() {
        return m_ctx.engine_services.sceneRegistry().resolve(m_ctx.scene_id);
    }

protected:
    SceneToolContext m_ctx;
};

}  // namespace cave