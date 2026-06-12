#include "cave/core/diagnostics/ILogSink.h"
#include "cave/core/ErrorMacros.h"

namespace cave {

class ErrorMacrosTest : public ::testing::Test {
public:
    static void handler1(void*, std::string_view function, std::string_view file_path, int line,
                         std::string_view error) {
        s_buffer.append(function);
        s_buffer.push_back(',');
        s_buffer.append(file_path);
        s_buffer.push_back(',');
        s_buffer.push_back('a' + static_cast<char>(line));
        s_buffer.push_back(',');
        s_buffer.append(error);
        s_buffer.push_back(';');
    }

    static void handler2(void*, std::string_view, std::string_view, int, std::string_view) {
        s_buffer.append("?;");
    }

    static void handler3(void*,
                         std::string_view function,
                         std::string_view file_path,
                         int line,
                         std::string_view error) {
        s_buffer.append(error);
        s_buffer.push_back(',');
        s_buffer.push_back('a' + static_cast<char>(line));
        s_buffer.push_back(',');
        s_buffer.append(file_path);
        s_buffer.push_back(',');
        s_buffer.append(function);
        s_buffer.push_back(';');
    }

    static void clearBuffer() { s_buffer.clear(); }

    static void assertHandler(void*,
                              std::string_view,
                              std::string_view,
                              int,
                              std::string_view) { exit(99); }

    static const std::string& getBuffer() { return s_buffer; }

    void SetUp() override {
        handler1_.data.errorFunc = handler1;
        cave::AddErrorHandler(&handler1_);
    }

    void TearDown() override {
        cave::RemoveErrorHandler(&handler1_);
        clearBuffer();
    }

protected:
    static std::string s_buffer;

    ErrorHandler handler1_;
};

std::string ErrorMacrosTest::s_buffer;

TEST_F(ErrorMacrosTest, AddErrorHandler) {
    ReportErrorImpl("a", "b", 2, "d");

    EXPECT_EQ(ErrorMacrosTest::getBuffer(), "a,b,c,d;");
}

TEST_F(ErrorMacrosTest, RemoveErrorHandler) {
    ErrorHandler handler2;
    handler2.data.errorFunc = ErrorMacrosTest::handler2;

    ErrorHandler handler3;
    handler3.data.errorFunc = ErrorMacrosTest::handler3;

    AddErrorHandler(&handler2);
    AddErrorHandler(&handler3);

    ReportErrorImpl("a", "b", 2, "d");
    EXPECT_EQ(ErrorMacrosTest::getBuffer(), "d,c,b,a;?;a,b,c,d;");
    ErrorMacrosTest::clearBuffer();

    RemoveErrorHandler(&handler2);

    ReportErrorImpl("b", "c", 3, "e");
    EXPECT_EQ(ErrorMacrosTest::getBuffer(), "e,d,c,b;b,c,d,e;");
    ErrorMacrosTest::clearBuffer();

    RemoveErrorHandler(&handler3);
    RemoveErrorHandler(&handler2);
}

TEST_F(ErrorMacrosTest, DEV_VERIFY_check_pass) {
    int a = 1;
    if (DEV_VERIFY_CHECK(a == 1)) {
        SUCCEED();
    } else {
        FAIL();
    }
}

TEST_F(ErrorMacrosTest, DEV_VERIFY_no_check_pass) {

    int a = 1;
    if (DEV_VERIFY_NO_CHECK(a == 1)) {
        SUCCEED();
    } else {
        FAIL();
    }
}

TEST_F(ErrorMacrosTest, DEV_VERIFY_no_check_fail) {
    int a = 1;
    if (DEV_VERIFY_NO_CHECK(a == 2)) {
        FAIL();
    } else {
        SUCCEED();
    }
}

TEST(ErrorMacros, ERR_FAIL) {
    auto func = []() {
        ERR_FAIL();
    };

    testing::internal::CaptureStdout();
    func();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.starts_with(
        R"(ERROR: Method/function failed.
    at)"))
        << output;
}

TEST(ErrorMacros, ERR_FAIL_MSG) {
    auto func = []() { ERR_FAIL_MSG("Bluh!"); };

    testing::internal::CaptureStdout();
    func();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.starts_with(
        R"(ERROR: Method/function failed.
Detail: Bluh!
    at)"))
        << output;
}

TEST(ErrorMacros, ERR_FAIL_V) {
    auto func = []() { ERR_FAIL_V(10); };

    testing::internal::CaptureStdout();
    func();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.starts_with(
        R"(ERROR: Method/function failed. Returning: 10
    at)"))
        << output;
}

TEST(ErrorMacros, ERR_FAIL_V_MSG) {
    auto func = []() { ERR_FAIL_V_MSG(nullptr, "Bruh..."); };

    testing::internal::CaptureStdout();
    func();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.starts_with(
        R"(ERROR: Method/function failed. Returning: nullptr
Detail: Bruh...
    at)"))
        << output;
}

TEST(ErrorMacros, ERR_FAIL_COND) {
    int a = 1;
    auto func = [&]() { ERR_FAIL_COND(a > 0); };

    testing::internal::CaptureStdout();
    func();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.starts_with(
        "ERROR: Condition \"a > 0\" is true."))
        << output;
}

