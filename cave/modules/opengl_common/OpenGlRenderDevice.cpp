#include "OpenGlRenderDevice.h"

#include <imgui/backends/imgui_impl_opengl3.h>

#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/core/math/geometry.h"
#include "engine/private/render/render_graph/RenderGraphDefines.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/runtime/display/GlfwDisplayService.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/scene/Scene.h"

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
            ::cave::ReportErrorImpl(__FUNCTION__, __FILE__, __LINE__, std::format("OpenGL Error (0x{:0>8X}): " #stmt, err)); \
        }                                                                                                                    \
    } while (0)

namespace cave::render {

OpenGlRenderDevice::OpenGlRenderDevice()
    : RenderDevice("OpenGlRenderDevice", rhi::Backend::OpenGL, 1) {
    m_dummy_vao = 0;
    m_window = nullptr;
    m_pipeline_state_manager = MakeOwner<OpenGlPipelineStateManager>();
    m_fbo_cache = MakeOwner<OpenGlFramebufferCache>();
}

OpenGlRenderDevice::~OpenGlRenderDevice() = default;

void OpenGlRenderDevice::FinalizeImpl() {
    m_fbo_cache.reset();

    m_pipeline_state_manager->finalize();
}

void OpenGlRenderDevice::setPipelineStateImpl(PipelineStateName p_name) {
    auto pipeline = reinterpret_cast<OpenGlPipelineState*>(m_pipeline_state_manager->findPSO(p_name));

    if (pipeline->desc.rasterizer_desc) {
        const auto cull_mode = pipeline->desc.rasterizer_desc->cullMode;
        if (cull_mode != m_stateCache.cullMode) {
            switch (cull_mode) {
                case cave::CullMode::NONE:
                    glDisable(GL_CULL_FACE);
                    break;
                case cave::CullMode::FRONT:
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_FRONT);
                    break;
                case cave::CullMode::BACK:
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_BACK);
                    break;
                case cave::CullMode::FRONT_AND_BACK:
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_FRONT_AND_BACK);
                    break;
                default:
                    CRASH_NOW();
                    break;
            }
            m_stateCache.cullMode = cull_mode;
        }

        const bool front_counter_clockwise = pipeline->desc.rasterizer_desc->frontCounterClockwise;
        if (front_counter_clockwise != m_stateCache.frontCounterClockwise) {
            glFrontFace(front_counter_clockwise ? GL_CCW : GL_CW);
            m_stateCache.frontCounterClockwise = front_counter_clockwise;
        }
    }

    if (pipeline->desc.depth_stencil_desc) {
        {
            const bool enable_depth_test = pipeline->desc.depth_stencil_desc->depthEnabled;
            if (enable_depth_test != m_stateCache.enableDepthTest) {
                if (enable_depth_test) {
                    glEnable(GL_DEPTH_TEST);
                } else {
                    glDisable(GL_DEPTH_TEST);
                }
                m_stateCache.enableDepthTest = enable_depth_test;
            }

            if (enable_depth_test) {
                const auto func = pipeline->desc.depth_stencil_desc->depthFunc;
                if (func != m_stateCache.depthFunc) {
                    glDepthFunc(gl::Convert(func));
                    m_stateCache.depthFunc = func;
                }
            }
        }
        {
            const bool enable_stencil_test = pipeline->desc.depth_stencil_desc->stencilEnabled;
            if (enable_stencil_test != m_stateCache.enableStencilTest) {
                if (enable_stencil_test) {
                    glEnable(GL_STENCIL_TEST);
                } else {
                    glDisable(GL_STENCIL_TEST);
                }
                m_stateCache.enableStencilTest = enable_stencil_test;
            }

            if (enable_stencil_test) {
                const auto& face = pipeline->desc.depth_stencil_desc->frontFace;
                const auto stencil_func = gl::Convert(face.stencilFunc);
                const auto fail_op = gl::Convert(face.stencilFailOp);
                const auto zfail_op = gl::Convert(face.stencilDepthFailOp);
                const auto zpass_op = gl::Convert(face.stencilPassOp);
                glStencilOp(fail_op, zfail_op, zpass_op);
                glStencilFunc(stencil_func, 0, 0xFF);
                m_stateCache.stencilFunc = face.stencilFunc;
            }
        }
    }
    if (auto blend_desc = pipeline->desc.blend_desc; blend_desc) {
        setBlendState(*blend_desc, nullptr, 0);
    }

    m_stateCache.topology = gl::Convert(pipeline->desc.primitive_topology);

    glUseProgram(pipeline->programId);
}

void OpenGlRenderDevice::clear(const RenderTargetDesc& p_target) {
    unused(p_target);
    DEV_ASSERT(0);
}

void OpenGlRenderDevice::setViewport(const Viewport& p_viewport) {
    if (p_viewport.topLeftY) {
        LOG_FATAL("TODO: adjust to bottom left y");
    }

    glViewport(p_viewport.topLeftX,
               p_viewport.topLeftY,
               p_viewport.width,
               p_viewport.height);
}

