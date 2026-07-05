#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/intent/IntentBus.h"

namespace cave {

using namespace ::cave::literals;

class TestIntentHandler : public IIntentHandler {
public:
    TestIntentHandler()
        : m_debug_id(MakeDebugId(this)) {}

    bool handleIntent(Intent&) override { return true; }

    DebugId debugId() const {
        return m_debug_id;
    }

private:
    const DebugId m_debug_id;
};

TEST(IntentBus, can_only_add_intent_handler_once_per_intent) {
    IntentBus dispatcher;
    TestIntentHandler handler;

    bool ok = dispatcher.addHandler("test"_sid, &handler);
    EXPECT_TRUE(ok);
    ok = dispatcher.addHandler("test"_sid, &handler);
    EXPECT_FALSE(ok);
    ok = dispatcher.removeHandler("test"_sid, &handler);
    EXPECT_TRUE(ok);
    ok = dispatcher.removeHandler("test"_sid, &handler);
    EXPECT_FALSE(ok);
}

TEST(IntentBus, can_add_different_handlers_to_same_intent) {
    IntentBus dispatcher;
    TestIntentHandler handler1;
    TestIntentHandler handler2;

    bool ok = dispatcher.addHandler("test"_sid, &handler1);
    EXPECT_TRUE(ok);
    ok = dispatcher.addHandler("test"_sid, &handler2);
    EXPECT_TRUE(ok);
    ok = dispatcher.removeHandler("test"_sid, &handler1);
    EXPECT_TRUE(ok);
    ok = dispatcher.removeHandler("test"_sid, &handler2);
    EXPECT_TRUE(ok);
}

}  // namespace cave
