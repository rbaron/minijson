#include <iostream>
#include <string>

#include "minijson.h"

int main(int argc, char **argv) {
  std::string text = R"(
    {
      "str_key": "hello, world",
      "num_key": 123,
      "nested_obj_key": {
        "nested_arr_key": [
          "elem1",
          "elem2",
          {
            "inner_obj_key": "ok!"
          }
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