#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "minijson.h"

namespace minijson::internal {
namespace {

TEST(BoundIteratorTest, ThrowOnOutOfRangeDereference) {
  const std::string input = "oi";
  BoundIterator it(input.begin(), input.end());
  ASSERT_EQ(*it, 'o');
  ASSERT_EQ(*++it, 'i');
  ASSERT_THROW(*++it, std::runtime_error);
}

TEST(BoundIteratorTest, ThrowOnOutOfRangePrefixIncrement) {
  const std::string input = "oi";
  BoundIterator it(input.begin(), input.end());
  ASSERT_EQ(*it, 'o');
  ASSERT_EQ(*++it, 'i');
  ASSERT_NO_THROW(++it);
  ASSERT_THROW(++it, std::runtime_error);
}

TEST(BoundIteratorTest, ThrowOnOutOfRangePostfixIncrement) {
  const std::string input = "oi";
  BoundIterator it(input.begin(), input.end());
  ASSERT_EQ(*it, 'o');
  ASSERT_EQ(*++it, 'i');
  ASSERT_NO_THROW(it++);
  ASSERT_THROW(it++, std::runtime_error);
}

TEST(BoundIteratorTest, Advance) {
  const std::string input = "hello, world";
  BoundIterator it(input.begin(), input.end());
  std::advance(it, 4);
  ASSERT_EQ(*it, input[4]);
}

TEST(BoundIteratorTest, AdvanceBeyondEndThrows) {
  const std::string input = "hello, world";
  BoundIterator it(input.begin(), input.end());
  ASSERT_THROW(std::advance(it, input.size() + 1), std::runtime_error);
}

TEST(BoundIteratorTest, EndDeref) {
  const std::string input = "h";
  BoundIterator it(input.begin(), input.end());
  ASSERT_FALSE(it.end());
  ++it;
  ASSERT_TRUE(it.end());
}

} // namespace
} // namespace minijson::internal