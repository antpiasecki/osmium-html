#include "tokenizer.hh"
#include <cassert>
#include <fstream>
#include <iostream>
#include <span>
#include <sstream>

int main(int argc, char *argv[]) {
  auto args = std::span(argv, static_cast<size_t>(argc));

  std::ifstream file(args[1]);
  assert(file.good());

  std::stringstream ss;
  ss << file.rdbuf();

  Tokenizer parser(ss.str());
  for (const auto &token : parser.parse()) {
    std::cout << token.dump() << std::endl;
  }
}