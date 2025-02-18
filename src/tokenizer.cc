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
  case TokenType::Doctype:
    os << "Doctype";
    break;
  default:
    UNIMPLEMENTED();
  }
  return os;
}

std::string Token::dump() const {
  std::stringstream ss;
  ss << "Token(" << m_type << ", \"" << m_data << "\"";
  for (const auto &attr : m_attributes) {
    ss << ", " << attr.name << "=" << attr.value;
  }
  ss << ")";
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
      {
          {State::Data, [this]() { handle_data(); }},
          {State::TagOpen, [this]() { handle_tag_open(); }},
          {State::TagName, [this]() { handle_tag_name(); }},
          {State::EndTagOpen, [this]() { handle_end_tag_open(); }},
          {State::MarkupDeclarationOpen,
           [this]() { handle_markup_declaration_open(); }},
          {State::Doctype, [this]() { handle_doctype(); }},
          {State::BeforeDoctypeName,
           [this]() { handle_before_doctype_name(); }},
          {State::DoctypeName, [this]() { handle_doctype_name(); }},
          {State::BeforeAttributeName,
           [this]() { handle_before_attribute_name(); }},
          {State::AttributeName, [this]() { handle_attribute_name(); }},
          {State::AfterAttributeName,
           [this]() { handle_after_attribute_name(); }},
          {State::BeforeAttributeValue,
           [this]() { handle_before_attribute_value(); }},
          {State::AttributeValueDoubleQuoted,
           [this]() { handle_attribute_value_double_quoted(); }},
          {State::AfterAttributeValueQuoted,
           [this]() { handle_after_attribute_value_quoted(); }},
      };

  auto it = state_handlers.find(m_state);
  assert(it != state_handlers.end());
  it->second();
}

// https://html.spec.whatwg.org/multipage/parsing.html#data-state
void Tokenizer::handle_data() {
  char c = consume();
  if (c == '<') {
    m_state = State::TagOpen;
  } else {
    m_tokens.emplace_back(TokenType::Character, std::string(1, c));
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#tag-open-state
void Tokenizer::handle_tag_open() {
  char c = consume();
  if (c == '!') {
    m_state = State::MarkupDeclarationOpen;
  } else if (c == '/') {
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
void Tokenizer::handle_tag_name() {
  char c = consume();
  if (c == '>') {
    m_state = State::Data;
  } else if (c == '\t' || c == '\n' || c == ' ') {
    m_state = State::BeforeAttributeName;
  } else {
    current_token().data() += c;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#end-tag-open-state
void Tokenizer::handle_end_tag_open() {
  char c = consume();
  if (std::isalpha(c) != 0) {
    m_tokens.emplace_back(TokenType::EndTag, "");
    m_current--;
    m_state = State::TagName;
  } else {
    UNIMPLEMENTED();
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#markup-declaration-open-state
void Tokenizer::handle_markup_declaration_open() {
  if (std::toupper(peek(0)) == 'D' && std::toupper(peek(1)) == 'O' &&
      std::toupper(peek(2)) == 'C' && std::toupper(peek(3)) == 'T' &&
      std::toupper(peek(4)) == 'Y' && std::toupper(peek(5)) == 'P' &&
      std::toupper(peek(6)) == 'E') {
    m_current += 7;
    m_state = State::Doctype;
  } else {
    UNIMPLEMENTED();
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#doctype-state
void Tokenizer::handle_doctype() {
  char c = consume();
  if (c == ' ') {
    m_state = State::BeforeDoctypeName;
  } else {
    UNIMPLEMENTED();
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#before-doctype-name-state
void Tokenizer::handle_before_doctype_name() {
  char c = consume();
  if (std::isalpha(c) != 0) {
    m_tokens.emplace_back(TokenType::Doctype, "");
    m_current--;
    m_state = State::DoctypeName;
  } else {
    UNIMPLEMENTED();
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#doctype-name-state
void Tokenizer::handle_doctype_name() {
  char c = consume();
  if (c == '>') {
    m_state = State::Data;
  } else {
    current_token().data() += c;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#before-attribute-name-state
void Tokenizer::handle_before_attribute_name() {
  char c = consume();
  if (c == ' ' || c == '\t' || c == '\n') {
    // ignore
  } else if (c == '/' || c == '>') {
    m_current--;
    m_state = State::AfterAttributeName;
  } else {
    current_token().attributes().push_back(Attribute{});
    m_current--;
    m_state = State::AttributeName;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#attribute-name-state
void Tokenizer::handle_attribute_name() {
  char c = consume();
  if (c == '\t' || c == '\n' || c == ' ' || c == '/' || c == '>') {
    m_current--;
    m_state = State::AfterAttributeName;
  } else if (c == '=') {
    m_state = State::BeforeAttributeValue;
  } else if (c == '>') {
    m_state = State::Data;
  } else {
    current_token().attributes().back().name += c;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#after-attribute-name-state
void Tokenizer::handle_after_attribute_name() {
  char c = consume();
  if (c == '\t' || c == '\n' || c == ' ') {
    // ignore
  } else if (c == '=') {
    m_state = State::BeforeAttributeValue;
  } else if (c == '>') {
    m_state = State::Data;
  } else {
    UNIMPLEMENTED();
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#before-attribute-value-state
void Tokenizer::handle_before_attribute_value() {
  char c = consume();
  if (c == ' ' || c == '\t' || c == '\n') {
    // ignore
  } else if (c == '"') {
    m_state = State::AttributeValueDoubleQuoted;
  } else {
    UNIMPLEMENTED();
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#attribute-value-(double-quoted)-state
void Tokenizer::handle_attribute_value_double_quoted() {
  char c = consume();
  if (c == '"') {
    m_state = State::AfterAttributeValueQuoted;
  } else {
    current_token().attributes().back().value += c;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#after-attribute-value-(quoted)-state
void Tokenizer::handle_after_attribute_value_quoted() {
  char c = consume();
  if (c == ' ' || c == '\t' || c == '\n') {
    m_state = State::BeforeAttributeName;
  } else if (c == '>') {
    m_state = State::Data;
  } else {
    UNIMPLEMENTED();
  }
}