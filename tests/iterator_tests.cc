#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "minijson.h"

namespace minijson {
namespace {

TEST(Iterator, IterateOverObjectKeyValues) {
  const std::string text = R"(
    {
      "key1": "hello",
      "key2": "world"
    }
  )";
  JSONNode json = Parse(text);
  std::vector<std::string> keys;
  std::vector<std::string> values;
  for (const auto &[k, v] : json.IterableObj()) {
    keys.push_back(k);
    values.push_back(v.GetStr());
  }
  ASSERT_THAT(keys, ::testing::UnorderedElementsAre("key1", "key2"));
  ASSERT_THAT(values, ::testing::UnorderedElementsAre("hello", "world"));
}

TEST(Iterator, ThrowWhenIteratingOverObjectAsArray) {
  const std::string text = R"(
    {
      "key1": "hello",
      "key2": "world"
    }
  )";
  JSONNode json = Parse(text);
  ASSERT_THROW(json.IterableArr(), std::runtime_error);
}

TEST(Iterator, IterateOverArrayValues) {
  const std::string text = R"(
    ["hello", "world"]
  )";
  JSONNode json = Parse(text);
  std::vector<std::string> values;
  for (const auto &v : json.IterableArr()) {
    values.push_back(v.GetStr());
  }
  ASSERT_THAT(values, ::testing::ElementsAre("hello", "world"));
}

TEST(Iterator, ThrowWhenIteratingOverArrayAsObject) {
  const std::string text = R"(
    ["hello", "world"]
  )";
  JSONNode json = Parse(text);
  ASSERT_THROW(json.IterableObj(), std::runtime_error);
}

} // namespace
} // namespace minijson