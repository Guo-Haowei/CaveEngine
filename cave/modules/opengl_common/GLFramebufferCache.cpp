#include "GLFramebufferCache.h"

#include "engine/private/render/rhi/RenderTarget.h"

namespace cave::render {

static FboKey MakeFboKey(const RenderTargetDesc& p_desc);

GLFramebufferCache::GLFramebufferCache() noexcept {
}

GLFramebufferCache::~GLFramebufferCache() {
    Clear();
}

void GLFramebufferCache::Clear() {
    for (auto [_, fbo] : m_fbos)
        if (fbo != 0)
            glDeleteFramebuffers(1, &fbo);
    m_fbos.clear();

    ResetStats();
}

GLuint GLFramebufferCache::GetOrCreateFbo(const RenderTargetDesc& p_desc) {
    const FboKey key = MakeFboKey(p_desc);

    auto [it, inserted] = m_fbos.try_emplace(key);
    if (inserted) {
        it->second = CreateFbo(key);
        ++m_stats.fbo_misses;
        LOG_WARN("fbo cache miss");
    } else {
        ++m_stats.fbo_hits;
    }

    return it->second;
}

GLFramebufferCache::Stats GLFramebufferCache::GetStats() const {
    m_stats.fbo_count = static_cast<uint32_t>(m_fbos.size());
    return m_stats;
}

GLuint GLFramebufferCache::CreateFbo(const FboKey& p_key) {
    GLuint fbo_handle = 0;

    glGenFramebuffers(1, &fbo_handle);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle);

    GLuint attachments[kMaxColorAttachments]{ 0 };

    for (int idx = 0; idx < p_key.numColors; ++idx) {
        const FboAttachmentKey& color = p_key.colors[idx];
        DEV_ASSERT(color.attachment_point < kMaxColorAttachments);
        GLuint attachment = GL_COLOR_ATTACHMENT0;
        GLenum type = GL_TEXTURE_2D;
        switch (color.kind) {
            case AttachKind::Tex2D: {
                attachment = GL_COLOR_ATTACHMENT0 + idx;
            } break;
            case AttachKind::CubeFace: {
                type = GL_TEXTURE_CUBE_MAP_POSITIVE_X + color.first_slice;
            } break;
            default:
                break;
        }
        attachments[idx] = attachment;
        glFramebufferTexture2D(GL_FRAMEBUFFER,  // target
                               attachment,      // attachment
                               type,            // texture target
                               color.tex,       // texture
                               color.mip        // level
        );
    }

    if (p_key.numColors) {
        glDrawBuffers(p_key.numColors, attachments);
    } else {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    if (p_key.hasDepthStencil) {
        GLenum attachment = 0;
        switch (p_key.depthStencil.attachment_point) {
            case FboAttachmentKey::Depth: {
                attachment = GL_DEPTH_ATTACHMENT;
            } break;
            case FboAttachmentKey::DepthStencil: {
                attachment = GL_DEPTH_STENCIL_ATTACHMENT;
            } break;
            default: {
                CRASH_NOW();
            } break;
        }
        p_key.depthStencil.attachment_point;
        glFramebufferTexture2D(GL_FRAMEBUFFER,          // target
                               attachment,              // attachment
                               GL_TEXTURE_2D,           // texture target
                               p_key.depthStencil.tex,  // texture
                               0);                      // level
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        CRASH_NOW();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fbo_handle;
}

static void FillTextureView(const TextureViewDesc& p_view, FboAttachmentKey& p_key) {
    p_key.first_slice = p_view.first_array_slice;
    p_key.mip = p_view.mip_slice;
    p_key.slice_count = p_view.array_size;
}

static FboAttachmentKey MakeColorAttachment(const ColorAttachmentDesc& p_desc, uint8_t p_attachment) {
    FboAttachmentKey key{};
    key.tex = static_cast<uint32_t>(p_desc.tex->GetHandle());
    key.attachment_point = p_attachment;
    switch (p_desc.tex->desc.type) {
        case AttachmentType::COLOR_2D: {
            key.kind = AttachKind::Tex2D;
        } break;
        case AttachmentType::COLOR_CUBE: {
            key.kind = AttachKind::CubeFace;
        } break;
        default: {
            CRASH_NOW();
        } break;
    }
    FillTextureView(p_desc.view, key);
    return key;
}

static FboAttachmentKey MakeDepthAttachment(const DepthAttachmentDesc& p_desc) {
    FboAttachmentKey key{};
    key.attachment_point = 255;
    key.tex = static_cast<uint32_t>(p_desc.tex->GetHandle());
    switch (p_desc.tex->desc.type) {
        case AttachmentType::DEPTH_STENCIL_2D: {
            key.kind = AttachKind::Tex2D;
            key.attachment_point = 254;
        } break;
        case AttachmentType::DEPTH_2D:
        case AttachmentType::SHADOW_2D: {
            key.kind = AttachKind::Tex2D;
        } break;
        case AttachmentType::SHADOW_CUBE_ARRAY: {
            key.kind = AttachKind::CubeFace;
            CRASH_NOW();
        } break;
        default: {
            CRASH_NOW();
        } break;
    }
    FillTextureView(p_desc.view, key);
    return key;
}

static FboKey MakeFboKey(const RenderTargetDesc& p_desc) {
    FboKey key{};
    key.numColors = static_cast<uint8_t>(p_desc.colors.size());

    for (uint8_t i = 0; i < key.numColors; ++i) {
        key.colors[i] = MakeColorAttachment(p_desc.colors[i], i);
    }

    if (p_desc.depth) {
        key.hasDepthStencil = true;
        key.depthStencil = MakeDepthAttachment(*p_desc.depth);
    }

    return key;
}

}  // namespace cave::render
