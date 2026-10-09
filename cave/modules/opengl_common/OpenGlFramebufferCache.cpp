#include "OpenGlFramebufferCache.h"

#include "engine/private/render/rhi/RenderTarget.h"

namespace cave::render {

namespace {

void FillTextureView(const TextureViewDesc& view, FboAttachmentKey& key) {
    key.first_slice = view.first_array_slice;
    key.mip = view.mip_slice;
    key.slice_count = view.array_size;
}

FboAttachmentKey MakeColorAttachment(const ColorAttachmentDesc& desc, uint8_t attachment) {
    FboAttachmentKey key{};
    key.tex = static_cast<uint32_t>(desc.tex->GetHandle());
    key.attachment_point = attachment;
    switch (desc.tex->desc.type) {
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
    FillTextureView(desc.view, key);
    return key;
}

FboAttachmentKey MakeDepthAttachment(const DepthAttachmentDesc& desc) {
    FboAttachmentKey key{};
    key.attachment_point = 255;
    key.tex = static_cast<uint32_t>(desc.tex->GetHandle());
    switch (desc.tex->desc.type) {
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
    FillTextureView(desc.view, key);
    return key;
}

FboKey MakeFboKey(const RenderTargetDesc& desc) {
    FboKey key{};
    key.numColors = static_cast<uint8_t>(desc.colors.size());

    for (uint8_t i = 0; i < key.numColors; ++i) {
        key.colors[i] = MakeColorAttachment(desc.colors[i], i);
    }

    if (desc.depth) {
        key.hasDepthStencil = true;
        key.depthStencil = MakeDepthAttachment(*desc.depth);
    }

    return key;
}

}  // namespace

OpenGlFramebufferCache::OpenGlFramebufferCache() noexcept {
}

OpenGlFramebufferCache::~OpenGlFramebufferCache() {
    clear();
}

void OpenGlFramebufferCache::clear() {
    for (auto [_, fbo] : m_fbos) {
        if (fbo != 0) {
            glDeleteFramebuffers(1, &fbo);
        }
    }
    m_fbos.clear();
}

GLuint OpenGlFramebufferCache::getOrCreateFbo(const RenderTargetDesc& desc) {
    const FboKey key = MakeFboKey(desc);

    auto [it, inserted] = m_fbos.try_emplace(key);
    if (inserted) {
        it->second = createFbo(key);
    }

    return it->second;
}

GLuint OpenGlFramebufferCache::createFbo(const FboKey& key) {
    GLuint fbo_handle = 0;

    glGenFramebuffers(1, &fbo_handle);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle);

    GLuint attachments[kMaxColorAttachments]{ 0 };

    for (int idx = 0; idx < key.numColors; ++idx) {
        const FboAttachmentKey& color = key.colors[idx];
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

    if (key.numColors) {
        glDrawBuffers(key.numColors, attachments);
    } else {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    if (key.hasDepthStencil) {
        GLenum attachment = 0;
        switch (key.depthStencil.attachment_point) {
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
        key.depthStencil.attachment_point;
        glFramebufferTexture2D(GL_FRAMEBUFFER,        // target
                               attachment,            // attachment
                               GL_TEXTURE_2D,         // texture target
                               key.depthStencil.tex,  // texture
                               0);                    // level
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        CRASH_NOW();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fbo_handle;
}

}  // namespace cave::render
