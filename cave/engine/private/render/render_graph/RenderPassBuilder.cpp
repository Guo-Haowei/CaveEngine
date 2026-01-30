#include "RenderPassBuilder.h"
#include "RenderGraphBuilder.h"

namespace cave::render {

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