auto OpenGlRenderDevice::createBuffer(const GpuBufferDesc& p_desc) -> Result<std::shared_ptr<GpuBuffer>> {
    auto type = gl::Convert(p_desc.type);

    const GLenum usage = p_desc.dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    GLuint handle = 0;
    glGenBuffers(1, &handle);
    glBindBuffer(type, handle);
    glBufferData(type, p_desc.element_count * p_desc.element_size, p_desc.initial_data, usage);
    glBindBuffer(type, 0);

    auto buffer = MakeRef<OpenGlBuffer>(p_desc);
    buffer->handle = handle;
    buffer->type = type;
    return buffer;
}

auto OpenGlRenderDevice::createMeshImpl(const GpuMeshDesc& mesh_desc,
                                    std::span<const GpuBufferDesc> vb_descs,
                                    const GpuBufferDesc* ib_desc) -> Result<std::shared_ptr<GpuMesh>> {
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

        const uint32_t ebo = ret->indexBuffer->GetHandle32();
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

        const uint32_t vbo = ret->vertexBuffers[slot]->GetHandle32();
        const uint32_t stride_in_byte = ret->desc.vertexLayout[slot].strideInByte;

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

void OpenGlRenderDevice::setMesh(const GpuMesh* p_mesh) {
    if (!p_mesh) {
        if (!m_dummy_vao) {
            glGenVertexArrays(1, &m_dummy_vao);
        }
        glBindVertexArray(m_dummy_vao);
        return;
    }
    auto mesh = reinterpret_cast<const OpenGlGpuMesh*>(p_mesh);
    glBindVertexArray(mesh->vao);
}

void OpenGlRenderDevice::updateBuffer(const GpuBufferDesc& p_desc, GpuBuffer* p_buffer) {
    DEV_ASSERT(p_desc.element_size == p_buffer->desc.element_size);
    if (DEV_VERIFY(p_buffer->desc.element_count >= p_desc.element_count)) {
        const uint32_t size_in_byte = p_desc.element_count * p_desc.element_size;
        const uint32_t vbo = p_buffer->GetHandle32();
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, size_in_byte, p_desc.initial_data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void OpenGlRenderDevice::drawElements(uint32_t p_count, uint32_t p_offset) {
    glDrawElements(m_stateCache.topology, p_count, GL_UNSIGNED_INT, (void*)(p_offset * sizeof(uint32_t)));
}

void OpenGlRenderDevice::drawElementsInstanced(uint32_t p_instance_count, uint32_t p_count, uint32_t p_offset) {
    glDrawElementsInstanced(m_stateCache.topology, p_count, GL_UNSIGNED_INT, (void*)(p_offset * sizeof(uint32_t)), p_instance_count);
}

void OpenGlRenderDevice::drawArrays(uint32_t p_count, uint32_t p_offset) {
    glDrawArrays(m_stateCache.topology, p_offset, p_count);
}

void OpenGlRenderDevice::drawArraysInstanced(uint32_t p_instance_count, uint32_t p_count, uint32_t p_offset) {
    glDrawArraysInstanced(m_stateCache.topology, p_offset, p_count, p_instance_count);
}

void OpenGlRenderDevice::dispatch(uint32_t p_num_groups_x, uint32_t p_num_groups_y, uint32_t p_num_groups_z) {
    unused(p_num_groups_x);
    unused(p_num_groups_y);
    unused(p_num_groups_z);
    CRASH_NOW_MSG("compute shader not supported");
}

void OpenGlRenderDevice::bindUnorderedAccessView(uint32_t p_slot, GpuTexture* p_texture) {
    unused(p_slot);
    unused(p_texture);
    CRASH_NOW_MSG("compute shader not supported");
}

void OpenGlRenderDevice::unbindUnorderedAccessView(uint32_t p_slot) {
    unused(p_slot);
    CRASH_NOW_MSG("compute shader not supported");
}

void OpenGlRenderDevice::bindStructuredBuffer(int slot, const GpuStructuredBuffer* buffer) {
    unused(slot);
    unused(buffer);
    CRASH_NOW_MSG("compute shader not supported");
}

void OpenGlRenderDevice::unbindStructuredBuffer(int slot) {
    unused(slot);
    CRASH_NOW_MSG("compute shader not supported");
}

void OpenGlRenderDevice::bindStructuredBufferSRV(int slot, const GpuStructuredBuffer* buffer) {
    bindStructuredBuffer(slot, buffer);
}

void OpenGlRenderDevice::unbindStructuredBufferSRV(int slot) {
    unbindStructuredBuffer(slot);
}

auto OpenGlRenderDevice::createConstantBuffer(const GpuBufferDesc& p_desc) -> Result<Ref<GpuConstantBuffer>> {
    GLuint handle = 0;

    glGenBuffers(1, &handle);
    if (handle == 0) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "failed to generate buffer");
    }

    glBindBuffer(GL_UNIFORM_BUFFER, handle);
    glBufferData(GL_UNIFORM_BUFFER, p_desc.element_count * p_desc.element_size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, p_desc.slot, handle);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    auto buffer = MakeRef<OpenGlConstantBuffer>(p_desc);
    buffer->handle = handle;
    return buffer;
}

auto OpenGlRenderDevice::createStructuredBuffer(const GpuBufferDesc& p_desc) -> Result<std::shared_ptr<GpuStructuredBuffer>> {
    unused(p_desc);
    CRASH_NOW();
    return nullptr;
}

void OpenGlRenderDevice::updateBufferData(const GpuBufferDesc& p_desc, const GpuStructuredBuffer* p_buffer) {
    unused(p_desc);
    unused(p_buffer);
    CRASH_NOW();
}

void OpenGlRenderDevice::updateConstantBuffer(const GpuConstantBuffer* p_buffer, const void* p_data, size_t p_size) {
    auto buffer = reinterpret_cast<const OpenGlConstantBuffer*>(p_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer->handle);
    glBufferData(GL_UNIFORM_BUFFER, p_size, p_data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGlRenderDevice::bindConstantBufferRange(const GpuConstantBuffer* p_buffer, uint32_t p_size, uint32_t p_offset) {
    auto buffer = reinterpret_cast<const OpenGlConstantBuffer*>(p_buffer);
    DEV_ASSERT(p_size + p_offset <= buffer->capacity);
    glBindBufferRange(GL_UNIFORM_BUFFER, p_buffer->GetSlot(), buffer->handle, p_offset, p_size);
}

void OpenGlRenderDevice::bindTexture(Dimension p_dimension, uint64_t p_handle, int p_slot) {
    if (p_handle == 0) {
        return;
    }

    const GLuint texture_type = gl::ConvertDimension(p_dimension);
    glActiveTexture(GL_TEXTURE0 + p_slot);
    glBindTexture(texture_type, static_cast<GLuint>(p_handle));
}

void OpenGlRenderDevice::unbindTexture(Dimension p_dimension, int p_slot) {
    const GLuint texture_type = gl::ConvertDimension(p_dimension);

    glActiveTexture(GL_TEXTURE0 + p_slot);
    glBindTexture(texture_type, 0);
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
                                  p_texture_desc.initialData));
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
                                      p_texture_desc.initialData));
            }
        } break;
