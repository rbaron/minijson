#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "minijson.h"

// Internal functions tests.
namespace minijson::internal {
namespace {

/*
 * Encoding tests
 */

TEST(EscapedUTF16ToCodePoint, ParseBaseMultilingualPlaneCodeUnit) {
  // é
  const std::string input("\\u00e9");
  BoundIterator it(input.begin(), input.end());
  ++it;
  ASSERT_EQ(EscapedUTF16ToCodepoint(&it), 0xe9);
}

TEST(EscapedUTD16ToCodePoint, ParseSupplementaryPlanesCodeUnit) {
  // 𝄞
  const std::string input("\\uD834\\uDD1E");
  BoundIterator it(input.begin(), input.end());
  ++it;
  ASSERT_EQ(EscapedUTF16ToCodepoint(&it), 0x1d11e);
}

TEST(EscapedUTD16ToCodePoint, ThrowOnWrongEscapeSymbol) {
  // 𝄞
  const std::string input("\\bD834");
  BoundIterator it(input.begin(), input.end());
  ++it;
  ASSERT_THROW(EscapedUTF16ToCodepoint(&it), std::runtime_error);
}

TEST(EscapedUTD16ToCodePoint, ThrowOnMissingDigits) {
  // Missing the 4th digit
  const std::string input("\\bD83");
  BoundIterator it(input.begin(), input.end());
  ++it;
  ASSERT_THROW(EscapedUTF16ToCodepoint(&it), std::runtime_error);
}

struct CodePointToUTF8TestParam {
  unsigned long int code;
  std::string bytes;
};

class CodePointToUTF8Test
    : public ::testing::TestWithParam<CodePointToUTF8TestParam> {};

TEST_P(CodePointToUTF8Test, ParseBaseMultilingualPlaneChar) {
  const CodePointToUTF8TestParam &param = GetParam();
  ASSERT_EQ(CodePointToUTF8(param.code), param.bytes);
}

INSTANTIATE_TEST_SUITE_P(
    CodePointToUTF8Test, CodePointToUTF8Test,
    ::testing::Values(
        // 💩
        CodePointToUTF8TestParam{0x1f4a9, "\xf0\x9f\x92\xa9"},
        // 𝄞
        CodePointToUTF8TestParam{0x1d11e, "\xf0\x9d\x84\x9e"},
        // 뻯
        CodePointToUTF8TestParam{0xbeef, "\xeb\xbb\xaf"},
        // é
        CodePointToUTF8TestParam{0xe9, "\xc3\xa9"},
        // a
        CodePointToUTF8TestParam{0x61, "\x61"}));

/*
 * Decoding tests
 */

struct UTF8ToCodePointTestParam {
  unsigned long int code;
  std::string bytes;
};

class UTF8ToCodePointTest
    : public ::testing::TestWithParam<UTF8ToCodePointTestParam> {};

TEST_P(UTF8ToCodePointTest, DecodeUTF8ByteSequence) {
  const UTF8ToCodePointTestParam &param = GetParam();
  BoundIterator it(param.bytes.begin(), param.bytes.end());
  ASSERT_EQ(UTF8ToCodePoint(&it), param.code);
  ASSERT_TRUE(it.end());
}

INSTANTIATE_TEST_SUITE_P(
    UTF8ToCodePointTest, UTF8ToCodePointTest,
    ::testing::Values(
        // 💩
        UTF8ToCodePointTestParam{0x1f4a9, "\xf0\x9f\x92\xa9"},
        // 𝄞
        UTF8ToCodePointTestParam{0x1d11e, "\xf0\x9d\x84\x9e"},
        // 뻯
        UTF8ToCodePointTestParam{0xbeef, "\xeb\xbb\xaf"},
        // é
        UTF8ToCodePointTestParam{0xe9, "\xc3\xa9"},
        // a
        UTF8ToCodePointTestParam{0x61, "\x61"}));
// ));

TEST(UTF8ToCodePointTest, ThrowOnMissingDigits) {
  // We'd expect to see two bytes here.
  const std::string input("\xc3");
  BoundIterator it(input.begin(), input.end());
  ASSERT_THROW(UTF8ToCodePoint(&it), std::runtime_error);
}
} // namespace
} // namespace minijson::internal
