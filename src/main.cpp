#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "types.h"
#include "parser.h"
#include "assembler.h"

using namespace a2;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "usage: a2.exe [input file]" << std::endl;
    return 0;
  }

  std::ifstream fs(argv[1]);
  if (fs.is_open()) {
    auto a2 = ParseA2(fs);
    DumpA2(*a2.get());

    std::stringstream ss;
    Assemble(*a2.get(), ss);
  }
  else {
	  std::cout << "cannot find file: " << argv[1] << std::endl;
  }
}
