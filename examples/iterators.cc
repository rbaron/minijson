#include <fstream>
#include <iostream>
#include <string>

#include "minijson.h"

int main(int argc, char **argv) {

  std::string text = R"(
    {
      "key1": "hello",
      "key2": "world",
      "arr_key": [
        "elem1",
        "elem2",
        "elem3",
      ]
    }
  )";
  minijson::JSONNode json = minijson::Parse(text);

  // Iterate over objects' key & values:
  for (const auto &[key, value] : json.IterableObj()) {
    std::cout << key << ": IsNull? -> " << value.IsNull() << std::endl;
  }

  // Iterate over arrays' values:
  for (const auto &value : json["arr_key"].IterableArr()) {
    std::cout << value.GetStr() << std::endl;
  }

  return 0;
}