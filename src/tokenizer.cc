#include "tokenizer.hh"
#include <cassert>
#include <functional>

std::vector<Token> Tokenizer::parse() {
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
          {State::AfterDoctypeName, [this]() { handle_after_doctype_name(); }},
          {State::AfterDoctypePublicKeyword,
           [this]() { handle_after_doctype_public_keyword(); }},
          {State::BeforeDoctypePublicIdentifier,
           [this]() { handle_before_doctype_public_identifier(); }},
          {State::DoctypePublicIdentifierDoubleQuoted,
           [this]() { handle_doctype_public_identifier_double_quoted(); }},
          {State::AfterDoctypePublicIdentifier,
           [this]() { handle_after_doctype_public_identifier(); }},
          {State::BeforeAttributeName,
           [this]() { handle_before_attribute_name(); }},
          {State::AttributeName, [this]() { handle_attribute_name(); }},
          {State::AfterAttributeName,
           [this]() { handle_after_attribute_name(); }},
          {State::BeforeAttributeValue,
           [this]() { handle_before_attribute_value(); }},
          {State::AttributeValueDoubleQuoted,
           [this]() { handle_attribute_value_double_quoted(); }},
          {State::AttributeValueSingleQuoted,
           [this]() { handle_attribute_value_single_quoted(); }},
          {State::AttributeValueUnquoted,
           [this]() { handle_attribute_value_unquoted(); }},
          {State::AfterAttributeValueQuoted,
           [this]() { handle_after_attribute_value_quoted(); }},
          {State::CommentStart, [this]() { handle_comment_start(); }},
          {State::CommentStartDash, [this]() { handle_comment_start_dash(); }},
          {State::Comment, [this]() { handle_comment(); }},
          {State::CommentLessThanSign,
           [this]() { handle_comment_less_than_sign(); }},
          {State::CommentLessThanSignBang,
           [this]() { handle_comment_less_than_sign_bang(); }},
          {State::CommentLessThanSignBangDash,
           [this]() { handle_comment_less_than_sign_bang_dash(); }},
          {State::CommentLessThanSignBangDashDash,
           [this]() { handle_comment_less_than_sign_bang_dash_dash(); }},
          {State::CommentEndDash, [this]() { handle_comment_end_dash(); }},
          {State::CommentEnd, [this]() { handle_comment_end(); }},
          {State::SelfClosingStartTag,
           [this]() { handle_self_closing_start_tag(); }},
          {State::ScriptData, [this]() { handle_script_data(); }},
          {State::StyleData, [this]() { handle_style_data(); }},
      };

  while (!eof()) {
    auto it = state_handlers.find(m_state);
    assert(it != state_handlers.end());
    it->second();
  }
  return m_tokens;
}

