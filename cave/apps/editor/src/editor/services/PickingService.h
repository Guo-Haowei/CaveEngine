#pragma once
#include "editor/panels/Tab.h"

namespace cave {

class EditorState;

struct PickRequest {
    TabId tab_id;
    float x_view_px;
    float y_view_px;
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
