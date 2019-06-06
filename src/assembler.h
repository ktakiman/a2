#pragma once

#include <iostream>
#include <string>

#include "types.h"

namespace a2 {

struct Bits {
  int size;             // in bytes
  unsigned int value;   // assume 32 bits max 
  bool resolved;
  std::string link;     // points to other piece (for address)
  std::string tag;      // lets other piece reference this piece
};

void Assemble(const A2& a2, std::ostream& binary);

}
