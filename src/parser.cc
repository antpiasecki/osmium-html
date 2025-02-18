#include "parser.hh"
#include "dom.hh"
#include <cassert>
#include <cctype>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>

#define UNIMPLEMENTED()                                                        \
  std::cerr << "UNIMPLEMENTED() reached on " << __FILE__ << ", line "          \
            << __LINE__ << "\n";                                               \
  exit(1)

std::shared_ptr<Node> Parser::parse() {
  while (!eof()) { // TODO: handle EOF
    std::cout << "stepping\n";
    step();
  }
  std::cout << m_text << std::endl;
  return nullptr;
}

void Parser::step() {
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
void Parser::step_data() {
  char c = consume();
  if (c == '<') {
    m_state = State::TagOpen;
  } else {
    // TODO: we're supposed to emit each character as a token for some reason
    // >Emit the current input character as a character token.
    m_text += c;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#tag-open-state
void Parser::step_tag_open() {
  char c = consume();
  if (c == '/') {
    m_state = State::EndTagOpen;
  } else if (std::isalpha(c) != 0) {
    m_current_tag_name = "";
    m_current--;
    m_state = State::TagName;
  } else {
    UNIMPLEMENTED();
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#tag-name-state
void Parser::step_tag_name() {
  char c = consume();
  if (c == '>') {
    m_state = State::Data;
    std::cout << "emitting " << m_current_tag_name << std::endl;
  } else {
    m_current_tag_name += c;
  }
}

// https://html.spec.whatwg.org/multipage/parsing.html#end-tag-open-state
void Parser::step_end_tag_open() {
  char c = consume();
  if (std::isalpha(c) != 0) {
    m_current_tag_name = "";
    m_current--;
    m_state = State::TagName;
  } else {
    UNIMPLEMENTED();
  }
}