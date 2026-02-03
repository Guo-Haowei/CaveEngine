#pragma once
#include "IPickConsumer.h"

namespace cave {

class EditorState;

struct PickRequest {
    math::Vector2f cursor;  // cursor in window space
};

class PickingService {
public:
    PickingService(EditorState& p_editor) noexcept;

    void Submit(PickRequest p_req);

    void Tick();

    void Register(IPickConsumer* p_consumer);
    void Unregister(IPickConsumer* p_consumer);

private:
    void Raycast(const PickData& data);

    EditorState& m_editor;

    Option<PickRequest> m_request;
    std::vector<IPickConsumer*> m_consumers;
};

}  // namespace cave
