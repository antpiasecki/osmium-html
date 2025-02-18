#include "parser.hh"
#include <fstream>
#include <iostream>

int main() {
  std::ifstream file("test.html");
  std::stringstream ss;
  ss << file.rdbuf();

  Parser parser(ss.str());
  std::cout << parser.parse() << std::endl;
}