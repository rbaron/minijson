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

// Template wrapping a container. It provides exception-throwing iterators.
// The main use is to throw errors on malformed json inputs.
template <typename C> class BoundStream {
public:
  BoundStream(const C &input) : input_(input) {}
  class iterator {
  public:
    using difference_type = typename C::const_iterator::difference_type;
    using value_type = typename C::const_iterator::value_type;
    using pointer = typename C::const_iterator::pointer;
    using reference = typename C::const_iterator::reference;
    using iterator_category = std::input_iterator_tag;
    iterator(const C &input) : it_(input.begin()), end_(input.end()) {}
    iterator(const typename C::const_iterator &end) : it_(end), end_(end) {}
    iterator &operator++() {
      if (it_ == end_) {
        throw std::runtime_error("Out of bounds increment");
      }
      ++it_;
      return *this;
    }
    // Postfix ++ operator. Note that the return type is a lvalue.
    iterator operator++(int _) {
      if (it_ == end_) {
        throw std::runtime_error("Out of bounds increment");
      }
      iterator copy = *this;
      it_++;
      return copy;
    }
    const value_type &operator*() const {
      if (it_ == end_) {
        throw std::runtime_error("Out of bounds dereference");
      }
      return *it_;
    }
    const pointer operator->() { return it_.operator->(); }
    bool operator!=(const iterator &rhs) const { return it_ != rhs.it_; }

  private:
    typename C::const_iterator it_;
    typename C::const_iterator end_;
  };
  iterator begin() { return iterator(input_); }
  iterator end() { return input_.end(); }

private:
  const C input_;
};

Token TokenizeString(std::string::const_iterator *it) {
  std::string out;
  while (*(++*it) != '"') {
    if (**it == '\\') {
      out += **it;
      out += *++*it;
    } else {
      out += **it;
    }
  }
  ++*it;
  return Token{TokenType::kStr, out};
}

