#pragma once

#include <string>
#include <vector>

namespace a2 {

struct NamedConstant {
  std::string name;
  unsigned int value = 0;
  std::size_t indent = 0;
};

NamedConstant TokenizeNamedConstant(const std::string& s); 

enum class ERefedType {
  kNone,
  kConst,
  kAddr
};

enum class ERefedOp {
  kNone,
  kAdd,
  kSubtract
};

struct Refed {
  ERefedType type = ERefedType::kNone;
  ERefedOp op = ERefedOp::kNone;
  std::string ref;
};

struct NamedRef {
  std::string name;
  std::vector<Refed> refs;
  std::size_t indent = 0;
};

NamedRef TokenizeNamedRef(const std::string& s);

}

namespace a2test {
void TestTokenizer();
}
