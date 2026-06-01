#include "cave/runtime/intent/IntentDispatcher.h"

#include "engine/private/core/diagnostics/DebugIdAllocator.h"

namespace cave {

class TestIntentHandler : public IIntentHandler {
public:
    TestIntentHandler()
        : m_debug_id(MakeDebugId(this)) {}

    void HandleIntent(const Intent&) {}

    DebugId GetDebugId() const {
        return m_debug_id;
    }

private:
    const DebugId m_debug_id;
};

TEST(IntentDispatcher, can_only_add_intent_handler_once_per_intent) {
    IntentDispatcher dispatcher;
    TestIntentHandler handler;

    bool ok = dispatcher.AddHandler(StringId("test"), &handler);
    EXPECT_TRUE(ok);
    ok = dispatcher.AddHandler(StringId("test"), &handler);
    EXPECT_FALSE(ok);
    ok = dispatcher.RemoveHandler(StringId("test"), &handler);
    EXPECT_TRUE(ok);
    ok = dispatcher.RemoveHandler(StringId("test"), &handler);
    EXPECT_FALSE(ok);
}

}  // namespace cave
