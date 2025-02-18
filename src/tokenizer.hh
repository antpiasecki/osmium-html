#pragma once

#include <string>
#include <utility>
#include <vector>

enum class TokenType {
  StartTag,
  EndTag,
  Character,
};

class Token {
public:
  Token(TokenType type, std::string data)
      : m_type(type), m_data(std::move(data)) {}

  [[nodiscard]] TokenType type() const { return m_type; }
  [[nodiscard]] std::string &data() { return m_data; }

  [[nodiscard]] std::string dump() const;

private:
  TokenType m_type;
  std::string m_data;
};

enum class State {
  Data,
  TagOpen,
  TagName,
  EndTagOpen,
};

class Tokenizer {
public:
  explicit Tokenizer(std::string data) : m_data(std::move(data)) {}

  std::vector<Token> parse();

private:
  State m_state{State::Data};
  std::string m_data;
  size_t m_current = 0;
  std::vector<Token> m_tokens;

  void step();
  void step_data();
  void step_tag_open();
  void step_tag_name();
  void step_end_tag_open();

  [[nodiscard]] Token &current_token() { return m_tokens.back(); }
  char consume() { return m_data[m_current++]; }
  [[nodiscard]] bool eof() const { return m_current >= m_data.length(); }
};