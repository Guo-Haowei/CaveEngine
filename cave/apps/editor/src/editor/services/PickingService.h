#pragma once
#include "cave/runtime/intent/IIntentHandler.h"

#include "IPickConsumer.h"

namespace cave {

class EditorState;

class PickingService final : public IIntentHandler {
public:
    PickingService(EditorState& p_editor);
    ~PickingService();

    void Pick(math::Vector2f p_point_win);

    void Register(IPickConsumer* p_consumer);
    void Unregister(IPickConsumer* p_consumer);

    bool handleIntent(Intent& p_intent) override;

    DebugId debugId() const override { return m_debug_id; }

private:
    void Raycast(const PickData& data);

    EditorState& m_editor;

    std::vector<IPickConsumer*> m_consumers;

    const DebugId m_debug_id;
};

}  // namespace cave
