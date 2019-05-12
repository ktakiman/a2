#include "assembler.h"

#include <vector>

namespace {

a2::Piece MakeThumbInstruction(unsigned int code, const std::string& tag = "") {
  a2::Piece p{};
  p.tag = 
  p.size = 2;
  p.value = code;
  return p;
}

using namespace a2;
void DumpPieces(const std::vector<Piece>& a2s) {
  std::cout << std::endl << "--- pieces ---" << std::endl;

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

void AssembleTable(const A2& a2, std::vector<Piece>& pieces) {
  for (const auto& entry : a2.table) {
    Piece piece{};
    piece.tag = entry.name;
    piece.size = 4;

    if (entry.dynamic.empty()) {
      piece.resolved = true;
      piece.value = entry.value;
    } else {
      piece.link = entry.dynamic;
    }
    pieces.push_back(piece);
  }

  // default implementation of reset

}

void Assemble(const A2& a2, std::ostream& binary) {
  std::vector<Piece> pieces;
  AssembleTable(a2, pieces);

  DumpPieces(pieces);
}

}

