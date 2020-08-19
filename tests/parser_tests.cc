#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "minijson.h"

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

TEST(Parser, ThrowOnMalformedArrayMissingClosingBracket) {
  const std::string text = R"(
    [true, false
  )";
  ASSERT_THROW(Parse(text), std::runtime_error);
}

TEST(Parser, ThrowOnMalformedArrayMissingComma) {
  const std::string text = R"(
    [true false]
  )";
  ASSERT_THROW(Parse(text), std::runtime_error);
}

} // namespace
} // namespace minijson