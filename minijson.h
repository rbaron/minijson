#ifndef _MINIJSON_H_
#define _MINIJSON_H_

#include <cctype>
#include <memory>
#include <optional>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <vector>

#define ASSERT_CHAR_AND_MOVE(it, char)                                         \
  if (((*it)++)->text != char) {                                               \
    throw std::runtime_error("Expected: " char);                               \
  }

#define ASSERT_TYPE(type)                                                      \
  if (type_ != type) {                                                         \
    throw std::runtime_error("Wrong type");                                    \
  }

namespace minijson {
namespace internal {

enum class TokenType {
  kLCurlyBracket,
  kRCurlyBracket,
  kLSquareBracket,
  kRSquareBracket,
  kConstant, // true, false, null
  kStr,
  kComma,
  kColon,
  kNumber,
};

struct Token {
  TokenType type;
  std::string text;
};

bool operator==(const Token &lhs, const Token &rhs) {
  return lhs.type == rhs.type && lhs.text == rhs.text;
}

Token TokenizeString(std::string::const_iterator *it) {
  // TODO: handle escaped double quotes
  auto start = *it;
  while (*(++*it) != '"')
    ;
  return Token{TokenType::kStr, std::string(start + 1, (*it)++)};
}

// TODO: allow a single '.'
Token TokenizeNumber(std::string::const_iterator *it) {
  auto start = *it;
  while (isdigit(*(++*it)))
    ;
  return Token{TokenType::kNumber, std::string(start, *it)};
}

bool IsNameChar(char c) { return isalnum(c) || c == '_' || c == '-'; }

Token TokenizeName(std::string::const_iterator *it) {
  auto start = *it;
  while (IsNameChar(*++*it))
    ;
  return Token{TokenType::kConstant, std::string(start, *it)};
}

std::optional<Token> TonekizeOne(std::string::const_iterator *it) {
  if (**it == '{') {
    return Token{TokenType::kLCurlyBracket, std::string(*it, ++*it)};
  } else if (**it == '}') {
    return Token{TokenType::kRCurlyBracket, std::string(*it, ++*it)};
  } else if (**it == '[') {
    return Token{TokenType::kLSquareBracket, std::string(*it, ++*it)};
  } else if (**it == ']') {
    return Token{TokenType::kRSquareBracket, std::string(*it, ++*it)};
  } else if (**it == ':') {
    return Token{TokenType::kColon, std::string(*it, ++*it)};
  } else if (**it == ',') {
    return Token{TokenType::kComma, std::string(*it, ++*it)};
  } else if (**it == '"') {
    return TokenizeString(it);
  } else if (isdigit(**it)) {
    return TokenizeNumber(it);
  } else if (IsNameChar(**it)) {
    return TokenizeName(it);
    // Whitespaces, hopefully
  } else {
    ++*it;
    return std::nullopt;
  }
}

class JSONNode;
JSONNode ParseJSONNode(std::vector<Token>::const_iterator *it);

using Obj = std::unordered_map<std::string, JSONNode>;
using Arr = std::vector<JSONNode>;

class JSONNode {
public:
  JSONNode() : type_(Type::kNull) {}
  explicit JSONNode(bool boolean) : type_(Type::kBoolean), bool_(boolean) {}
  explicit JSONNode(double number) : type_(Type::kNumber), number_(number) {}
  explicit JSONNode(const std::string &str) : type_(Type::kStr), str_(str) {}
  explicit JSONNode(Arr &&arr) : type_(Type::kArr), arr_(arr) {}
  explicit JSONNode(Obj &&obj) : type_(Type::kObj), obj_(obj) {}

  JSONNode(const JSONNode &rhs) {
    type_ = rhs.type_;
    if (rhs.type_ == Type::kObj) {
      // Placement new!
      new (&obj_) Obj(rhs.obj_);
    } else if (rhs.type_ == Type::kArr) {
      new (&arr_) Arr(rhs.arr_);
    } else if (rhs.type_ == Type::kStr) {
      new (&str_) std::string(rhs.str_);
    } else if (rhs.type_ == Type::kNumber) {
      number_ = rhs.number_;
    } else if (rhs.type_ == Type::kBoolean) {
      bool_ = rhs.bool_;
    } else if (rhs.type_ == Type::kNull) {
    } else {
      throw std::runtime_error("Not copyable");
    }
  }

