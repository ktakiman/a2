#pragma once

#include <string>

namespace a2 {

struct NamedValue {
  std::string name;
  unsigned int value = 0;
  std::size_t indent = 0;

  bool operator==(const NamedValue& other);
};


NamedValue TokenizeNamedValue(const std::string& s); 

}

namespace a2test {
void TestTokenizer();
}
