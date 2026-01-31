#pragma once

namespace cave {

// @TODO: entirely refactor this
class CameraComponent;
class Scene;
class IRenderDevice;

enum class PathTracerMode {
    NONE,
    INTERACTIVE,
    TILED,
};

void RequestPathTracerUpdate(const CameraComponent& p_camera, Scene& p_scene);
// path tracer
void SetPathTracerMode(PathTracerMode p_mode);
bool IsPathTracerActive();
void BindPathTracerData(IRenderDevice& p_graphics_manager);
void UnbindPathTracerData(IRenderDevice& p_graphics_manager);

}  // namespace cave
