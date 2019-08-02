#include "assembler.h"

#include <vector>

#include "tokenizer.h"

namespace {

using namespace a2;

a2::Bits MakeThumbInstruction(unsigned int code, const std::string& tag = "") {
  a2::Bits bits{};
  bits.tag = 
  bits.size = 2;
  bits.value = code;
  return bits;
}

std::size_t FetchConstantValue(const std::string& s, const A2& a2) {
  const ConstantsData* constants = nullptr;
  for (auto& token : TokenizeConstRef(s)) {
    if (constants == nullptr) {
      for (auto& pair : a2.constants) {
        auto itr = pair.second->children.find(token);
        if (itr != pair.second->children.end()) {
          constants = itr->second.get();
          break;
        }
      }
    } else {
      auto itr = constants->children.find(token);
      if (itr != constants->children.end()) {
        constants = itr->second.get();
      }
    }
  }

  // throw here if constant was not found???

  return constants != nullptr ? constants->value : 0;
}

using namespace a2;
void DumpBits(const std::vector<Bits>& a2s) {
  std::cout << std::endl << "--- bits ---" << std::endl;

  for (auto& a2 : a2s) {
    std::cout << a2.tag << ": sz = " << a2.size << ", ";
    if (a2.resolved) {
      std::cout << a2.value;
    } else {
      std::cout << "[" << a2.link << "]";
    }
    std::cout << std::endl; 
  }
}

}

namespace a2 {

void AssembleTable(const A2& a2, std::vector<Bits>& bits) {
  for (const auto& entry : a2.table) {
    Bits bit{};
    bit.tag = entry.name;
    bit.size = 4;

    /*
    if (entry.dynamic.empty()) {
      bit.resolved = true;
      bit.value = entry.value;
    } else {
      bit.link = entry.dynamic;
    }
    bits.push_back(bit);
    */
  }

  // default implementation of reset

}

void Assemble(const A2& a2, std::ostream& binary) {
  std::vector<Bits> bits;
  AssembleTable(a2, bits);

  DumpBits(bits);
}

}

