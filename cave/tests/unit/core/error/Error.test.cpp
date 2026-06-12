#include <array>
#include <regex>

#include <gtest/gtest.h>

#include "cave/core/error/Result.h"

using namespace cave;

static Result<void> DoStuffImpl() {
    return CAVE_ERROR(ErrorCode::ERR_BUG, "???");
}

static Result<void> DoStuffWrapper() {
    auto res = DoStuffImpl();
    if (!res) {
        return CAVE_ERROR(res.error());
    }
    return {};
}

static Result<void> MyFunc() {
    auto res = DoStuffWrapper();
    if (!res) {
        return CAVE_ERROR(res.error());
    }
    return {};
}

static void ExpectFunctionOrderInString(const std::string& text,
                                        std::span<const std::string_view> expected_functions) {
    std::regex word_regex("(DoStuffImpl|DoStuffWrapper|MyFunc)");
    auto words_begin = std::sregex_iterator(text.begin(), text.end(), word_regex);
    auto words_end = std::sregex_iterator();

    size_t index = 0;
    for (std::sregex_iterator it = words_begin; it != words_end; ++it) {
        ASSERT_LT(index, expected_functions.size());

        const std::smatch match = *it;
        EXPECT_EQ(match.str(), expected_functions[index]);

        ++index;
    }

    EXPECT_EQ(index, expected_functions.size());
}

namespace cave {

TEST(InternalError, ConstructorNoString) {
    // clang-format off
    int line = __LINE__; auto err = CAVE_ERROR(ErrorCode::OK).error();
    // clang-format on

    EXPECT_EQ(err.value(), ErrorCode::OK);
    EXPECT_EQ(err.message(), "");

    const auto& frames = err.frames();
    ASSERT_EQ(frames.size(), 1u);
    EXPECT_EQ(frames[0].line, line);
}

TEST(InternalError, ConstructorWithFormattedString) {
    auto err = CAVE_ERROR(ErrorCode::ERR_ALREADY_EXISTS, "({}={}={})", 1, 2, 3).error();

    EXPECT_EQ(err.value(), ErrorCode::ERR_ALREADY_EXISTS);
    EXPECT_EQ(err.message(), "(1=2=3)");

    const auto& frames = err.frames();
    ASSERT_EQ(frames.size(), 1u);
}

TEST(InternalError, ConstructorWithLongFormat) {
    auto err = CAVE_ERROR(ErrorCode::ERR_ALREADY_EXISTS,
                          "({}={}={}={}={}={})",
                          1,
                          -2,
                          3,
                          5.5f,
                          'c',
                          "def")
                   .error();

    EXPECT_EQ(err.value(), ErrorCode::ERR_ALREADY_EXISTS);
    EXPECT_EQ(err.message(), "(1=-2=3=5.5=c=def)");

    const auto& frames = err.frames();
    ASSERT_EQ(frames.size(), 1u);
}

TEST(InternalError, ErrorStackFramesAreRecordedInOriginToCallerOrder) {
    auto res = MyFunc();

    ASSERT_FALSE(res.has_value());

    const Error& err = res.error();

    EXPECT_EQ(err.value(), ErrorCode::ERR_BUG);
    EXPECT_EQ(err.message(), "???");

    const auto& frames = err.frames();

    ASSERT_EQ(frames.size(), 3u);

    EXPECT_STREQ(frames[0].func.data(), "DoStuffImpl");
    EXPECT_STREQ(frames[1].func.data(), "DoStuffWrapper");
    EXPECT_STREQ(frames[2].func.data(), "MyFunc");

    EXPECT_GT(frames[0].line, 0);
    EXPECT_GT(frames[1].line, 0);
    EXPECT_GT(frames[2].line, 0);
}

TEST(InternalError, ErrorStackStringContainsFunctionOrder) {
    auto res = MyFunc();

    ASSERT_FALSE(res.has_value());

    const std::string text = ToString(res.error());

    EXPECT_NE(text.find("ERR_BUG"), std::string::npos);
    EXPECT_NE(text.find("???"), std::string::npos);

    constexpr std::array<std::string_view, 3> expected_functions = {
        "DoStuffImpl",
        "DoStuffWrapper",
        "MyFunc",
    };

    ExpectFunctionOrderInString(text, expected_functions);
}

TEST(InternalError, WrappingErrorAppendsOneFrame) {
    auto inner = DoStuffImpl();

    ASSERT_FALSE(inner.has_value());
    ASSERT_EQ(inner.error().frames().size(), 1u);

    auto wrapped = CAVE_ERROR(inner.error()).error();

    EXPECT_EQ(wrapped.value(), ErrorCode::ERR_BUG);
    EXPECT_EQ(wrapped.message(), "???");

    const auto& frames = wrapped.frames();
    ASSERT_EQ(frames.size(), 2u);

    EXPECT_STREQ(frames[0].func.data(), "DoStuffImpl");
}

}  // namespace cave
