
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "minijson.h"

// Internal functions tests.
namespace minijson::internal {
namespace {

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

} // namespace
} // namespace minijson::internal

// Public interface tests.
namespace minijson {
namespace {

TEST(Parser, ParseString) {
  const std::string text = R"(
    {
      "key": "hello, world"
    }
  )";
  JSONNode json = Parse(text);
  ASSERT_EQ(json["key"].GetStr(), "hello, world");
}

TEST(Parser, ParseNumber) {
  const std::string text = R"(
    {
      "ok": 123
    }
  )";
  JSONNode json = Parse(text);
  ASSERT_DOUBLE_EQ(json["ok"].GetNum(), 123);
}

TEST(Parser, ParseBoolean) {
  const std::string text = R"(
    {
      "true_key": true,
      "false_key": false
    }
  )";
  JSONNode json = Parse(text);
  ASSERT_TRUE(json["true_key"].GetBool());
  ASSERT_FALSE(json["false_key"].GetBool());
}

TEST(Parser, ParseNull) {
  const std::string text = R"(
    {
      "null_key": null,
      "str_key": "hello, world"
    }
  )";
  JSONNode json = Parse(text);
  ASSERT_TRUE(json["null_key"].IsNull());
  ASSERT_FALSE(json["str_key"].IsNull());
}

TEST(Parser, ParseArray) {
  const std::string text = R"(
    {
      "key": [
        "hello, world",
        {
          "nested": [1, 2]
        }
      ]
    }
  )";
  JSONNode json = Parse(text);
  ASSERT_EQ(json["key"][0].GetStr(), "hello, world");
  ASSERT_DOUBLE_EQ(json["key"][1]["nested"][1].GetNum(), 2);
}

TEST(Parser, ParseNestedDoc) {
  const std::string text = R"(
    {
      "ok": 123,
      "nested": {
        "nested_1": "abc",
        "nested_2": {
          "nested_2_1": "ok!"
        }
      }
    }
  )";
  JSONNode json = Parse(text);
  ASSERT_EQ(json["nested"]["nested_1"].GetStr(), "abc");
  ASSERT_EQ(json["nested"]["nested_2"]["nested_2_1"].GetStr(), "ok!");
}

TEST(Parser, ComplainAboutMissingComma) {
  const std::string text = R"(
    {
      "ok": 123
      "key2": 321
    }
  )";
  ASSERT_THROW(Parse(text), std::runtime_error);
}

TEST(Parser, ThrowOnGetWrongType) {
  const std::string text = R"(
    {
      "ok": 123
    }
  )";
  JSONNode json = Parse(text);
  ASSERT_THROW(json.GetStr(), std::runtime_error);
}

TEST(Parser, ThrowOnMalformedNumber) {
  const std::string text = R"(
    {
      "ok": 123.123.31
    }
  )";
  ASSERT_THROW(Parse(text), std::runtime_error);
}

TEST(Parser, ThrowOnUnterminatedDocument) {
  const std::string text = R"(
    {
      "ok": 123.123.31,
      "ok2:" {
    }
  )";
  ASSERT_THROW(Parse(text), std::runtime_error);
}

TEST(Parser, ThrowOnLeftover) {
  const std::string text = R"(
    {
      "ok": 123,
    }trailingcrap
  )";
  ASSERT_THROW(Parse(text), std::runtime_error);
}

} // namespace
} // namespace minijson