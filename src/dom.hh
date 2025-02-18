#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class Node {
public:
  virtual ~Node() = default;
  virtual std::string dump() = 0;
};

class Element : public Node {
public:
  explicit Element(std::string name) : m_name(std::move(name)) {}

  [[nodiscard]] std::string name() const { return m_name; }
  [[nodiscard]] const std::unordered_map<std::string, std::string> &
  attributes() const {
    return m_attributes;
  }
  [[nodiscard]] const std::vector<std::shared_ptr<Node>> &children() const {
    return m_children;
  }

  std::string dump() override {
    std::stringstream ss;
    ss << "Element(\"" << m_name << "\", children: [";
    // TODO: attributes
    for (const auto &e : m_children) {
      ss << e->dump() << ",";
    }
    ss << "])";
    return ss.str();
  }

private:
  std::string m_name;
  std::unordered_map<std::string, std::string> m_attributes;
  std::vector<std::shared_ptr<Node>> m_children;
};

class TextNode : public Node {
public:
  explicit TextNode(std::string content) : m_content(std::move(content)) {}

  [[nodiscard]] std::string content() const { return m_content; }

  std::string dump() override {
    std::stringstream ss;
    ss << "TextNode(\"" << m_content << "\")";
    return ss.str();
  }

private:
  std::string m_content;
};