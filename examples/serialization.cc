#include <iostream>
#include <string>

#include "minijson.h"

int main(int argc, char **argv) {
  std::string text = R"(
    {
      "str_key": "hello, world"
    }
  )";

  minijson::JSONNode json = minijson::Parse(text);

  // Write to any std::ostream.
  std::cout << json << std::endl;

  // Including files or strings.
  std::ostringstream out;
  out << json << std::endl;
  std::cout << out.str() << std::endl;

  // For convenience, we can directly produce a string. Under the hood, it will
  // create its own std::ostringstream and do as the example above.
  std::string json_str = minijson::Serialize(json);
  std::cout << json_str << std::endl;

  return 0;
}