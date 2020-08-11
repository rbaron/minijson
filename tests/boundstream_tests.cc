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
  for (const auto& c : stream) {
    output += c;
  }
  ASSERT_EQ(input, output);
}

TEST(BoundStreamTest, ThrowOnOutOfRangeDerefference) {
  const std::string input = "oi";
  BoundStream stream(input);
  BoundStream::iterator it = stream.begin();
  ASSERT_EQ(*it, 'o');
  ASSERT_EQ(*++it, 'i');
  ASSERT_THROW(*++it, std::runtime_error);
}

TEST(BoundStreamTest, ThrowOnOutOfRangeIncrement) {
  const std::string input = "oi";
  BoundStream stream(input);
  BoundStream::iterator it = stream.begin();
  ASSERT_EQ(*it, 'o');
  ASSERT_EQ(*++it, 'i');
  ASSERT_NO_THROW(++it);
  ASSERT_THROW(++it, std::runtime_error);
}

TEST(BoundStreamTest, Advance) {
  const std::string input = "hello, world";
  BoundStream stream(input);
  BoundStream::iterator it = stream.begin();
  std::advance(it, 4);
  ASSERT_EQ(*it, input[4]);
}

} // namespace
} // namespace minijson::internal