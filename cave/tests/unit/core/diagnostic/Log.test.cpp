#include "cave/core/diagnostics/ILogSink.h"

namespace cave {

class TestLogSink final : public ILogSink {
public:
    void submit(const LogEvent& log) override {
        logs.push_back(log);
    }

    std::vector<LogEvent> logs;
};

class LogTest : public ::testing::Test {
protected:
    void SetUp() override {
        cave::SetLogger(&sink_);
    }

    void TearDown() override {
        cave::SetLogger(nullptr);
    }

    TestLogSink sink_;
};

TEST(ToString, ConvertsLogLevel) {
    EXPECT_STREQ(ToString(LOG_LEVEL_TRACE), "TRACE");
    EXPECT_STREQ(ToString(LOG_LEVEL_INFO), "INFO ");
    EXPECT_STREQ(ToString(LOG_LEVEL_OK), "OK   ");
    EXPECT_STREQ(ToString(LOG_LEVEL_WARN), "WARN ");
    EXPECT_STREQ(ToString(LOG_LEVEL_ERROR), "ERROR");
    EXPECT_STREQ(ToString(LOG_LEVEL_FATAL), "FATAL");
}

TEST(FormatLog, ContainsExpectedFields) {
    cave::LogEvent log{};
    log.level = cave::LOG_LEVEL_WARN;
    log.channel = cave::LogChannel::Default;
    log.repeat = 1;
    log.timestamp_ms = 123;
    std::snprintf(log.time_str, sizeof(log.time_str), "12:34:56.78");
    log.message = "formatted message";

    const std::string text = cave::FormatLog(log);

    EXPECT_NE(text.find("12:34:56.78"), std::string::npos);
    EXPECT_NE(text.find("WARN"), std::string::npos);
    EXPECT_NE(text.find("formatted message"), std::string::npos);
}

TEST_F(LogTest, LogInfoSubmitsEventToSink) {
    LOG_INFO("hello log");

    ASSERT_EQ(sink_.logs.size(), 1u);

    const cave::LogEvent& log = sink_.logs[0];
    EXPECT_EQ(log.level, cave::LOG_LEVEL_INFO);
    EXPECT_EQ(log.channel, cave::LogChannel::Default);
    EXPECT_EQ(log.message, "hello log");
}

TEST_F(LogTest, LogFormatArgumentsAreApplied) {
    LOG_WARN("value = {}, name = {}", 42, "chess");

    ASSERT_EQ(sink_.logs.size(), 1u);

    const cave::LogEvent& log = sink_.logs[0];
    EXPECT_EQ(log.level, cave::LOG_LEVEL_WARN);
    EXPECT_EQ(log.channel, cave::LogChannel::Default);
    EXPECT_EQ(log.message, "value = 42, name = chess");
}

TEST_F(LogTest, LogWithExplicitChannel) {
    LOG_ERROR(cave::LogChannel::Default, "explicit channel");

    ASSERT_EQ(sink_.logs.size(), 1u);

    const cave::LogEvent& log = sink_.logs[0];
    EXPECT_EQ(log.level, cave::LOG_LEVEL_ERROR);
    EXPECT_EQ(log.channel, cave::LogChannel::Default);
    EXPECT_EQ(log.message, "explicit channel");
}

TEST_F(LogTest, SetLoggerToNullDropsLogs) {
    cave::SetLogger(nullptr);

    LOG_INFO("this should be ignored");

    EXPECT_TRUE(sink_.logs.empty());
}

}  // namespace cave
