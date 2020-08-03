
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "lexer.h"
#include "parser.h"

namespace minijson {
namespace {

// TEST(Parser, ParseNumber) {
//   const std::string text = R"(
//     123
//   )";

//   const auto tokens = Tokenize(text);
//   auto it = tokens.begin();

//   std::unique_ptr<JSONNode> json = ParseJSONNode(&it);
//   JSONNumber *n = json->get<JSONNumber>();
//   ASSERT_DOUBLE_EQ(n->Value(), 123);
// }

// TEST(Parser, ParseString) {
//   const std::string text = R"(
//     "hello, world"
//   )";

//   const auto tokens = Tokenize(text);
//   auto it = tokens.begin();

//   std::unique_ptr<JSONNode> json = ParseJSONNode(&it);
//   JSONStr *str = json->get<JSONStr>();
//   ASSERT_EQ(str->Value(), "hello, world");
// }

// TEST(Parser, ParseDoc) {
//   const std::string text = R"(
//     {
//       "ok": 123
//     }
//   )";

//   const auto tokens = Tokenize(text);
//   auto it = tokens.begin();

//   std::unique_ptr<JSONNode> json = ParseJSONNode(&it);
//   JSONDoc *doc = json->get<JSONDoc>();
//   ASSERT_DOUBLE_EQ((*doc)["ok"]->get<JSONNumber>()->Value(), 123);
// }

TEST(Parser, ParseNestedDoc) {
  const std::string text = R"(
    {
      "ok": 123,
      "nested": {
        "nested_1": "abc",
        "nested_2": {
          "nested_3": 123
        }
      }
    }
  )";

  const auto tokens = Tokenize(text);
  auto it = tokens.begin();

  std::unique_ptr<JSONNode> json = ParseJSONNode(&it);
  JSONDoc *doc = json->get<JSONDoc>();
  ASSERT_EQ(doc->operator[]("nested")
                ->get<JSONDoc>()
                ->
                operator[]("nested_1")
                ->get<JSONStr>()
                ->Value(),
            "abc");
  ASSERT_EQ(doc->operator[]("nested")
                ->get<JSONDoc>()
                ->
                operator[]("nested_2")
                ->get<JSONDoc>()
                ->
                operator[]("nested_3")
                ->get<JSONNumber>()
                ->Value(),
            123);
}

} // namespace
} // namespace minijson