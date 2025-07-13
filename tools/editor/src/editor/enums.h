#pragma once

namespace my {

enum class ToolType {
    None,
    Edit,
    TileMap,
    Count,
};

enum class ToolCameraPolicy {
    Any,
    Only2D,
};

enum class GizmoAction : uint8_t {
    Translate,
    Rotate,
    Scale,
};

}  // namespace my
