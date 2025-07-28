#pragma once
#include "engine/empty/empty_graphics_manager.h"

// @TODO: refactor
#include "render_target.h"
#include "shader_program.h"

namespace cave::rs {

void setSize(int width, int height);

// pipeline state
void setVertexShader(IVertexShader* vs);
void setFragmentShader(IFragmentShader* fs);

void setVertexArray(const VSInput* vertices);
void setIndexArray(const unsigned int* indices);

void setRenderTarget(RenderTarget* renderTarget);

class SoftwareRenderer : public EmptyGraphicsManager {
public:
    SoftwareRenderer()
        : EmptyGraphicsManager("SoftwareRenderer") {}

    void Clear(const Framebuffer* p_framebuffer,
               ClearFlags p_flags,
               const float* p_clear_color = DEFAULT_CLEAR_COLOR,
               float p_clear_depth = 1.0f,
               uint8_t p_clear_stencil = 0,
               int p_index = 0) override;

    void SetPipelineStateImpl(PipelineStateName p_name) override;

    void DrawElements(uint32_t p_count, uint32_t p_offset = 0) override;

    void DrawArrays(uint32_t p_count, uint32_t p_offset = 0) override;
};

}  // namespace cave::rs
