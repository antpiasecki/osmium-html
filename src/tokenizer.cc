#include "tokenizer.hh"
#include <cassert>
#include <cctype>
#include <functional>
#include <iostream>
#include <sstream>

#define UNIMPLEMENTED()                                                        \
  std::cerr << "UNIMPLEMENTED() reached on " << __FILE__ << ", line "          \
            << __LINE__ << "\n";                                               \
  exit(1)

std::ostream &operator<<(std::ostream &os, const TokenType e) {
  switch (e) {
  case TokenType::StartTag:
    os << "StartTag";
    break;
  case TokenType::EndTag:
    os << "EndTag";
    break;
  case TokenType::Character:
    os << "Character";
    break;
  default:
    UNIMPLEMENTED();
  }
  return os;
}

std::string Token::dump() const {
  std::stringstream ss;
  ss << "Token(" << m_type << ", " << m_data << ")";
  return ss.str();
}

std::vector<Token> Tokenizer::parse() {
  while (!eof()) { // TODO: handle EOF
    step();
  }
  return m_tokens;
}

void Tokenizer::step() {
  static const std::unordered_map<State, std::function<void()>> state_handlers =
      {{State::Data, [this]() { step_data(); }},
       {State::TagOpen, [this]() { step_tag_open(); }},
       {State::TagName, [this]() { step_tag_name(); }},
       {State::EndTagOpen, [this]() { step_end_tag_open(); }}};

  auto it = state_handlers.find(m_state);
  assert(it != state_handlers.end());
  it->second();
}

// https://html.spec.whatwg.org/multipage/parsing.html#data-state
void Tokenizer::step_data() {
  char c = consume();
  if (c == '<') {
    m_state = State::TagOpen;
  } else {
    m_tokens.emplace_back(TokenType::Character, std::string(1, c));
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#tag-open-state
void Tokenizer::step_tag_open() {
  char c = consume();
  if (c == '/') {
    m_state = State::EndTagOpen;
  } else if (std::isalpha(c) != 0) {
    m_tokens.emplace_back(TokenType::StartTag, "");
    m_current--;
    m_state = State::TagName;
  } else {
    UNIMPLEMENTED();
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#tag-name-state
void Tokenizer::step_tag_name() {
  char c = consume();
  if (c == '>') {
    m_state = State::Data;
  } else {
    current_token().data() += c;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#end-tag-open-state
void Tokenizer::step_end_tag_open() {
  char c = consume();
  if (std::isalpha(c) != 0) {
    m_tokens.emplace_back(TokenType::EndTag, "");
    m_current--;
    m_state = State::TagName;
  } else {
    UNIMPLEMENTED();
  }
}