#include "OpenGlCommandList.h"

#include "OpenGlFramebufferCache.h"
#include "OpenGlHelpers.h"
#include "OpenGlResources.h"
#include "OpenGlPipelineStateManager.h"

namespace cave::render {

void OpenGlCommandList::draw(uint32_t count,
                             uint32_t offset) {
    glDrawArrays(m_stateCache.topology, offset, count);
}

void OpenGlCommandList::drawInstanced(uint32_t instance_count,
                                      uint32_t count,
                                      uint32_t offset) {
    glDrawArraysInstanced(m_stateCache.topology, offset, count, instance_count);
}

void OpenGlCommandList::drawIndexed(uint32_t count,
                                    uint32_t offset) {
    glDrawElements(m_stateCache.topology, count, GL_UNSIGNED_INT, (void*)(offset * sizeof(uint32_t)));
}

void OpenGlCommandList::drawIndexedInstanced(uint32_t instance_count,
                                             uint32_t count,
                                             uint32_t offset) {
    glDrawElementsInstanced(m_stateCache.topology, count, GL_UNSIGNED_INT, (void*)(offset * sizeof(uint32_t)), instance_count);
}

void OpenGlCommandList::beginRendering(const RenderingInfo& info) {
    GLuint fbo = m_fbo_cache.getOrCreateFbo(info);
    DEV_ASSERT(fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    uint32_t flags = 0;
    if (info.depth) {
        const DepthAttachmentDesc& desc = *info.depth;
        if (desc.depth_load == LoadOp::Clear) {
            flags |= GL_DEPTH_BUFFER_BIT;
            glClearDepthf(desc.clear_depth);  // @TODO: cache
        }
        if (desc.stencil_load == LoadOp::Clear) {
            flags |= GL_STENCIL_BUFFER_BIT;
            glClearStencil(desc.clear_stencil);  // @TODO: cache
        }
    }
    if (!info.colors.empty()) {
        const ColorAttachmentDesc& desc = info.colors[0];
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

void OpenGlCommandList::endRendering() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGlCommandList::pushDebugGroup(std::string_view name) {
    unused(name);
#if !USING(PLATFORM_APPLE)
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name.data());
#endif
}

void OpenGlCommandList::popDebugGroup() {
#if !USING(PLATFORM_APPLE)
    glPopDebugGroup();
#endif
}

void OpenGlCommandList::bindConstantBufferRange(const GpuResource* buffer,
                                                uint32_t size,
                                                uint32_t offset) {
    auto gl_buffer = reinterpret_cast<const OpenGlBuffer*>(buffer);
    DEV_ASSERT(size + offset <= gl_buffer->desc.byteSize());
    glBindBufferRange(GL_UNIFORM_BUFFER, buffer->getSlot(), gl_buffer->handle, offset, size);
}

void OpenGlCommandList::setViewport(const Viewport& p_viewport) {
    if (p_viewport.top_left_y) {
        LOG_FATAL("TODO: adjust to bottom left y");
    }

    glViewport(p_viewport.top_left_x,
               p_viewport.top_left_y,
               p_viewport.width,
               p_viewport.height);
}

void OpenGlCommandList::setStencilRef(uint32_t ref) {
    glStencilFunc(gl::Convert(m_stateCache.stencilFunc), ref, 0xFF);
}

void OpenGlCommandList::setBlendState(const BlendDesc& blend_desc, const float*, uint32_t) {
    const auto& desc = blend_desc.renderTargets[0];
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

void OpenGlCommandList::setPipelineState(PipelineStateName name) {
    auto pipeline = reinterpret_cast<OpenGlPipelineState*>(m_pipeline_state_manager.findPSO(name));

    if (pipeline->desc.rasterizer_desc) {
        const auto cull_mode = pipeline->desc.rasterizer_desc->cullMode;
        if (cull_mode != m_stateCache.cullMode) {
            switch (cull_mode) {
                case CullMode::None:
                    glDisable(GL_CULL_FACE);
                    break;
                case CullMode::Front:
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_FRONT);
                    break;
                case CullMode::Back:
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_BACK);
                    break;
                case CullMode::FrontAndBack:
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

void OpenGlCommandList::dispatch(uint32_t num_groups_x,
                                 uint32_t num_groups_y,
                                 uint32_t num_groups_z) {
    glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
    // @TODO: this probably shouldn't be here
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}

void OpenGlCommandList::setMesh(const GpuMesh* p_mesh) {
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

void OpenGlCommandList::bindTexture(Dimension dimension, uint64_t handle, int slot) {
    if (handle) {
        const GLuint texture_type = gl::ConvertDimension(dimension);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(texture_type, static_cast<GLuint>(handle));
    }
}

void OpenGlCommandList::unbindTexture(Dimension dimension, int slot) {
    const GLuint texture_type = gl::ConvertDimension(dimension);

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(texture_type, 0);
}

void OpenGlCommandList::bindStructuredBuffer(int slot, const GpuResource* buffer) {
    auto gl_buffer = reinterpret_cast<const OpenGlBuffer*>(buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl_buffer->handle);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, gl_buffer->handle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void OpenGlCommandList::unbindStructuredBuffer(int slot) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, 0);
}

void OpenGlCommandList::bindStructuredBufferSRV(int slot, const GpuResource* buffer) {
    bindStructuredBuffer(slot, buffer);
}

void OpenGlCommandList::unbindStructuredBufferSRV(int slot) {
    unbindStructuredBuffer(slot);
}

void OpenGlCommandList::bindUnorderedAccessView(uint32_t slot, GpuResource* texture) {
    DEV_ASSERT(texture);
    auto internal_format = gl::ConvertInternalFormat(texture->desc.format);
    glBindImageTexture(slot, texture->getHandle32(), 0, GL_TRUE, 0, GL_READ_WRITE, internal_format);
}

void OpenGlCommandList::unbindUnorderedAccessView(uint32_t slot) {
    glBindImageTexture(slot, 0, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R11F_G11F_B10F);
}

}  // namespace cave::render
