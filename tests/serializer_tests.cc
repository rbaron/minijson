#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "minijson.h"

namespace minijson {
namespace {

TEST(Serializer, Boolean) {
  ASSERT_EQ(Serialize(JSONNode(true)), "true");
  ASSERT_EQ(Serialize(JSONNode(false)), "false");
}

TEST(Serializer, Null) { ASSERT_EQ(Serialize(JSONNode()), "null"); }

TEST(Serializer, UTF8String) {
  JSONNode node("Hello, 🌎");
  ASSERT_EQ(Serialize(node), R"j("Hello, 🌎")j");
}

TEST(Serializer, Double) {
  JSONNode node(123.45);
  ASSERT_EQ(Serialize(node), "123.450000");
}

TEST(Serializer, EmptyArray) {
  auto vec = std::make_unique<std::vector<JSONNode>>();
  JSONNode node(std::move(vec));
  ASSERT_EQ(Serialize(node), "[]");
}

TEST(Serializer, Array) {
  auto vec = std::make_unique<std::vector<JSONNode>>();
  vec->push_back(JSONNode(true));
  vec->push_back(JSONNode(5.0));
  JSONNode node(std::move(vec));
  ASSERT_EQ(Serialize(node), "[true,5.000000]");
}

TEST(Serializer, EmptyObj) {
  auto obj = std::make_unique<std::unordered_map<std::string, JSONNode>>();
  JSONNode node(std::move(obj));
  ASSERT_EQ(Serialize(node), "{}");
}

// TODO: make this test deterministic. The order of kv pairs is unspecified.
TEST(Serializer, Obj) {
  auto obj = std::make_unique<std::unordered_map<std::string, JSONNode>>();
  JSONNode node(std::move(obj));
  node["hello"] = JSONNode("goodbye");
  node["oi"] = JSONNode("tchau");
  ASSERT_EQ(Serialize(node), R"j({"oi":"tchau","hello":"goodbye"})j");
}

} // namespace
} // namespace minijson
