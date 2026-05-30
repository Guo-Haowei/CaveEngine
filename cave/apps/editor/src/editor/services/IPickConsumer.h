#pragma once
#include "cave/core/ids/DebugId.h"
#include "cave/core/ids/SceneId.h"
#include "cave/core/math/Box.h"
#include "cave/core/math/Matrix.h"

#include "editor/document/DocId.h"

namespace cave {

struct PickData {
    math::Matrix4x4f proj_view;
    math::Vector2f cursor;  // cursor in viewport space
    math::Vector2f extent;  // viewport extent
    SceneId scene_id;
    DocId doc_id;
};

class IPickConsumer {
public:
    virtual ~IPickConsumer() = default;

    virtual Option<PickData> GetPickData(const math::Vector2f& p_pos_screen) = 0;

    virtual DebugId GetDebugId() const = 0;
};

}  // namespace cave