// https://html.spec.whatwg.org/multipage/parsing.html#data-state
void Tokenizer::handle_data() {
  char c = consume();
  // TODO: handle entities
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
  } else if (c == '?') {
    UNIMPLEMENTED();
  } else {
    UNIMPLEMENTED();
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#tag-name-state
void Tokenizer::handle_tag_name() {
  char c = consume();
  if (c == '>') {
    if (current_token().type() == TokenType::StartTag &&
        current_token().data() == "script") {
      m_state = State::ScriptData;
    } else if (current_token().type() == TokenType::StartTag &&
               current_token().data() == "style") {
      m_state = State::StyleData;
    } else {
      m_state = State::Data;
    }
  } else if (c == '/') {
    m_state = State::SelfClosingStartTag;
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
  } else if (c == '>') {
    UNIMPLEMENTED();
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
  } else if (std::toupper(peek(0)) == '-' && std::toupper(peek(1)) == '-') {
    m_current += 2;
    m_tokens.emplace_back(TokenType::Comment, "");
    m_state = State::CommentStart;
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
  if (c == ' ' || c == '\t' || c == '\n') {
    m_state = State::AfterDoctypeName;
  } else if (c == '>') {
    m_state = State::Data;
  } else {
    current_token().data() += c;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#after-doctype-name-state
void Tokenizer::handle_after_doctype_name() {
  char c = consume();
  if (c == ' ' || c == '\t' || c == '\n') {
    // ignore
  } else if (c == '>') {
    m_state = State::Data;
  } else if (std::toupper(peek(-1)) == 'P' && std::toupper(peek(0)) == 'U' &&
             std::toupper(peek(1)) == 'B' && std::toupper(peek(2)) == 'L' &&
             std::toupper(peek(3)) == 'I' && std::toupper(peek(4)) == 'C') {
    m_current += 5;
    m_state = State::AfterDoctypePublicKeyword;
  } else {
    UNIMPLEMENTED();
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#after-doctype-public-keyword-state
void Tokenizer::handle_after_doctype_public_keyword() {
  char c = consume();
  if (c == ' ' || c == '\t' || c == '\n') {
    m_state = State::BeforeDoctypePublicIdentifier;
  } else {
    UNIMPLEMENTED();
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#before-doctype-public-identifier-state
void Tokenizer::handle_before_doctype_public_identifier() {
  char c = consume();
  if (c == ' ' || c == '\t' || c == '\n') {
    // ignore
  } else if (c == '"') {
    m_state = State::DoctypePublicIdentifierDoubleQuoted;
  } else if (c == '\'') {
    UNIMPLEMENTED();
  } else if (c == '>') {
    UNIMPLEMENTED();
  } else {
    UNIMPLEMENTED();
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#before-doctype-public-identifier-state
void Tokenizer::handle_doctype_public_identifier_double_quoted() {
  char c = consume();
  if (c == '"') {
    m_state = State::AfterDoctypePublicIdentifier;
  } else {
    // TODO: >Append the current input character to the current DOCTYPE token's
    // public identifier.
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#after-doctype-public-identifier-state
void Tokenizer::handle_after_doctype_public_identifier() {
  char c = consume();
  if (c == ' ' || c == '\t' || c == '\n') {
    UNIMPLEMENTED();
  } else if (c == '"') {
    UNIMPLEMENTED();
  } else if (c == '\'') {
    UNIMPLEMENTED();
  } else if (c == '>') {
    m_state = State::Data;
  } else {
    UNIMPLEMENTED();
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
  } else if (c == '=') {
    UNIMPLEMENTED();
  } else {
    current_token().attributes().push_back(Token::Attribute{});
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
  } else if (c == '"' || c == '\'' || c == '<') {
    UNIMPLEMENTED();
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
  } else if (c == '/') {
    m_state = State::SelfClosingStartTag;
  } else if (c == '>') {
    m_state = State::Data;
  } else {
    current_token().attributes().push_back(Token::Attribute{});
    m_current--;
    m_state = State::AttributeName;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#before-attribute-value-state
void Tokenizer::handle_before_attribute_value() {
  char c = consume();
  if (c == ' ' || c == '\t' || c == '\n') {
    // ignore
  } else if (c == '"') {
    m_state = State::AttributeValueDoubleQuoted;
  } else if (c == '\'') {
    m_state = State::AttributeValueSingleQuoted;
  } else if (c == '>') {
    UNIMPLEMENTED();
  } else {
    m_state = State::AttributeValueUnquoted;
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

// https://html.spec.whatwg.org/multipage/parsing.html#attribute-value-(single-quoted)-state
void Tokenizer::handle_attribute_value_single_quoted() {
  char c = consume();
  if (c == '\'') {
    m_state = State::AfterAttributeValueQuoted;
  } else {
    current_token().attributes().back().value += c;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#attribute-value-(unquoted)-state
void Tokenizer::handle_attribute_value_unquoted() {
  char c = consume();
  if (c == ' ' || c == '\t' || c == '\n') {
    m_state = State::BeforeAttributeName;
  } else if (c == '>') {
    if (current_token().type() == TokenType::StartTag &&
        current_token().data() == "script") {
      m_state = State::ScriptData;
    } else if (current_token().type() == TokenType::StartTag &&
               current_token().data() == "style") {
      m_state = State::StyleData;
    } else {
      m_state = State::Data;
    }
  } else {
    current_token().attributes().back().value += c;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#after-attribute-value-(quoted)-state
void Tokenizer::handle_after_attribute_value_quoted() {
  char c = consume();
  if (c == ' ' || c == '\t' || c == '\n') {
    m_state = State::BeforeAttributeName;
  } else if (c == '/') {
    m_state = State::SelfClosingStartTag;
  } else if (c == '>') {
    if (current_token().type() == TokenType::StartTag &&
        current_token().data() == "script") {
      m_state = State::ScriptData;
    } else if (current_token().type() == TokenType::StartTag &&
               current_token().data() == "style") {
      m_state = State::StyleData;
    } else {
      m_state = State::Data;
    }
  } else {
    UNIMPLEMENTED();
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#comment-start-state
void Tokenizer::handle_comment_start() {
  char c = consume();
  if (c == '-') {
    m_state = State::CommentStartDash;
  } else if (c == '>') {
    UNIMPLEMENTED();
  } else {
    m_current--;
    m_state = State::Comment;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#comment-start-dash-state
void Tokenizer::handle_comment_start_dash() {
  char c = consume();
  if (c == '-') {
    m_state = State::CommentEnd;
  } else if (c == '>') {
    UNIMPLEMENTED();
  } else {
    current_token().data() += "-";
    m_current--;
    m_state = State::Comment;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#comment-state
void Tokenizer::handle_comment() {
  char c = consume();
  if (c == '<') {
    current_token().data() += c;
    m_state = State::CommentLessThanSign;
  } else if (c == '-') {
    m_state = State::CommentEndDash;
  } else {
    current_token().data() += c;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#comment-less-than-sign-state
void Tokenizer::handle_comment_less_than_sign() {
  char c = consume();
  if (c == '<') {
    current_token().data() += c;
  } else if (c == '!') {
    m_state = State::CommentLessThanSignBang;
  } else {
    m_current--;
    m_state = State::Comment;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#comment-less-than-sign-bang-state
void Tokenizer::handle_comment_less_than_sign_bang() {
  char c = consume();
  if (c == '-') {
    m_state = State::CommentLessThanSignBangDash;
  } else {
    m_current--;
    m_state = State::Comment;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#comment-less-than-sign-bang-dash-state
void Tokenizer::handle_comment_less_than_sign_bang_dash() {
  char c = consume();
  if (c == '-') {
    m_state = State::CommentLessThanSignBangDashDash;
  } else {
    m_current--;
    m_state = State::Comment;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#comment-less-than-sign-bang-dash-dash-state
void Tokenizer::handle_comment_less_than_sign_bang_dash_dash() {
  char c = consume();
  if (c == '>') {
    m_state = State::CommentEnd;
  } else {
    // TODO: this is an error but ebay uses it
    m_current--;
    m_state = State::CommentEnd;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#comment-end-dash-state
void Tokenizer::handle_comment_end_dash() {
  char c = consume();
  if (c == '-') {
    m_state = State::CommentEnd;
  } else {
    current_token().data() += "-";
    m_current--;
    m_state = State::Comment;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#comment-end-state
void Tokenizer::handle_comment_end() {
  char c = consume();
  if (c == '>') {
    m_state = State::Data;
  } else if (c == '!') {
    UNIMPLEMENTED();
  } else if (c == '-') {
    current_token().data() += c;
  } else {
    current_token().data() += "-";
    current_token().data() += "-";
    m_current--;
    m_state = State::Comment;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#self-closing-start-tag-state
void Tokenizer::handle_self_closing_start_tag() {
  char c = consume();
  if (c == '>') {
    current_token().set_is_self_closing(true);
    m_state = State::Data;
  } else {
    UNIMPLEMENTED();
  }
}

// not in the spec and very buggy but its the easiest way to do this. im sorry
void Tokenizer::handle_script_data() {
  if (std::toupper(peek(0)) == '<' && std::toupper(peek(1)) == '/' &&
      std::toupper(peek(2)) == 'S' && std::toupper(peek(3)) == 'C' &&
      std::toupper(peek(4)) == 'R' && std::toupper(peek(5)) == 'I' &&
      std::toupper(peek(6)) == 'P' && std::toupper(peek(7)) == 'T' &&
      std::toupper(peek(8)) == '>') {
    m_state = State::Data;
  } else {
    char c = consume();
    m_tokens.emplace_back(TokenType::Character, std::string(1, c));
  }
}

void Tokenizer::handle_style_data() {
  if (std::toupper(peek(0)) == '<' && std::toupper(peek(1)) == '/' &&
      std::toupper(peek(2)) == 'S' && std::toupper(peek(3)) == 'T' &&
      std::toupper(peek(4)) == 'Y' && std::toupper(peek(5)) == 'L' &&
      std::toupper(peek(6)) == 'E' && std::toupper(peek(7)) == '>') {
    m_state = State::Data;
  } else {
    char c = consume();
    m_tokens.emplace_back(TokenType::Character, std::string(1, c));
  }
}