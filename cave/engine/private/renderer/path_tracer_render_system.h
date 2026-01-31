#pragma once

namespace cave::render {
class IRenderDevice;
}  // namespace cave::render

namespace cave {

// @TODO: entirely refactor this
class CameraComponent;
class Scene;

enum class PathTracerMode {
    NONE,
    INTERACTIVE,
    TILED,
};

void RequestPathTracerUpdate(const CameraComponent& p_camera, Scene& p_scene);
// path tracer
void SetPathTracerMode(PathTracerMode p_mode);
bool IsPathTracerActive();
void BindPathTracerData(render::IRenderDevice& p_graphics_manager);
void UnbindPathTracerData(render::IRenderDevice& p_graphics_manager);

}  // namespace cave
