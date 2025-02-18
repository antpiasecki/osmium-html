#pragma once

#include "dom.hh"
#include "tokenizer.hh"
#include <memory>
#include <stack>

class Parser {
public:
  explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

  std::shared_ptr<Node> parse();

private:
  size_t m_current = 0;
  std::vector<Token> m_tokens;
  std::stack<std::shared_ptr<Element>> m_open_elements;
  std::string text;

  static bool is_void_element(const std::string &name);

  std::shared_ptr<Element> current_node() { return m_open_elements.top(); }
  Token &consume() { return m_tokens[m_current++]; }
  [[nodiscard]] bool eof() const { return m_current >= m_tokens.size(); }
};