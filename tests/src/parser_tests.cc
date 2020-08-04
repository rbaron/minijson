
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "lexer.h"
#include "parser.h"

namespace minijson {
namespace {

TEST(Parser, ParseString) {
  const std::string text = R"(
    {
      "key": "hello, world"
    }
  )";

  const auto tokens = Tokenize(text);
  auto it = tokens.begin();

  JSONNode json = ParseJSONNode(&it);
  ASSERT_EQ(json["key"].GetStr(), "hello, world");
}

TEST(Parser, ParseNumber) {
  const std::string text = R"(
    {
      "ok": 123
    }
  )";
  const auto tokens = Tokenize(text);
  auto it = tokens.begin();
  JSONNode json = ParseJSONNode(&it);
  ASSERT_DOUBLE_EQ(json["ok"].GetNum(), 123);
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
  const auto tokens = Tokenize(text);
  auto it = tokens.begin();
  JSONNode json = ParseJSONNode(&it);
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
  const auto tokens = Tokenize(text);
  auto it = tokens.begin();
  JSONNode json = ParseJSONNode(&it);
}

} // namespace
} // namespace minijson