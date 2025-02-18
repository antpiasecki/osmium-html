#pragma once

#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

static std::string escape(const std::string &s) {
  auto out = s;
  out = std::regex_replace(out, std::regex("\n"), "\\n");
  out = std::regex_replace(out, std::regex("\""), "\\\"");
  return out;
}

class Node {
public:
  virtual ~Node() = default;
  virtual std::string dump() = 0;
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

  std::string dump() override {
    std::stringstream ss;
    ss << "Element(\"" << m_name << "\", attributes={";
    for (const auto &a : m_attributes) {
      ss << "\"" << escape(a.first) << "\":\"" << escape(a.second) << "\",";
    }
    ss << "}, children=[";
    for (const auto &e : m_children) {
      ss << e->dump() << ",";
    }
    ss << "])";
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

  std::string dump() override {
    return "TextNode(\"" + escape(m_content) + "\")";
  }

private:
  std::string m_content;
};