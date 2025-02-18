#include "tokenizer.hh"
#include <fstream>
#include <iostream>
#include <sstream>

int main() {
  std::ifstream file("test.html");
  std::stringstream ss;
  ss << file.rdbuf();

  Tokenizer parser(ss.str());
  for (const auto &token : parser.parse()) {
    std::cout << token.dump() << std::endl;
  }
}