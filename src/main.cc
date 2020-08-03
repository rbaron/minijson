#include <iostream>
#include <string>

#include "lexer.h"
#include "parser.h"

using namespace minijson;

int main(int argc, char **argv) {
  // const std::string json = R"(
  //   {
  //     "hello": "ok",
  //     "number_key": 123,
  //     "bool_key": true,
  //     "list_key": [
  //       "item1",
  //       321,
  //     ],
  //     "nested_key": {
  //       "nested_key_1": 123,
  //       "nested_key_2": "abc",
  //     }
  //   }
  // )";
  // std::cout << json << std::endl;
  // auto tokens = minijson::Tokenize(json);
  // for (const auto& t : tokens) {
  //   std::cout << t.text << std::endl;
  // }

  const std::vector<Token> tokens{
    {TokenType::kStr, "I am a string"}
  };

  auto it = tokens.begin();
  std::unique_ptr<JSONNode> parsed = ParseJSONNode(&it);
  std::cout << parsed->Repr() << std::endl;
  return 0;
}