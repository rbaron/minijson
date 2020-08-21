[![Tests workflow](https://github.com/rbaron/minijson/workflows/Tests/badge.svg)](https://github.com/rbaron/minijson/actions?query=workflow%3ATests)

# minijson
Tiny header-only C++ JSON parsing library. Work in progress.

# Examples
See files under `examples/`.

# Parsing
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

# Serialization
From `examples/serialization.cc`:
```cpp
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
```

# Tests
## Unit tests
```bash
# Build
$ make build
# Run
$ make unittests
```