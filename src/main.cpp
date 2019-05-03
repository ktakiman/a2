#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <memory>
#include <unordered_map>
#include <iomanip>

#include "types.h"
#include "parser.h"

using namespace a2;

int main(int argc, char* argv[]) {
  if (argc == 0) {
    std::cout << "usage: a2.exe [input file]" << std::endl;
    return 0;
  }

  std::ifstream fs(argv[1]);
  auto a2 = ParseA2(fs);

  //DumpA2(*a2.get());
}
