
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "minijson.h"

namespace minijson::internal {
namespace {

TEST(Parser, CopyJSONNode) {
  auto arr = std::make_unique<JSONNode::Arr>();
  JSONNode str_node("Hello");
  arr->push_back(str_node);
  JSONNode json1(std::move(arr));
  JSONNode json2(json1);
}

TEST(Parser, CopyAssignBoolean) {
  JSONNode str_node("Hello");
  ASSERT_EQ(str_node.GetStr(), "Hello");
}

TEST(Parser, CopyJSONNodeWithNestedObj) {
  auto nested_obj = std::make_unique<JSONNode::Obj>();
  (*nested_obj)["key_nested"] = JSONNode(123.0);
  JSONNode nested_node(std::move(nested_obj));

  auto obj = std::make_unique<JSONNode::Obj>();
  (*obj)["key1"] = JSONNode("hello");
  (*obj)["key2"] = JSONNode("world");
  (*obj)["key3"] = nested_node;

  JSONNode json1(std::move(obj));
  JSONNode json2(json1);

  ASSERT_NE(&json1["key1"], &json2["key2"]);
  ASSERT_EQ(json1["key1"].GetStr(), json2["key1"].GetStr());
}

} // namespace
} // namespace minijson::internal