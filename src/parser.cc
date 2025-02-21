#include "parser.hh"
#include "tokenizer.hh"
#include <cassert>
#include <memory>

// TODO: actually implement the spec
std::shared_ptr<Node> Parser::parse() {
  auto root = std::make_shared<Element>("root");
  m_open_elements.push(root);

  while (!eof()) {
    auto t = consume();

    switch (t.type()) {
    case TokenType::StartTag: {
      if (!text.empty()) {
        if (current_node()->name() != "head") {
          current_node()->append(std::make_shared<TextNode>(text));
        }
        text = "";
      }

      auto el = std::make_shared<Element>(t.data());

      for (const auto &attr : t.attributes()) {
        el->attributes()[attr.name] = attr.value;
      }

      current_node()->append(el);

      if (!t.is_self_closing() && !is_void_element(t.data())) {
        m_open_elements.push(el);
      }
    }; break;
    case TokenType::EndTag:
      if (!text.empty()) {
        if (current_node()->name() != "head") {
          current_node()->append(std::make_shared<TextNode>(text));
        }
        text = "";
      }

      if (current_node()->name() != t.data()) {
        // TODO: we really should handle this but there is like a thousand
        // different insertion modes in the spec
      } else {
        m_open_elements.pop();
      }
      break;
    case TokenType::Character:
      text += t.data();
      break;
    case TokenType::Doctype:
      // TODO
      root->append(std::make_shared<Element>("DOCTYPE"));
      break;
    case TokenType::Comment:
      // TODO
      break;
    default:
      UNIMPLEMENTED();
    }
  }

  if (!text.empty()) {
    root->append(std::make_shared<TextNode>(text));
  }

  return root;
}

// https://html.spec.whatwg.org/multipage/syntax.html#void-elements
bool Parser::is_void_element(const std::string &name) {
  return name == "area" || name == "base" || name == "br" || name == "col" ||
         name == "embed" || name == "hr" || name == "img" || name == "input" ||
         name == "link" || name == "meta" || name == "source" ||
         name == "track" || name == "wbr";
}

std::shared_ptr<Node> parse(const std::string &s) {
  Tokenizer tokenizer(s);
  auto tokens = tokenizer.parse();

  Parser parser(tokens);
  return parser.parse();
}