#ifndef _PARSER_H_
#define _PARSER_H_

#include <memory>
#include <stdlib.h>
#include <string>
#include <unordered_map>

#include "lexer.h"

#define ASSERT_CHAR_AND_MOVE(it, char) \
  if (((*it)++)->text != char) { \
    throw std::runtime_error("Expected: " char); \
  }

#define ASSERT_TYPE(type) \
  if (type_ != type) { \
    throw std::runtime_error("Wrong type"); \
  }

namespace minijson {

class JSONNode;

namespace {
  using Obj = std::unordered_map<std::string, JSONNode>;
  using Arr = std::vector<JSONNode>;
}  // namespace

class JSONNode {
 public:
  JSONNode(): type_(Type::kUndefined) {}
  explicit JSONNode(Obj&& obj): type_(Type::kObj), obj_(obj) {}
  explicit JSONNode(const std::string& str): type_(Type::kStr), str_(str) {}
  explicit JSONNode(double number): type_(Type::kNumber), number_(number) {}

  JSONNode(const JSONNode& rhs) {
    type_ = rhs.type_;
    if (rhs.type_ == Type::kObj) {
      // Placement new!
      new(&obj_) Obj(rhs.obj_);
    } else if (rhs.type_ == Type::kStr) {
      new(&str_) std::string(rhs.str_);
    } else if (rhs.type_ == Type::kNumber) {
      number_ = rhs.number_;
    } else {
      throw std::runtime_error("Not copyable");
    }
  }

  // Copy-and-swap. Note that the argument is passed by value,
  // so we operate on a copy.
  JSONNode& operator=(JSONNode rhs) {
    std::swap(obj_, rhs.obj_);
    std::swap(str_, rhs.str_);
    std::swap(number_, rhs.number_);
    std::swap(type_, rhs.type_);
    return *this;
  }

  ~JSONNode() {
    if (type_ == Type::kStr) {
      str_.~basic_string();
    } else if (type_ == Type::kObj) {
      obj_.~unordered_map();
    }
  };

  JSONNode& operator[](const std::string& key) {
    ASSERT_TYPE(Type::kObj);
    return obj_[key];
  }

  double GetNum() const {
    ASSERT_TYPE(Type::kNumber);
    return number_;
  }

  std::string GetStr() const {
    ASSERT_TYPE(Type::kStr);
    return str_;
  }

 private:
  // We implement the "tagged union" idiom from
  // "The C++ Programming Language 4th edition".
  enum class Type {
    kUndefined, kNumber, kStr, kObj,
  };
  Type type_;
  // TODO: I think making this a named union might make
  // the Big Five^{TM} cleaner, but not sure about the ctor.
  union {
    double number_;
    std::string str_;
    Obj obj_;
  };
};

JSONNode ParseJSONNode(std::vector<Token>::const_iterator* it);

JSONNode ParseJSONObj(std::vector<Token>::const_iterator* it) {
  Obj obj;
  ASSERT_CHAR_AND_MOVE(it, "{");
  while ((*it)->type != TokenType::kRCurlyBracket) {
    std::string key = ((*it)++)->text;
    ASSERT_CHAR_AND_MOVE(it, ":");
    obj.emplace(key, ParseJSONNode(it));
    if ((*it)->type == TokenType::kComma) {
      ++*it;
    }
  }
  ASSERT_CHAR_AND_MOVE(it, "}");
  return JSONNode(std::move(obj));
}

JSONNode ParseJSONNode(std::vector<Token>::const_iterator* it) {
  switch ((*it)->type) {
    case TokenType::kStr: return JSONNode((*it)++->text);
    case TokenType::kNumber: return JSONNode(std::stod((*it)++->text));
    case TokenType::kLCurlyBracket: return ParseJSONObj(it);
    default: throw new std::runtime_error("Invalid token type");
  }
}

}  // namespace minijson

#endif  // _PARSER_H_