#if !USING(USE_GLES3)
        case GL_TEXTURE_CUBE_MAP_ARRAY: {
            GL_CHECK(glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY,
                                  0,
                                  internal_format,
                                  p_texture_desc.width,
                                  p_texture_desc.height,
                                  p_texture_desc.arraySize,
                                  0, format, data_type, p_texture_desc.initialData));
        } break;
#endif
        case GL_TEXTURE_3D: {
            GL_CHECK(glTexStorage3D(GL_TEXTURE_3D,
                                    p_texture_desc.mipLevels,
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
    if (p_texture_desc.miscFlags & RESOURCE_MISC_GENERATE_MIPS) {
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

void OpenGlRenderDevice::setStencilRef(uint32_t p_ref) {
    glStencilFunc(gl::Convert(m_stateCache.stencilFunc), p_ref, 0xFF);
}

void OpenGlRenderDevice::setBlendState(const BlendDesc& p_desc, const float*, uint32_t) {
    const auto& desc = p_desc.renderTargets[0];
    if (desc.blendEnabled) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
        return;
    }

    const bool r_mask = desc.colorWriteMask & COLOR_WRITE_ENABLE_RED;
    const bool g_mask = desc.colorWriteMask & COLOR_WRITE_ENABLE_GREEN;
    const bool b_mask = desc.colorWriteMask & COLOR_WRITE_ENABLE_BLUE;
    const bool a_mask = desc.colorWriteMask & COLOR_WRITE_ENABLE_ALPHA;

    glColorMask(r_mask, g_mask, b_mask, a_mask);

    auto src_blend = gl::Convert(desc.blendSrc);
    auto dest_blend = gl::Convert(desc.blendDest);
    auto func = gl::Convert(desc.blendOp);

    glBlendEquation(func);
    glBlendFunc(src_blend, dest_blend);
}

void OpenGlRenderDevice::setRenderTargets(const RenderTargetDesc& p_target) {
    GLuint fbo = m_fbo_cache->getOrCreateFbo(p_target);
    DEV_ASSERT(fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    uint32_t flags = 0;
    if (p_target.depth) {
        const DepthAttachmentDesc& desc = *p_target.depth;
        if (desc.depth_load == LoadOp::Clear) {
            flags |= GL_DEPTH_BUFFER_BIT;
            glClearDepthf(desc.clear_depth);  // @TODO: cache
        }
        if (desc.stencil_load == LoadOp::Clear) {
            flags |= GL_STENCIL_BUFFER_BIT;
            glClearStencil(desc.clear_stencil);  // @TODO: cache
        }
    }
    if (!p_target.colors.empty()) {
        const ColorAttachmentDesc& desc = p_target.colors[0];
        if (desc.load == LoadOp::Clear) {
            flags |= GL_COLOR_BUFFER_BIT;
            glClearColor(desc.clear_color[0],
                         desc.clear_color[1],
                         desc.clear_color[2],
                         desc.clear_color[3]);
        }
    }
    if (flags) {
        glClear(flags);
    }
}

void OpenGlRenderDevice::unsetRenderTargets() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

}  // namespace cave::render
