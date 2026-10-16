#include "OpenGlRenderDevice.h"

#include <imgui/backends/imgui_impl_opengl3.h>

#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/render/render_graph/RenderGraphDefines.h"
#include "engine/private/render/renderer/GraphicsDvars.h"
#include "engine/private/runtime/display/GlfwDisplayService.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/scene/Scene.h"

#include "OpenGlCommandList.h"
#include "OpenGlFramebufferCache.h"
#include "OpenGlHelpers.h"
#include "OpenGlPipelineStateManager.h"
#include "OpenGlResources.h"

// @NOTE: include GLFW after opengl
// @TODO: opengl shouldn't know about glfw
#include <GLFW/glfw3.h>

// #define RESIDENT_TEXTURE USE_IF(USING(PLATFORM_WINDOWS))
#define RESIDENT_TEXTURE NOT_IN_USE

//-----------------------------------------------------------------------------------------------------------------

// @TODO: wrap this
#define GL_CHECK(stmt)                                                                                                       \
    do {                                                                                                                     \
        stmt;                                                                                                                \
        GLenum err = glGetError();                                                                                           \
        if (err != GL_NO_ERROR) {                                                                                            \
            ::cave::ReportErrorImpl(__FUNCTION__, __FILE__, __LINE__, std::format("OpenGl Error (0x{:0>8X}): " #stmt, err)); \
        }                                                                                                                    \
    } while (0)

namespace cave::render {

OpenGlRenderDevice::OpenGlRenderDevice()
    : RenderDevice("OpenGlRenderDevice", Backend::OpenGl, 1) {
    m_window = nullptr;
    m_fbo_cache = MakeOwner<OpenGlFramebufferCache>();

    auto pso_manager = MakeOwner<OpenGlPipelineStateManager>();
    m_command_list = MakeOwner<OpenGlCommandList>(*m_fbo_cache, *pso_manager);
    m_pipeline_state_manager = std::move(pso_manager);
}

OpenGlRenderDevice::~OpenGlRenderDevice() = default;

void OpenGlRenderDevice::FinalizeImpl() {
    m_fbo_cache.reset();

    m_pipeline_state_manager->finalize();
}

auto OpenGlRenderDevice::createBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuBuffer>> {
    auto type = gl::Convert(desc.type);

    const GLenum usage = desc.dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    GLuint handle = 0;
    glGenBuffers(1, &handle);
    glBindBuffer(type, handle);
    glBufferData(type, desc.element_count * desc.element_size, desc.initial_data, usage);
    glBindBuffer(type, 0);

    auto buffer = MakeRef<OpenGlBuffer>(desc);
    buffer->handle = handle;
    buffer->type = type;
    return buffer;
}

auto OpenGlRenderDevice::createMeshImpl(const GpuMeshDesc& mesh_desc,
                                        std::span<const GpuBufferDesc> vb_descs,
                                        const GpuBufferDesc* ib_desc) -> Result<Ref<GpuMesh>> {
    // create VAO
    uint32_t vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    auto ret = MakeRef<OpenGlGpuMesh>(mesh_desc);
    ret->vao = vao;

    // create EBO
    if (ib_desc) {
        auto res = createBuffer(*ib_desc);
        if (!res) {
            return CAVE_ERROR(res.error());
        }
        ret->indexBuffer = *res;

        const uint32_t ebo = ret->indexBuffer->getHandle32();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    }

    for (uint32_t slot = 0; slot < (uint32_t)vb_descs.size(); ++slot) {
        if (vb_descs[slot].element_count == 0) {
            continue;
        }

        auto res = createBuffer(vb_descs[slot]);
        if (!res) {
            return CAVE_ERROR(res.error());
        }
        ret->vertexBuffers[slot] = *res;

        const uint32_t vbo = ret->vertexBuffers[slot]->getHandle32();
        const uint32_t stride_in_byte = ret->desc.vertex_layout[slot].stride_in_byte;

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(slot,
                              stride_in_byte / sizeof(float),
                              GL_FLOAT,
                              GL_FALSE,
                              stride_in_byte,
                              0);
        glEnableVertexAttribArray(slot);
    }

    glBindVertexArray(0);
    return ret;
}

void OpenGlRenderDevice::updateBuffer(const GpuBufferDesc& p_desc, GpuBuffer* p_buffer) {
    DEV_ASSERT(p_desc.element_size == p_buffer->desc.element_size);
    if (DEV_VERIFY(p_buffer->desc.element_count >= p_desc.element_count)) {
        const uint32_t size_in_byte = p_desc.element_count * p_desc.element_size;
        const uint32_t vbo = p_buffer->getHandle32();
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, size_in_byte, p_desc.initial_data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

auto OpenGlRenderDevice::createConstantBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuBuffer>> {
    GLuint handle = 0;

    glGenBuffers(1, &handle);
    if (handle == 0) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "failed to generate buffer");
    }

    glBindBuffer(GL_UNIFORM_BUFFER, handle);
    glBufferData(GL_UNIFORM_BUFFER, desc.element_count * desc.element_size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, desc.slot, handle);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    auto buffer = MakeRef<OpenGlBuffer>(desc);
    buffer->handle = handle;
    return buffer;
}

auto OpenGlRenderDevice::createStructuredBuffer(const GpuBufferDesc&) -> Result<Ref<GpuBuffer>> {
    CRASH_NOW();
    return nullptr;
}

void OpenGlRenderDevice::updateBufferData(const GpuBufferDesc&, const GpuBuffer*) {
    CRASH_NOW();
}

void OpenGlRenderDevice::updateConstantBuffer(const GpuBuffer* buffer, const void* data, size_t size) {
    auto gl_buffer = reinterpret_cast<const OpenGlBuffer*>(buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, gl_buffer->handle);
    glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGlRenderDevice::generateMipmap(const GpuTexture* base) {
    auto dimension = gl::ConvertDimension(base->desc.dimension);
    glBindTexture(dimension, base->GetHandle32());
    glGenerateMipmap(dimension);
    glBindTexture(dimension, 0);
}

Ref<GpuTexture> OpenGlRenderDevice::createTextureImpl(const GpuTextureDesc& p_texture_desc, const SamplerDesc& p_sampler_desc) {
    GLuint texture_id = 0;
    glGenTextures(1, &texture_id);

    GLenum texture_type = gl::ConvertDimension(p_texture_desc.dimension);
    GLenum internal_format = gl::ConvertInternalFormat(p_texture_desc.format);
    GLenum format = gl::ConvertFormat(p_texture_desc.format);
    GLenum data_type = gl::ConvertDataType(p_texture_desc.format);

    glBindTexture(texture_type, texture_id);

    switch (texture_type) {
        case GL_TEXTURE_2D: {
            GL_CHECK(glTexImage2D(GL_TEXTURE_2D,
                                  0,
                                  internal_format,
                                  p_texture_desc.width,
                                  p_texture_desc.height,
                                  0,
                                  format,
                                  data_type,
                                  p_texture_desc.initial_data));
        } break;
        case GL_TEXTURE_CUBE_MAP: {
            for (int i = 0; i < 6; ++i) {
                GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                      0,
                                      internal_format,
                                      p_texture_desc.width,
                                      p_texture_desc.height,
                                      0,
                                      format,
                                      data_type,
                                      p_texture_desc.initial_data));
            }
        } break;
#if !USING(USE_GLES3)
        case GL_TEXTURE_CUBE_MAP_ARRAY: {
            GL_CHECK(glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY,
                                  0,
                                  internal_format,
                                  p_texture_desc.width,
                                  p_texture_desc.height,
                                  p_texture_desc.array_size,
                                  0, format, data_type, p_texture_desc.initial_data));
        } break;
#endif
        case GL_TEXTURE_3D: {
            GL_CHECK(glTexStorage3D(GL_TEXTURE_3D,
                                    p_texture_desc.mip_levels,
                                    internal_format,
                                    p_texture_desc.width,
                                    p_texture_desc.height,
                                    p_texture_desc.depth));
        } break;
        default:
            CRASH_NOW();
            break;
    }

    gl::SetSampler(texture_type, p_sampler_desc);
    if (p_texture_desc.misc_flags & RESOURCE_MISC_GENERATE_MIPS) {
        glGenerateMipmap(texture_type);
    }

    glBindTexture(texture_type, 0);

#if USING(RESIDENT_TEXTURE)
    GLuint64 resident_id = glGetTextureHandleARB(texture_id);
    glMakeTextureHandleResidentARB(resident_id);
#else
    GLuint64 resident_id = 0;
#endif

    auto texture = MakeRef<OpenGlGpuTexture>(p_texture_desc);
    texture->handle = texture_id;
    texture->residentHandle = resident_id;
    return texture;
}

void OpenGlRenderDevice::render() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // @TODO: refactor this
    if (m_app->isRuntime()) {
        CRASH_NOW();
        // const auto [width, height] = m_app->GetDisplayService()->windowSize();
        // unused(width);
        // unused(height);
        //  RenderGraphBuilder::DrawDebugImages(*GetRenderData(),
        //                                                width,
        //                                                height,
        //                                                *this);
    }

    if (m_app->specification().enableImgui) {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}

void OpenGlRenderDevice::present() {
    CAVE_PROFILE_EVENT();

    if (m_app->specification().enableImgui) {
        GLFWwindow* oldContext = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(oldContext);
    }

    glfwSwapBuffers(m_window);
}

ICommandList* OpenGlRenderDevice::getCommandList() const {
    return m_command_list.get();
}

}  // namespace cave::render