  // Copy-and-swap. Note that the argument is passed by value,
  // so we operate on a copy.
  JSONNode &operator=(JSONNode rhs) {
    std::swap(type_, rhs.type_);
    std::swap(obj_, rhs.obj_);
    std::swap(str_, rhs.str_);
    std::swap(number_, rhs.number_);
    std::swap(type_, rhs.type_);
    std::swap(bool_, rhs.bool_);
    return *this;
  }

  ~JSONNode() {
    if (type_ == Type::kStr) {
      str_.~basic_string();
    } else if (type_ == Type::kObj) {
      obj_.~unordered_map();
    }
  };

  JSONNode &operator[](const std::string &key) {
    ASSERT_TYPE(Type::kObj);
    return obj_[key];
  }

  JSONNode &operator[](const size_t idx) {
    ASSERT_TYPE(Type::kArr);
    return arr_[idx];
  }

  bool IsNull() const {
    return type_ == Type::kNull;
  }

  double GetBool() const {
    ASSERT_TYPE(Type::kBoolean);
    return bool_;
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
    kNull,
    kBoolean,
    kNumber,
    kStr,
    kArr,
    kObj,
  };
  Type type_;
  // TODO: I think making this a named union might make
  // the Big Five^{TM} cleaner, but not sure about the ctor.
  union {
    double number_;
    std::string str_;
    Obj obj_;
    Arr arr_;
    bool bool_;
  };
};

JSONNode ParseJSONConstant(std::vector<Token>::const_iterator *it) {
  const std::string& name = ((*it)++)->text;
  if (name == "true") {
    return JSONNode(true);
  } else if (name == "false") {
    return JSONNode(false);
  } else if (name == "null") {
    return JSONNode();
  } else {
    throw std::runtime_error("Unkown name: " + name);
  }
}

JSONNode ParseJSONArr(std::vector<Token>::const_iterator *it) {
  Arr arr;
  ASSERT_CHAR_AND_MOVE(it, "[");
  while ((*it)->type != TokenType::kRSquareBracket) {
    arr.emplace_back(ParseJSONNode(it));
    if ((*it)->type == TokenType::kComma) {
      ++*it;
    } else if ((*it)->type != TokenType::kRSquareBracket) {
      throw std::runtime_error("Expected ]");
    }
  }
  ASSERT_CHAR_AND_MOVE(it, "]");
  return JSONNode(std::move(arr));
}

JSONNode ParseJSONObj(std::vector<Token>::const_iterator *it) {
  Obj obj;
  ASSERT_CHAR_AND_MOVE(it, "{");
  while ((*it)->type != TokenType::kRCurlyBracket) {
    std::string key = ((*it)++)->text;
    ASSERT_CHAR_AND_MOVE(it, ":");
    obj.emplace(key, ParseJSONNode(it));
    if ((*it)->type == TokenType::kComma) {
      ++*it;
    } else if ((*it)->type != TokenType::kRCurlyBracket) {
      throw std::runtime_error("Expected }");
    }
  }
  ASSERT_CHAR_AND_MOVE(it, "}");
  return JSONNode(std::move(obj));
}

std::vector<Token> Tokenize(const std::string &input) {
  std::vector<Token> tokens;
  for (auto it = input.begin(); it != input.end();) {
    std::optional<Token> token = TonekizeOne(&it);
    if (token.has_value()) {
      tokens.push_back(token.value());
    }
  }
  return tokens;
}

JSONNode ParseJSONNode(std::vector<Token>::const_iterator *it) {
  switch ((*it)->type) {
  case TokenType::kStr:
    return JSONNode((*it)++->text);
  case TokenType::kNumber:
    return JSONNode(std::stod((*it)++->text));
  case TokenType::kConstant:
    return ParseJSONConstant(it);
  case TokenType::kLCurlyBracket:
    return ParseJSONObj(it);
  case TokenType::kLSquareBracket:
    return ParseJSONArr(it);
  default:
    throw new std::runtime_error("Invalid token type");
  }
}

} // namespace internal

using JSONNode = internal::JSONNode;

JSONNode Parse(const std::string& text) {
  const auto tokens =  internal::Tokenize(text);
  auto it = tokens.begin();
  return internal::ParseJSONNode(&it);
}

} // namespace minijson

#endif // _MINIJSON_H_