Token TokenizeNumber(std::string::const_iterator *it) {
  auto start = *it;
  while (isdigit(*(++*it)) || **it == '.')
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

class JSONNode {
public:
  using Obj = std::unordered_map<std::string, JSONNode>;
  using Arr = std::vector<JSONNode>;

  JSONNode() : type_(Type::kNull) {}
  explicit JSONNode(bool boolean) : type_(Type::kBoolean), bool_(boolean) {}
  explicit JSONNode(double number) : type_(Type::kNumber), number_(number) {}
  explicit JSONNode(const std::string &str) : type_(Type::kStr), str_(str) {}
  explicit JSONNode(const char *str) : type_(Type::kStr), str_(str) {}
  explicit JSONNode(std::unique_ptr<Arr> arr)
      : type_(Type::kArr), arr_(std::move(arr)) {}
  explicit JSONNode(std::unique_ptr<Obj> obj)
      : type_(Type::kObj), obj_(std::move(obj)) {}

  // We need to be careful when initializing the union. Since all members begin
  // at the the same address in memory, we need to make sure the correct one is
  // initialized. This is done with the "placement new" operator for non-builtin
  // types.
  JSONNode(const JSONNode &rhs) {
    type_ = rhs.type_;
    if (rhs.type_ == Type::kNull) {
    } else if (rhs.type_ == Type::kBoolean) {
      bool_ = rhs.bool_;
    } else if (rhs.type_ == Type::kNumber) {
      number_ = rhs.number_;
    } else if (rhs.type_ == Type::kStr) {
      new (&str_) std::string(rhs.str_);
    } else if (rhs.type_ == Type::kArr) {
      new (&arr_) std::unique_ptr(std::make_unique<Arr>(*rhs.arr_));
    } else if (rhs.type_ == Type::kObj) {
      new (&obj_) std::unique_ptr(std::make_unique<Obj>(*rhs.obj_));
    } else {
      throw std::runtime_error("Not copy constructible");
    }
  }

  // Copy-and-swap. We cannot pass by value otherwise there's resolution
  // ambiguity between the copy and move assignments.
  JSONNode &operator=(const JSONNode &rhs) {
    JSONNode tmp(rhs);
    std::swap(type_, tmp.type_);
    if (type_ == Type::kNull) {
    } else if (type_ == Type::kBoolean) {
      std::swap(bool_, tmp.bool_);
    } else if (type_ == Type::kNumber) {
      std::swap(number_, tmp.number_);
    } else if (type_ == Type::kStr) {
      std::swap(str_, tmp.str_);
    } else if (type_ == Type::kArr) {
      std::swap(arr_, tmp.arr_);
    } else if (type_ == Type::kObj) {
      std::swap(obj_, tmp.obj_);
    } else {
      throw std::runtime_error("Not copy assignable");
    }
    return *this;
  }

  JSONNode(JSONNode &&rhs) {
    type_ = rhs.type_;
    if (rhs.type_ == Type::kNull) {
    } else if (rhs.type_ == Type::kBoolean) {
      bool_ = rhs.bool_;
    } else if (rhs.type_ == Type::kNumber) {
      number_ = rhs.number_;
    } else if (rhs.type_ == Type::kStr) {
      new (&str_) std::string(std::move(rhs.str_));
    } else if (rhs.type_ == Type::kArr) {
      new (&arr_) std::unique_ptr(std::move(rhs.arr_));
    } else if (rhs.type_ == Type::kObj) {
      new (&obj_) std::unique_ptr(std::move(rhs.obj_));
    } else {
      throw std::runtime_error("Not move constructible");
    }
  }

  // TODO: extract conditional swaps so we can use the same code for copy and
  // move assignments.
  JSONNode &operator=(JSONNode &&rhs) {
    std::swap(type_, rhs.type_);
    if (type_ == Type::kNull) {
    } else if (type_ == Type::kBoolean) {
      std::swap(bool_, rhs.bool_);
    } else if (type_ == Type::kNumber) {
      std::swap(number_, rhs.number_);
    } else if (type_ == Type::kStr) {
      std::swap(str_, rhs.str_);
    } else if (type_ == Type::kArr) {
      std::swap(arr_, rhs.arr_);
    } else if (type_ == Type::kObj) {
      std::swap(obj_, rhs.obj_);
    } else {
      throw std::runtime_error("Not move assignable");
    }
    return *this;
  }

  ~JSONNode() {
    if (type_ == Type::kStr) {
      str_.~basic_string();
    } else if (type_ == Type::kObj) {
      obj_.~unique_ptr();
    } else if (type_ == Type::kArr) {
      arr_.~unique_ptr();
    }
  };

  JSONNode &operator[](const std::string &key) {
    ASSERT_TYPE(Type::kObj);
    return (*obj_)[key];
  }

  JSONNode &operator[](const size_t idx) {
    ASSERT_TYPE(Type::kArr);
    return (*arr_)[idx];
  }

  bool IsNull() const { return type_ == Type::kNull; }

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
  union {
    bool bool_;
    double number_;
    std::string str_;
    std::unique_ptr<Obj> obj_;
    std::unique_ptr<Arr> arr_;
  };
};

JSONNode ParseJSONNode(BoundStream<std::vector<Token>>::iterator *it);

JSONNode ParseJSONNumber(BoundStream<std::vector<Token>>::iterator *it) {
  const std::string &text = ((*it)++)->text;
  std::string::size_type sz;
  double number = std::stod(text, &sz);
  if (sz != text.size()) {
    throw std::runtime_error("Invalid number: " + text);
  }
  return JSONNode(number);
}

JSONNode ParseJSONConstant(BoundStream<std::vector<Token>>::iterator *it) {
  const std::string &name = ((*it)++)->text;
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

// Takes the escaped UTF16 representation of a unicode code point (eg: "\\u00ec"
// or "\\uD834\\uDD1E") and outputs its unicode code point. *it should be
// pointing to the character 'u'. The iterator is read until the correct number
// is parsed.
unsigned long int EscapedUTF16ToCodepoint(std::string::const_iterator *it) {
  ++*it; // 'u'
  std::string digits(*it, *it + 4);
  *it = *it + 4;
  std::string::size_type sz;
  unsigned int val = std::stoul(digits, &sz, 16);
  // Single 16-bit code unit
  if (val <= 0xd7ff || (val >= 0xe000 && val <= 0xffff)) {
    return val;
    // Surrogate pairs
  } else if (val >= 0xd800 && val <= 0xdfff) {
    ++*it; // '\\'
    ++*it; // 'u'
    std::string low_digits(*it, *it + 4);
    *it = *it + 4;
    unsigned int low = std::stoul(low_digits, &sz, 16);
    // Take 10 LSBs from the high code unit (val) and concatenate with the ones
    // from the low code unit (low).
    return ((0x3ff & val) << 10) | (0x3ff & low) | 0x10000;
  } else {
    throw std::runtime_error("Invalid unicode encoding with value: " +
                             std::to_string(val));
  }
}

// https://linux.die.net/man/7/utf8
std::string CodePointToUTF8(unsigned long int code) {
  if (code < 0x80) {
    return {static_cast<char>(code & 0xff)};
  } else if (code < 0x800) {
    return {static_cast<char>(0xc0 | (code >> 6)),
            static_cast<char>(0x80 | (code & 0x3f))};
  } else if (code < 0x10000) {
    return {static_cast<char>(0xe0 | (code >> 12)),
            static_cast<char>(0x80 | ((code >> 6) & 0x3f)),
            static_cast<char>(0x80 | (code & 0x3f))};
  } else if (code < 0x200000) {
    return {static_cast<char>(0xf0 | (code >> 18)),
            static_cast<char>(0x80 | ((code >> 12) & 0x3f)),
            static_cast<char>(0x80 | ((code >> 6) & 0x3f)),
            static_cast<char>(0x80 | (code & 0x3f))};
  }
  throw std::runtime_error("Code point out of normal people range.");
}

// TODO: bound-checked iterator
std::string ParseString(const std::string &input) {
  std::string out;
  out.reserve(input.size());
  auto it = input.begin();
  while (it != input.end()) {
    if (*it == '\\') {
      switch (*++it) {
      case 'b':
        out += '\b';
        break;
      case 'f':
        out += '\f';
        break;
      case 'n':
        out += '\n';
        break;
      case 'r':
        out += '\r';
        break;
      case 't':
        out += '\t';
        break;
      case '"':
        out += '\"';
        break;
      case '\\':
        out += '\\';
        break;
      case 'u':
        out += CodePointToUTF8(EscapedUTF16ToCodepoint(&it));
        break;
      default:
        throw std::runtime_error("Unrecognized escape sequence: \\" +
                                 std::to_string(*it));
      }
    } else {
      out += *(it++);
    }
  }
  out.shrink_to_fit();
  return out;
}

JSONNode ParseJSONArr(BoundStream<std::vector<Token>>::iterator *it) {
  auto arr = std::make_unique<JSONNode::Arr>();
  ASSERT_CHAR_AND_MOVE(it, "[");
  while ((*it)->type != TokenType::kRSquareBracket) {
    arr->emplace_back(ParseJSONNode(it));
    if ((*it)->type == TokenType::kComma) {
      ++*it;
    } else if ((*it)->type != TokenType::kRSquareBracket) {
      throw std::runtime_error("Expected ]");
    }
  }
  ASSERT_CHAR_AND_MOVE(it, "]");
  return JSONNode(std::move(arr));
}

JSONNode ParseJSONObj(BoundStream<std::vector<Token>>::iterator *it) {
  auto obj = std::make_unique<JSONNode::Obj>();
  ASSERT_CHAR_AND_MOVE(it, "{");
  while ((*it)->type != TokenType::kRCurlyBracket) {
    std::string key = ParseString(((*it)++)->text);
    ASSERT_CHAR_AND_MOVE(it, ":");
    obj->emplace(key, ParseJSONNode(it));
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

// Parses a JSONNode and sets *it to point to the next token to be parsed.
JSONNode ParseJSONNode(BoundStream<std::vector<Token>>::iterator *it) {
  switch ((*it)->type) {
  case TokenType::kStr:
    return JSONNode(ParseString(((*it)++)->text));
  case TokenType::kNumber:
    return ParseJSONNumber(it);
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

JSONNode Parse(const std::string &text) {
  const std::vector<internal::Token> tokens = internal::Tokenize(text);
  internal::BoundStream stream(tokens);
  auto it = stream.begin();
  JSONNode result = internal::ParseJSONNode(&it);
  if (it != stream.end()) {
    throw std::runtime_error(
        "A document was parsed, but there is leftover data in the input");
  }
  return result;
}

} // namespace minijson

#endif // _MINIJSON_H_