#pragma once
#include "engine/private/renderer/gpu_resource.h"
#include "engine/private/runtime/null/NullRenderDevice.h"

// @TODO: refactor
#include "render_target.h"

namespace cave {

enum VaryingFlag : uint8_t {
    VARYING_COLOR = 1u << 0,
    VARYING_NORMAL = 1u << 1,
    VARYING_UV = 1u << 2,
    VARYING_WORLD_POSITION = 1u << 3,
};

struct alignas(16) VSInput {
    math::Vec4f position;
    math::Vec4f normal;
    math::Vec2f uv;
};

struct alignas(16) VSOutput {
    math::Vec4f position;
    math::Vec4f world_position;
    math::Vec4f normal;
    math::Vec4f color;
    math::Vec2f uv;
};

class SwPipeline {
public:
    SwPipeline(uint8_t varyingFlags)
        : m_varying_flags(varyingFlags) {}

    virtual VSOutput ProcessVertex(const VSInput& input) = 0;

    virtual math::Vec3f ProcessFragment(const VSOutput& input) = 0;

    uint8_t GetVaryingFlags() const { return m_varying_flags; }

    const uint8_t m_varying_flags;
};

struct SwMesh : GpuMesh {
    using GpuMesh::GpuMesh;

    std::vector<VSInput> vertices;
    std::vector<uint32_t> indices;
};

struct OutTriangle {
    VSOutput p0, p1, p2;
    int discarded = false;
};

class SwGraphicsManager : public render::NullRenderDevice {
public:
    SwGraphicsManager()
        : NullRenderDevice("SwGraphicsManager") {}

    void Clear(const render::RenderTargetDesc& p_target) override;

    void SetPipelineStateImpl(PipelineStateName p_name) override;

    void DrawElements(uint32_t p_count, uint32_t p_offset = 0) override;

    void DrawArrays(uint32_t p_count, uint32_t p_offset = 0) override;

    auto CreateMesh(const MeshAsset& p_mesh) -> Result<std::shared_ptr<GpuMesh>> override;

    void SetMesh(const GpuMesh* p_mesh) override;

    //---------------------------------
    // @TODO: refactor
    struct RenderState {
        SwPipeline* pipeline = nullptr;
        SwRenderTarget* rt = nullptr;

        const VSInput* vertices = nullptr;
        const uint32_t* indices = nullptr;
    };

    RenderState m_state;

    void SetPipeline(SwPipeline* p_pipeline) { m_state.pipeline = p_pipeline; }
    void setRenderTarget(SwRenderTarget* renderTarget) { m_state.rt = renderTarget; }

    void setSize(int width, int height) {
        DEV_ASSERT(width > 0 && height > 0);
        m_state.rt->resize(width, height);
    }

private:
    void ProcessFragment(OutTriangle& vs_out);

    void DrawArrayInternal(std::vector<OutTriangle>& trigs);

    OutTriangle ProcessTriangle(const VSInput& vs_in0,
                                const VSInput& vs_in1,
                                const VSInput& vs_in2);
};

}  // namespace cave
