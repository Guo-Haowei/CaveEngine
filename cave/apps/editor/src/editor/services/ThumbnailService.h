#pragma once
#include "editor/thumbnail/ThumbnailKey.h"

namespace cave {

class EditorState;

class ThumbnailService {
public:
    explicit ThumbnailService(EditorState& p_editor) noexcept;

    uint64_t GetOrRequest(const ThumbnailKey& p_guid);

    void BeginFrame();

    void EndFrame();  // call this after rendering, mark cache ready

    void Invalidate(const Guid& p_guid);

private:
    EditorState& m_editor;

    std::list<ThumbnailKey> m_pending;
    std::list<ThumbnailKey> m_inflight;
};

}  // namespace cave
