#include "engine/private/core/os/os.h"

namespace cave {

class TestLogger : public ILogSink {
public:
    void Submit(const LogEvent& p_log) override {
        m_buffer.append(p_log.message);
    }

    const std::string& GetBuffer() const { return m_buffer; }
    void ClearBuffer() { m_buffer.clear(); }

private:
    std::string m_buffer;
};

TEST(print, PrintImpl) {
    OS dummy_os;
    auto logger = std::make_shared<TestLogger>();
    dummy_os.AddLogger(logger);

    LogImpl(LOG_LEVEL_ERROR, "{}, {}, {}", 1, 'c', "200");
    EXPECT_EQ(logger->GetBuffer(), "1, c, 200");
}

}  // namespace cave
