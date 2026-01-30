#include "RenderPassBuilder.h"
#include "RenderGraphBuilder.h"

namespace cave::render {

RGTextureHandle RenderPassBuilder::Create(RGResourceCreateDesc&& p_desc) {
    RGTextureHandle handle = m_builder.CreateTexture(std::move(p_desc));
    m_creates.emplace_back(handle);
    return handle;
}

RGTextureHandle RenderPassBuilder::Import(RGResourceImportDesc&& p_desc) {
    RGTextureHandle handle = m_builder.ImportTexture(std::move(p_desc));
    m_creates.emplace_back(handle);
    return handle;
}

RenderPassBuilder& RenderPassBuilder::Read(ResourceAccess p_access, RGTextureHandle p_handle) {
    m_reads.emplace_back(Resource{ p_handle, p_access });
    return *this;
}

RenderPassBuilder& RenderPassBuilder::Write(ResourceAccess p_access, RGTextureHandle p_handle) {
    // ignore DSV write?
    m_writes.emplace_back(Resource{ p_handle, p_access });
    return *this;
}

RenderPassBuilder& RenderPassBuilder::SetExecuteFunc(ExecuteFunc p_func) {
    m_func = p_func;
    return *this;
}

}  // namespace cave::render
