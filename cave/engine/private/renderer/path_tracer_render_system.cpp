
#include "path_tracer_render_system.h"

#include "engine/private/renderer/path_tracer/path_tracer.h"

namespace cave {

using namespace cave::render;

static struct {
    PathTracer pt;
} s_glob;

void RequestPathTracerUpdate(Scene& p_scene) {
    // @TODO: refactor
    s_glob.pt.Update(p_scene);
}

// path tracer
void SetPathTracerMode(PathTracerMode p_mode) {
    s_glob.pt.SetMode(p_mode);
}

bool IsPathTracerActive() {
    return s_glob.pt.IsActive();
}

void BindPathTracerData(IRenderDevice& p_graphics_manager) {
    s_glob.pt.BindData(p_graphics_manager);
}

void UnbindPathTracerData(IRenderDevice& p_graphics_manager) {
    s_glob.pt.UnbindData(p_graphics_manager);
}

}  // namespace cave
