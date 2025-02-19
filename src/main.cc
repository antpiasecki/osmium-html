#include "parser.hh"
#include "tokenizer.hh"
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

int main(int /*argc*/, char *argv[]) {
  std::ifstream file(argv[1]);
  assert(file.good());

  std::stringstream ss;
  ss << file.rdbuf();

  Tokenizer tokenizer(ss.str());
  auto tokens = tokenizer.parse();

  Parser parser(tokens);
  std::cout << parser.parse()->dump(0) << std::endl;
}