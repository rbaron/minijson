#ifndef _PARSER_H_
#define _PARSER_H_

#include <memory>
#include <stdlib.h>
#include <string>
#include <unordered_map>

#include "lexer.h"

namespace minijson {

class JSONNode {
 public:
  virtual ~JSONNode() = default;
  virtual std::string Repr() const = 0;

  template <typename T>
  T* get() {
    return static_cast<T*>(this);
  }
};

class JSONDoc : public JSONNode {
 public:
  virtual std::string Repr() const override {
    std::string out;
    for (const auto& [key, value] : map_) {
      out += key + ": " + value->Repr() + "\n";
    }
    return out;
  }

  // JSONNode* operator[](const std::string& key) {
  std::unique_ptr<JSONNode>& operator[](const std::string& key) {
    return map_[key];
  }

 private:
  std::unordered_map<std::string, std::unique_ptr<JSONNode>> map_;
};

class JSONStr : public JSONNode {
 public:
  explicit JSONStr(const std::string& str): val_(str) {}

  std::string Repr() const override {
    return "\"" + val_ + "\"";
  }

  std::string Value() {
    return val_;
  }
 private:
  std::string val_;
};

class JSONNumber : public JSONNode {
 public:
  explicit JSONNumber(const std::string& str): val_(std::stod(str)) {}

  std::string Repr() const override {
    return std::to_string(val_);
  }

  double Value() {
    return val_;
  }
 private:
  double val_;
};

std::unique_ptr<JSONNode> ParseJSONNode(std::vector<Token>::const_iterator* it);

std::unique_ptr<JSONDoc> ParseJSONDoc(std::vector<Token>::const_iterator* it) {
  auto doc = std::make_unique<JSONDoc>();
  ++*it;
  while ((*it)->type != TokenType::kRCurlyBracket) {
    std::string key = ((*it)++)->text;
    // TODO: assert colon
    ++*it;
    std::unique_ptr<JSONNode> value = ParseJSONNode(it);
    (*doc)[key] = std::move(value);

    if ((*it)->type == TokenType::kComma) {
      ++*it;
    }
  }
  // Consume right curly bracket
  ++it;
  return doc;
}

std::unique_ptr<JSONNode> ParseJSONNode(std::vector<Token>::const_iterator* it) {
  if ((*it)->type == TokenType::kStr) return std::make_unique<JSONStr>(((*it)++)->text);
  else if ((*it)->type == TokenType::kNumber) return std::make_unique<JSONNumber>(((*it)++)->text);
  else if ((*it)->type == TokenType::kLCurlyBracket) return ParseJSONDoc(it);
  else return nullptr;
}

}  // namespace minijson

#endif  // _PARSER_H_