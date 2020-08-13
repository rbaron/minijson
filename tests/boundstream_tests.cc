#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "minijson.h"

namespace minijson::internal {
namespace {

TEST(BoundStreamTest, WorkWithRangeForLoop) {
  const std::string input = "hello, world";
  std::string output;
  BoundStream stream(input);
  for (const auto &c : stream) {
    output += c;
  }
  ASSERT_EQ(input, output);
}

TEST(BoundStreamTest, ThrowOnOutOfRangeDereference) {
  const std::string input = "oi";
  BoundStream stream(input);
  auto it = stream.begin();
  ASSERT_EQ(*it, 'o');
  ASSERT_EQ(*++it, 'i');
  ASSERT_THROW(*++it, std::runtime_error);
}

TEST(BoundStreamTest, ThrowOnOutOfRangePrefixIncrement) {
  const std::string input = "oi";
  BoundStream stream(input);
  auto it = stream.begin();
  ASSERT_EQ(*it, 'o');
  ASSERT_EQ(*++it, 'i');
  ASSERT_NO_THROW(++it);
  ASSERT_THROW(++it, std::runtime_error);
}

TEST(BoundStreamTest, ThrowOnOutOfRangePostfixIncrement) {
  const std::string input = "oi";
  BoundStream stream(input);
  auto it = stream.begin();
  ASSERT_EQ(*it, 'o');
  ASSERT_EQ(*++it, 'i');
  ASSERT_NO_THROW(it++);
  ASSERT_THROW(it++, std::runtime_error);
}

TEST(BoundStreamTest, Advance) {
  const std::string input = "hello, world";
  BoundStream stream(input);
  auto it = stream.begin();
  std::advance(it, 4);
  ASSERT_EQ(*it, input[4]);
}

TEST(BoundStreamTest, AdvanceBeyondEndThrows) {
  const std::string input = "hello, world";
  BoundStream stream(input);
  auto it = stream.begin();
  ASSERT_THROW(std::advance(it, input.size() + 1), std::runtime_error);
}

TEST(BoundStreamTest, EndDeref) {
  const std::string input = "h";
  BoundStream stream(input);
  auto it = stream.begin();
  ASSERT_TRUE(it != stream.end());
  ++it;
  ASSERT_FALSE(it != stream.end());
}

} // namespace
} // namespace minijson::internal