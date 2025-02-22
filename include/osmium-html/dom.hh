#pragma once

#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
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
  [[nodiscard]] virtual bool is_element() const = 0;
  virtual std::string dump(size_t i) = 0;
};

using NodePtr = std::shared_ptr<Node>;

class Element : public Node {
public:
  using Attributes = std::unordered_map<std::string, std::string>;

  explicit Element(std::string name) : m_name(std::move(name)) {}

  [[nodiscard]] std::string name() const { return m_name; }
  [[nodiscard]] Attributes &attributes() { return m_attributes; }
  [[nodiscard]] const std::vector<NodePtr> &children() const {
    return m_children;
  }

  [[nodiscard]] bool is_heading() const {
    static const std::set<std::string_view> tags = {"h1", "h2", "h3",
                                                    "h4", "h5", "h6"};
    return tags.find(m_name) != tags.end();
  }

  void append(const NodePtr &child) { m_children.emplace_back(child); }

  [[nodiscard]] bool is_element() const override { return true; }

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
  std::vector<NodePtr> m_children;
};

using ElementPtr = std::shared_ptr<Element>;

class TextNode : public Node {
public:
  explicit TextNode(std::string content) : m_content(std::move(content)) {}

  [[nodiscard]] std::string content() const { return m_content; }

  [[nodiscard]] bool is_element() const override { return false; }

  std::string dump(size_t i) override {
    std::stringstream ss;
    ss << std::string(2 * i, ' ') << "- \"" + escape(m_content) << "\"\n";
    return ss.str();
  }

private:
  std::string m_content;
};

using TextNodePtr = std::shared_ptr<TextNode>;