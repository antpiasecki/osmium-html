#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

static std::string escape(const std::string &s) {
  std::string out;
  out.reserve(s.size());

  for (char c : s) {
    switch (c) {
    case '\n':
      out += "\\n";
      break;
    case '\"':
      out += "\\\"";
      break;
    default:
      out += c;
      break;
    }
  }

  return out;
}

class Node {
public:
  virtual ~Node() = default;
  virtual std::string dump(size_t i) = 0;
};

class Element : public Node {
public:
  using Attributes = std::unordered_map<std::string, std::string>;

  explicit Element(std::string name) : m_name(std::move(name)) {}

  [[nodiscard]] std::string name() const { return m_name; }
  [[nodiscard]] Attributes &attributes() { return m_attributes; }
  [[nodiscard]] const std::vector<std::shared_ptr<Node>> &children() const {
    return m_children;
  }

  void append(const std::shared_ptr<Node> &child) {
    m_children.emplace_back(child);
  }

  std::string dump(size_t i) override {
    std::stringstream ss;
    ss << std::string(2 * i, ' ') << "- " << m_name;
    for (const auto &a : m_attributes) {
      ss << " " << escape(a.first) << "=\"" << escape(a.second) << "\"";
    }
    ss << "\n";
    for (const auto &e : m_children) {
      ss << e->dump(i + 2);
    }
    return ss.str();
  }

private:
  std::string m_name;
  Attributes m_attributes;
  std::vector<std::shared_ptr<Node>> m_children;
};

class TextNode : public Node {
public:
  explicit TextNode(std::string content) : m_content(std::move(content)) {}

  [[nodiscard]] std::string content() const { return m_content; }

  std::string dump(size_t i) override {
    std::stringstream ss;
    ss << std::string(2 * i, ' ') << "- \"" + escape(m_content) << "\"\n";
    return ss.str();
  }

private:
  std::string m_content;
};