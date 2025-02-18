#pragma once

#include <memory>
#include <utility>

#include "dom.hh"

enum class State {
  Data,
  TagOpen,
  TagName,
  EndTagOpen,
};

class Parser {
public:
  explicit Parser(std::string data) : m_data(std::move(data)) {}

  std::shared_ptr<Node> parse();

private:
  State m_state{State::Data};
  std::string m_data;
  size_t m_current = 0;
  std::string m_current_tag_name;
  std::string m_text;

  void step();
  void step_data();
  void step_tag_open();
  void step_tag_name();
  void step_end_tag_open();

  char consume() { return m_data[m_current++]; }
  [[nodiscard]] bool eof() const { return m_current >= m_data.length(); }
};