TEST(ErrorMacros, ERR_FAIL_COND_MSG) {
    int a = 1;
    auto func = [&]() { ERR_FAIL_COND_MSG(a >= 0, "invalid buffer size"); };

    testing::internal::CaptureStdout();
    func();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.starts_with(
        R"(ERROR: Condition "a >= 0" is true.
Detail: invalid buffer size
    at)"))
        << output;
}

TEST(ErrorMacros, ERR_FAIL_COND_V) {
    auto func = [&]() {
        int a = 1;
        ERR_FAIL_COND_V(a >= 0, 1);
        return 2;
    };

    testing::internal::CaptureStdout();
    func();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.starts_with(
        R"(ERROR: Condition "a >= 0" is true. Returning: 1
    at)"))
        << output;
}

TEST(ErrorMacros, ERR_FAIL_COND_V_MSG) {
    int a = 1;
    auto func = [&]() {
        ERR_FAIL_COND_V_MSG(a >= 0, 1, "???");
        return 0;
    };

    testing::internal::CaptureStdout();
    func();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.starts_with(
        R"(ERROR: Condition "a >= 0" is true. Returning: 1
Detail: ???
    at)"))
        << output;
}

TEST(ErrorMacros, ERR_FAIL_INDEX) {
    auto func = [&]() {
        int a = 1;
        int b = 1;
        ERR_FAIL_INDEX(a, b);
    };

    testing::internal::CaptureStdout();
    func();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.starts_with(
        R"(ERROR: Index a = 1 is out of bounds (b = 1).
    at)"))
        << output;
}

TEST(ErrorMacros, ERR_FAIL_INDEX_MSG) {
    auto func = [&]() {
        int a = 1;
        int b = 1;
        ERR_FAIL_INDEX_MSG(a, b, "?????");
    };

    testing::internal::CaptureStdout();
    func();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.starts_with(
        R"(ERROR: Index a = 1 is out of bounds (b = 1).
Detail: ?????
    at)"))
        << output;
}

TEST(ErrorMacros, ERR_FAIL_INDEX_V) {
    auto func = [&]() {
        int a = 1;
        int b = 1;
        ERR_FAIL_INDEX_V(a, b, 10);
        return 20;
    };

    testing::internal::CaptureStdout();
    func();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.starts_with(
        R"(ERROR: Index a = 1 is out of bounds (b = 1).
    at)"))
        << output;
}

TEST(ErrorMacros, ERR_FAIL_INDEX_V_MSG) {
    auto func = [&]() {
        int a = 1;
        int b = 1;
        ERR_FAIL_INDEX_V_MSG(a, b, 10, "????");
        return 20;
    };

    testing::internal::CaptureStdout();
    func();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.starts_with(
        R"(ERROR: Index a = 1 is out of bounds (b = 1).
Detail: ????
    at)"))
        << output;
}

}  // namespace cave
