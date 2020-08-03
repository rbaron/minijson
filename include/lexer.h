#ifndef _LEXER_H_
#define _LEXER_H_

#include <cctype>
#include <optional>
#include <vector>
#include <string>
#include <string_view>

namespace minijson {

enum class TokenType {
  kUnknown,
  kLCurlyBracket,
  kRCurlyBracket,
  kLSquareBracket,
  kRSquareBracket,
  kName, // true, false, null
  kStr,
  kComma,
  kColon,
  kNumber,
};

struct Token {
  TokenType type;
  std::string text;
};

bool operator==(const Token& lhs, const Token& rhs) {
  return lhs.type == rhs.type && lhs.text == rhs.text;
}

namespace {

Token TokenizeString(std::string::const_iterator *it) {
  // TODO: handle escaped double quotes
  auto start = *it;
  while (*(++*it) != '"');
  return Token{TokenType::kStr, std::string(start+1, (*it)++)};
}

// TODO: allow a single '.'
Token TokenizeNumber(std::string::const_iterator *it) {
  auto start = *it;
  while (isdigit(*(++*it)));
  return Token{TokenType::kNumber, std::string(start, *it)};
}

bool IsNameChar(char c) {
  return isalnum(c) || c == '_' || c == '-';
}

Token TokenizeName(std::string::const_iterator *it) {
  auto start = *it;
  while (IsNameChar(*++*it));
  return Token{TokenType::kName, std::string(start, *it)};
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

}  // namespace

std::vector<Token> Tokenize(const std::string& input) {
  std::vector<Token> tokens;
  for (auto it = input.begin(); it != input.end(); ) {
    std::optional<Token> token = TonekizeOne(&it);
    if (token.has_value()) {
      tokens.push_back(token.value());
    }
  }
  return tokens;
}

}  // namespace minijson
#endif  // _LEXER_H_