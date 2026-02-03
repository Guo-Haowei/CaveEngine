#pragma once
#include "editor/panels/Tab.h"

namespace cave {

class EditorState;

struct PickRequest {
    math::Vector2f cursor;  // cursor in window space
    math::Vector2f pos;     // viewport position in screen space
    math::Vector2f size;    // viewport size
};

struct PickContext {
    
};

class PickingService {
public:
    PickingService(EditorState& p_editor) noexcept;

    void Submit(PickRequest p_req);

    void Tick();

private:
    EditorState& m_editor;

    std::vector<PickRequest> m_requests;
};

}  // namespace cave
