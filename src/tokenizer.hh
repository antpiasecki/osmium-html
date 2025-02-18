#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#define UNIMPLEMENTED()                                                        \
  std::cerr << "UNIMPLEMENTED() reached on " << __FILE__ << ", line "          \
            << __LINE__ << "\n";                                               \
  exit(1)

enum class TokenType {
  StartTag,
  EndTag,
  Character,
  Doctype,
  Comment,
};

inline std::ostream &operator<<(std::ostream &os, const TokenType e) {
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
  case TokenType::Comment:
    os << "Comment";
    break;
  default:
    UNIMPLEMENTED();
  }
  return os;
}

class Token {
public:
  struct Attribute {
    std::string name;
    std::string value;
  };

  Token(TokenType type, std::string data)
      : m_type(type), m_data(std::move(data)) {}

  [[nodiscard]] TokenType type() const { return m_type; }
  [[nodiscard]] std::string &data() { return m_data; }
  [[nodiscard]] std::vector<Attribute> &attributes() { return m_attributes; }
  [[nodiscard]] bool is_self_closing() const { return m_is_self_closing; }
  void set_is_self_closing(bool v) { m_is_self_closing = v; }

  [[nodiscard]] std::string dump() const;

private:
  TokenType m_type;
  std::string m_data;
  std::vector<Attribute> m_attributes;
  bool m_is_self_closing = false;
};

class Tokenizer {
public:
  explicit Tokenizer(std::string data) : m_data(std::move(data)) {}

  std::vector<Token> parse();

private:
  enum class State {
    Data,
    TagOpen,
    TagName,
    EndTagOpen,
    MarkupDeclarationOpen,
    Doctype,
    BeforeDoctypeName,
    DoctypeName,
    AfterDoctypeName,
    AfterDoctypePublicKeyword,
    BeforeDoctypePublicIdentifier,
    DoctypePublicIdentifierDoubleQuoted,
    AfterDoctypePublicIdentifier,
    BeforeAttributeName,
    AttributeName,
    AfterAttributeName,
    BeforeAttributeValue,
    AttributeValueDoubleQuoted,
    AttributeValueUnquoted,
    AfterAttributeValueQuoted,
    CommentStart,
    Comment,
    CommentEndDash,
    CommentEnd,
    SelfClosingStartTag,
    ScriptData,
  };

  State m_state{State::Data};
  std::string m_data;
  size_t m_current = 0;
  std::vector<Token> m_tokens;

  void handle_data();
  void handle_tag_open();
  void handle_tag_name();
  void handle_end_tag_open();
  void handle_markup_declaration_open();
  void handle_doctype();
  void handle_before_doctype_name();
  void handle_doctype_name();
  void handle_after_doctype_name();
  void handle_after_doctype_public_keyword();
  void handle_before_doctype_public_identifier();
  void handle_doctype_public_identifier_double_quoted();
  void handle_after_doctype_public_identifier();
  void handle_before_attribute_name();
  void handle_attribute_name();
  void handle_after_attribute_name();
  void handle_before_attribute_value();
  void handle_attribute_value_double_quoted();
  void handle_attribute_value_unquoted();
  void handle_after_attribute_value_quoted();
  void handle_comment_start();
  void handle_comment();
  void handle_comment_end_dash();
  void handle_comment_end();
  void handle_self_closing_start_tag();
  void handle_script_data();

  [[nodiscard]] Token &current_token() { return m_tokens.back(); }
  char consume() { return m_data[m_current++]; }
  char peek(long i) {
    return m_data[static_cast<size_t>(static_cast<long>(m_current) + i)];
  }
  [[nodiscard]] bool eof() const { return m_current >= m_data.length(); }
};