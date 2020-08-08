#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "minijson.h"

namespace minijson::internal {
namespace {

TEST(Tokenizer, TokenizeSmokeTest) {
  const std::string json = R"(
    {
      "hello": "ok",
      "number_key": 123,
      "bool_key": true,
      "list_key": [
        "item1",
        321
      ],
      "nested_key": {
        "nested_key_1": 123,
        "nested_key_2": "abc"
      }
    }
  )";
  auto tokens = Tokenize(json);
  ASSERT_THAT(
      tokens,
      ::testing::ElementsAre(
          Token{TokenType::kLCurlyBracket, "{"},
          Token{TokenType::kStr, "hello"}, Token{TokenType::kColon, ":"},
          Token{TokenType::kStr, "ok"}, Token{TokenType::kComma, ","},
          Token{TokenType::kStr, "number_key"}, Token{TokenType::kColon, ":"},
          Token{TokenType::kNumber, "123"}, Token{TokenType::kComma, ","},
          Token{TokenType::kStr, "bool_key"}, Token{TokenType::kColon, ":"},
          Token{TokenType::kConstant, "true"}, Token{TokenType::kComma, ","},
          Token{TokenType::kStr, "list_key"}, Token{TokenType::kColon, ":"},
          Token{TokenType::kLSquareBracket, "["},
          Token{TokenType::kStr, "item1"}, Token{TokenType::kComma, ","},
          Token{TokenType::kNumber, "321"},
          Token{TokenType::kRSquareBracket, "]"}, Token{TokenType::kComma, ","},
          Token{TokenType::kStr, "nested_key"}, Token{TokenType::kColon, ":"},
          Token{TokenType::kLCurlyBracket, "{"},
          Token{TokenType::kStr, "nested_key_1"}, Token{TokenType::kColon, ":"},
          Token{TokenType::kNumber, "123"}, Token{TokenType::kComma, ","},
          Token{TokenType::kStr, "nested_key_2"}, Token{TokenType::kColon, ":"},
          Token{TokenType::kStr, "abc"}, Token{TokenType::kRCurlyBracket, "}"},
          Token{TokenType::kRCurlyBracket, "}"}));
}

TEST(Tokenizer, UnderstandEscapeSequences) {
  const std::string json = R"(
    "It goes:\n\"Muchos años después, frente al pelotón de fusilamiento (...)\""
  )";
  auto tokens = Tokenize(json);
  ASSERT_THAT(tokens, ::testing::ElementsAre(
                          Token{TokenType::kStr,
                                "It goes:\\n\\\"Muchos años después, frente al "
                                "pelotón de fusilamiento (...)\\\""}));
}

TEST(Tokenizer, IntegerWorks) {
  auto tokens = Tokenize("123");
  ASSERT_THAT(tokens, ::testing::ElementsAre(Token{TokenType::kNumber, "123"}));
}

TEST(Tokenizer, FloatingPointWorks) {
  auto tokens = Tokenize("123.123");
  ASSERT_THAT(tokens,
              ::testing::ElementsAre(Token{TokenType::kNumber, "123.123"}));
}

} // namespace
} // namespace minijson::internal