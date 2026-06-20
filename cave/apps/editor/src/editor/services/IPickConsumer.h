#pragma once
#include "cave/core/ids/DebugId.h"
#include "cave/core/ids/SceneId.h"
#include "cave/core/math/Box.h"
#include "cave/core/math/Matrix.h"

#include "editor/document/DocId.h"

namespace cave {

struct PickData {
    math::Mat4f proj_view;
    math::Vec2f cursor_ndc;
    SceneId scene_id;
    DocId doc_id;
};

class IPickConsumer {
public:
    virtual ~IPickConsumer() = default;

    virtual Option<PickData> getPickData(const math::Vec2f& p_pos_screen) = 0;

    virtual DebugId debugId() const = 0;
};

}  // namespace cave