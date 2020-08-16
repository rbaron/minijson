#include <fstream>
#include <iostream>
#include <string>

#include "minijson.h"

int main(int argc, char **argv) {

  std::ifstream file_stream("examples/parsing_from_file_example.json");
  minijson::JSONNode json = minijson::Parse(&file_stream);

  std::cout
      << "\"inner_key\" contains: "
      << json["nested_obj_key"]["nested_arr_key"][2]["inner_obj_key"].GetStr()
      << std::endl;

  return 0;
}