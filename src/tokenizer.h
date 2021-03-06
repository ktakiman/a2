#pragma once

#include <string>
#include <vector>
#include <tuple>

#include "types.h"

namespace a2 {

struct NamedConstant {
  std::string name;
  unsigned int value = 0;
  std::size_t indent = 0;
};

NamedConstant TokenizeNamedConstant(const std::string& s); 

NamedRef TokenizeNamedRef(const std::string& s);

Instruction TokenizeInstruction(const std::string& s);

std::tuple<bool, std::string> TryTokenizeNamedTag(const std::string& s);

std::vector<std::string> TokenizeConstRef(const std::string& s);

}

namespace a2test {
void TestTokenizer();
}
