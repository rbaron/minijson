[![Tests workflow](https://github.com/rbaron/minijson/workflows/Tests/badge.svg)](https://github.com/rbaron/minijson/actions?query=workflow%3ATests)

# minijson
Tiny header-only C++ JSON parsing library. Work in progress.

# Example
From `examples/json_parsing.cc`:
```cpp
#include <iostream>
#include <string>

#include "minijson.h"

int main(int argc, char **argv) {
  std::string text = R"(
    {
      "str_key": "hello, world",
      "num_key": 123,
      "null_key": null,
      "true_key": true,
      "nested_obj_key": {
        "nested_arr_key": [
          "elem1",
          "elem2",
          {
            "inner_obj_key": "ok!"
          },
        ]
      }
    }
  )";

  minijson::JSONNode json = minijson::Parse(text);

  // Get string values
  std::cout << "\"str_key\" contains: " << json["str_key"].GetStr()
            << std::endl;

  // Get numeric values (as `double`s)
  std::cout << "\"str_key\" contains: " << json["num_key"].GetNum()
            << std::endl;

  // Get boolean values
  std::cout << "\"true_key\" contains: " << json["true_key"].GetBool()
            << std::endl;

  // Test for null values
  std::cout << "is \"null_key\" null? " << json["null_key"].IsNull()
            << std::endl;

  // Index into arrays
  std::cout << "\"nested_arr_key\"[0] contains: "
            << json["nested_obj_key"]["nested_arr_key"][0].GetStr()
            << std::endl;

  // Access nested objects
  std::cout
      << "\"inner_key\" contains: "
      << json["nested_obj_key"]["nested_arr_key"][2]["inner_obj_key"].GetStr()
      << std::endl;

  return 0;
}
```

# Tests
## Unit tests
```bash
# Build
$ make build
# Run
$ make unittests
```
