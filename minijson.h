#ifndef _MINIJSON_H_
#define _MINIJSON_H_

#include <cctype>
#include <iterator>
#include <memory>
#include <optional>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <vector>

#define ASSERT_TOKEN_AND_MOVE(it, char)                                        \
  if (((*it)++)->text != char) {                                               \
    throw std::runtime_error("Expected: " char);                               \
  }

#define ASSERT_CHAR_AND_MOVE(it, ch)                                           \
  if (*((*it)++) != ch) {                                                      \
    throw std::runtime_error(std::string("Expected: ") + ch);                  \
  }

// Given a pointer to an iterator, get the current value and advance.
#define GET_AND_MOVE(it) *((*it)++)

#define ASSERT_TYPE(type)                                                      \
  if (type_ != type) {                                                         \
    throw std::runtime_error("Wrong type");                                    \
  }

namespace minijson {

namespace internal {
class JSONNode;
}
std::string Serialize(const internal::JSONNode &json);

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

// Bounds-checking iterator. The main use is to throw errors on malformed json
// inputs.
template <typename Iter> class BoundIterator {
public:
  using difference_type = typename Iter::difference_type;
  using value_type = typename Iter::value_type;
  using pointer = typename Iter::pointer;
  using reference = typename Iter::reference;
  using iterator_category = std::input_iterator_tag;

  BoundIterator(const Iter &begin, const Iter &end) : it_(begin), end_(end) {}

  // Prefix ++ operator.
  BoundIterator &operator++() {
    check_bounds("Out of bounds increment");
    ++it_;
    return *this;
  }
  // Postfix ++ operator. Note that the return type is a pre-increment copy.
  BoundIterator operator++(int _) {
    check_bounds("Out of bounds increment");
    BoundIterator copy = *this;
    it_++;
    return copy;
  }
  const value_type &operator*() const {
    check_bounds("Out of bounds dereference");
    return *it_;
  }
  const pointer operator->() { return it_.operator->(); }
  bool operator!=(const BoundIterator &rhs) const { return it_ != rhs.it_; }
  bool end() { return it_ == end_; }

private:
  void check_bounds(const std::string &error_msg) const {
    if (it_ == end_) {
      throw std::runtime_error(error_msg);
    }
  }
  Iter it_;
  Iter end_;
};

Token TokenizeString(BoundIterator<std::istream_iterator<char>> *it) {
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

Token TokenizeNumber(BoundIterator<std::istream_iterator<char>> *it) {
  std::string out;
  out += **it;
  while (!(++*it).end() && (isdigit(**it) || **it == '.')) {
    out += **it;
  }
  return Token{TokenType::kNumber, out};
}

Token TokenizeConstant(BoundIterator<std::istream_iterator<char>> *it) {
  std::string out;
  out += **it;
  while (isalpha(*++*it)) {
    out += **it;
  }
  return Token{TokenType::kConstant, out};
}

std::optional<Token>
TonekizeOne(BoundIterator<std::istream_iterator<char>> *it) {
  if (**it == '{') {
    ++*it;
    return Token{TokenType::kLCurlyBracket, "{"};
  } else if (**it == '}') {
    ++*it;
    return Token{TokenType::kRCurlyBracket, "}"};
  } else if (**it == '[') {
    ++*it;
    return Token{TokenType::kLSquareBracket, "["};
  } else if (**it == ']') {
    ++*it;
    return Token{TokenType::kRSquareBracket, "]"};
  } else if (**it == ':') {
    ++*it;
    return Token{TokenType::kColon, ":"};
  } else if (**it == ',') {
    ++*it;
    return Token{TokenType::kComma, ","};
  } else if (**it == '"') {
    return TokenizeString(it);
  } else if (isdigit(**it)) {
    return TokenizeNumber(it);
  } else if (isalpha(**it)) {
    return TokenizeConstant(it);
    // Whitespaces, hopefully
  } else {
    ++*it;
    return std::nullopt;
  }
}

std::vector<Token> Tokenize(std::istream *input) {
  std::vector<Token> tokens;
  *input >> std::noskipws;
  // TODO: consider istreambuf_iterator.
  BoundIterator it{std::istream_iterator<char>(*input),
                   std::istream_iterator<char>()};
  while (!it.end()) {
    std::optional<Token> token = TonekizeOne(&it);
    if (token.has_value()) {
      tokens.push_back(token.value());
    }
  }
  return tokens;
}

// TODO: handle integers. See https://tools.ietf.org/html/rfc7159#section-6.
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

  friend void swap(JSONNode &lhs, JSONNode &rhs) {
    using std::swap;
    swap(lhs.type_, rhs.type_);
    if (lhs.type_ == Type::kNull) {
    } else if (lhs.type_ == Type::kBoolean) {
      swap(lhs.bool_, rhs.bool_);
    } else if (lhs.type_ == Type::kNumber) {
      swap(lhs.number_, rhs.number_);
    } else if (lhs.type_ == Type::kStr) {
      swap(lhs.str_, rhs.str_);
    } else if (lhs.type_ == Type::kArr) {
      swap(lhs.arr_, rhs.arr_);
    } else if (lhs.type_ == Type::kObj) {
      swap(lhs.obj_, rhs.obj_);
    } else {
      throw std::runtime_error("Swap not implemented for type");
    }
  }

  // Copy-and-swap.
  // Note that this already covers the move assignment operation too, since we
  // have provided a move constructor - in this case, rhs will be
  // move-constructed when the assignment operator is called with a rvalue.
  JSONNode &operator=(JSONNode rhs) {
    swap(*this, rhs);
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

  enum class Type {
    kNull,
    kBoolean,
    kNumber,
    kStr,
    kArr,
    kObj,
  };

  Type GetType() const { return type_; }

  // TODO: allow null -> obj type change.
  JSONNode &operator[](const std::string &key) {
    ASSERT_TYPE(Type::kObj);
    return (*obj_)[key];
  }

  JSONNode &operator[](const size_t idx) {
    ASSERT_TYPE(Type::kArr);
    return (*arr_)[idx];
  }

  // TODO: allow null -> arr type change.
  void push_back(JSONNode rhs) {
    ASSERT_TYPE(Type::kArr);
    arr_->push_back(rhs);
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

  // Iterators.
private:
  class iterable_obj;
  class iterable_arr;

public:
  iterable_obj IterableObj() const;
  iterable_arr IterableArr() const;

  // Data members.
private:
  // We implement the "tagged union" idiom from
  // "The C++ Programming Language 4th edition".
  Type type_;
  union {
    bool bool_;
    double number_;
    std::string str_;
    std::unique_ptr<Obj> obj_;
    std::unique_ptr<Arr> arr_;
  };
};

class JSONNode::iterable_obj {
public:
  iterable_obj(const JSONNode *const json_node) : json_node_(json_node) {}
  Obj::const_iterator begin() const { return json_node_->obj_->begin(); }
  Obj::const_iterator end() const { return json_node_->obj_->end(); }

private:
  const JSONNode *const json_node_;
};

class JSONNode::iterable_arr {
public:
  iterable_arr(const JSONNode *const json_node) : json_node_(json_node) {}
  Arr::const_iterator begin() const { return json_node_->arr_->begin(); }
  Arr::const_iterator end() const { return json_node_->arr_->end(); }

private:
  const JSONNode *const json_node_;
};

JSONNode::iterable_obj JSONNode::IterableObj() const {
  ASSERT_TYPE(Type::kObj);
  return iterable_obj(this);
}
JSONNode::iterable_arr JSONNode::IterableArr() const {
  ASSERT_TYPE(Type::kArr);
  return iterable_arr(this);
}

JSONNode ParseJSONNode(BoundIterator<std::vector<Token>::const_iterator> *it);

JSONNode
ParseJSONNumber(BoundIterator<std::vector<Token>::const_iterator> *it) {
  const std::string &text = ((*it)++)->text;
  std::string::size_type sz;
  double number = std::stod(text, &sz);
  if (sz != text.size()) {
    throw std::runtime_error("Invalid number: " + text);
  }
  return JSONNode(number);
}

JSONNode
ParseJSONConstant(BoundIterator<std::vector<Token>::const_iterator> *it) {
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
unsigned long int
EscapedUTF16ToCodepoint(BoundIterator<std::string::const_iterator> *it) {
  ASSERT_CHAR_AND_MOVE(it, 'u');
  auto begin = *it;
  std::advance(*it, 4);
  std::string digits(begin, *it);
  std::string::size_type sz;
  unsigned int val = std::stoul(digits, &sz, 16);
  // Single 16-bit code unit
  if (val <= 0xd7ff || (val >= 0xe000 && val <= 0xffff)) {
    return val;
    // Surrogate pairs
  } else if (val >= 0xd800 && val <= 0xdfff) {
    ASSERT_CHAR_AND_MOVE(it, '\\');
    ASSERT_CHAR_AND_MOVE(it, 'u');
    begin = *it;
    std::advance(*it, 4);
    std::string low_digits(begin, *it);
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
  throw std::runtime_error("Code point out of usual range.");
}

// https://linux.die.net/man/7/utf8
unsigned long int
UTF8ToCodePoint(BoundIterator<std::string::const_iterator> *it) {
  // char first = *(*it++);
  unsigned long int first = static_cast<unsigned char>(GET_AND_MOVE(it));
  // Single byte.
  if (!(first & 0x80)) {
    return first & ~0x80;
    // Two bytes.
  } else if (first >> 5 == 0b110) {
    return ((first & 0x1f) << 6) | (GET_AND_MOVE(it) & 0x3f);
    // Three bytes.
  } else if (first >> 4 == 0b1110) {
    return ((first & 0b1111) << 12) | ((GET_AND_MOVE(it) & 0b111111) << 6) |
           (GET_AND_MOVE(it) & 0b111111);
    // Four bytes.
  } else if (first >> 3 == 0b11110) {
    return ((first & 0b111) << 18) | ((GET_AND_MOVE(it) & 0b111111) << 12) |
           ((GET_AND_MOVE(it) & 0b111111) << 6) | (GET_AND_MOVE(it) & 0b111111);
  }
  throw std::runtime_error("Unable to convert utf-8 to codepoint.");
}

std::string ParseString(const std::string &input) {
  std::string out;
  out.reserve(input.size());
  BoundIterator it(input.begin(), input.end());
  while (!it.end()) {
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

JSONNode ParseJSONArr(BoundIterator<std::vector<Token>::const_iterator> *it) {
  auto arr = std::make_unique<JSONNode::Arr>();
  ASSERT_TOKEN_AND_MOVE(it, "[");
  while ((*it)->type != TokenType::kRSquareBracket) {
    arr->emplace_back(ParseJSONNode(it));
    if ((*it)->type == TokenType::kComma) {
      ++*it;
    } else if ((*it)->type != TokenType::kRSquareBracket) {
      throw std::runtime_error("Expected ]");
    }
  }
  ASSERT_TOKEN_AND_MOVE(it, "]");
  return JSONNode(std::move(arr));
}

JSONNode ParseJSONObj(BoundIterator<std::vector<Token>::const_iterator> *it) {
  auto obj = std::make_unique<JSONNode::Obj>();
  ASSERT_TOKEN_AND_MOVE(it, "{");
  while ((*it)->type != TokenType::kRCurlyBracket) {
    std::string key = ParseString(((*it)++)->text);
    ASSERT_TOKEN_AND_MOVE(it, ":");
    obj->emplace(key, ParseJSONNode(it));
    if ((*it)->type == TokenType::kComma) {
      ++*it;
    } else if ((*it)->type != TokenType::kRCurlyBracket) {
      throw std::runtime_error("Expected }");
    }
  }
  ASSERT_TOKEN_AND_MOVE(it, "}");
  return JSONNode(std::move(obj));
}

// Parses a JSONNode and sets *it to point to the next token to be parsed.
JSONNode ParseJSONNode(BoundIterator<std::vector<Token>::const_iterator> *it) {
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

std::string SerializeString(const std::string &str) {
  std::string out("\"");
  out += str;
  out += '\"';
  return out;
}

std::string SerializeArray(const JSONNode &json) {
  auto it = json.IterableArr();
  if (it.begin() == it.end()) {
    return "[]";
  }
  std::string out("[");
  std::for_each(it.begin(), it.end(), [&out](const JSONNode &node) {
    out += Serialize(node);
    out += ',';
  });
  // Remove trailing ','
  out.pop_back();
  out += "]";
  return out;
}

std::string SerializeObj(const JSONNode &json) {
  auto it = json.IterableObj();
  if (it.begin() == it.end()) {
    return "{}";
  }
  std::string out("{");
  std::for_each(it.begin(), it.end(),
                [&out](const JSONNode::Obj::value_type &kv) {
                  out += SerializeString(kv.first);
                  out += ':';
                  out += Serialize(kv.second);
                  out += ',';
                });
  // Remove trailing ','
  out.pop_back();
  out += "}";
  return out;
}

} // namespace internal

using JSONNode = internal::JSONNode;

JSONNode Parse(std::istream *in) {
  const std::vector<internal::Token> tokens = internal::Tokenize(in);
  internal::BoundIterator it(tokens.begin(), tokens.end());
  JSONNode result = internal::ParseJSONNode(&it);
  if (!it.end()) {
    throw std::runtime_error(
        "A document was parsed, but there is leftover data in the input");
  }
  return result;
}

JSONNode Parse(const std::string &text) {
  std::istringstream in(text);
  return Parse(&in);
}

std::string Serialize(const JSONNode &json) {
  switch (json.GetType()) {
  case JSONNode::Type::kBoolean:
    return json.GetBool() ? "true" : "false";
  case JSONNode::Type::kNull:
    return "null";
  case JSONNode::Type::kStr:
    return internal::SerializeString(json.GetStr());
  case JSONNode::Type::kNumber:
    return std::to_string(json.GetNum());
  case JSONNode::Type::kArr:
    return internal::SerializeArray(json);
  case JSONNode::Type::kObj:
    return internal::SerializeObj(json);
  default:
    throw std::runtime_error("Unhandled JSONNode type.");
  }
}

} // namespace minijson

#endif // _MINIJSON_H_