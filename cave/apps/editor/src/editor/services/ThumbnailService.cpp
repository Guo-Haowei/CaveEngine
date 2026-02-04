#include "ThumbnailService.h"

namespace cave {

ThumbnailService::ThumbnailService(EditorState& p_editor) noexcept
    : m_editor(p_editor) {
}

uint64_t ThumbnailService::GetOrRequest(const ThumbnailKey& p_guid) {
    unused(p_guid);
    return 0;
}

void ThumbnailService::BeginFrame() {
}

void ThumbnailService::EndFrame() {
}

void ThumbnailService::Invalidate(const Guid& p_guid) {
    unused(p_guid);
}

}  // namespace cave
