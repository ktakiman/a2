#include "types.h"

namespace a2 {
  
Refed::Refed(const std::string& ref, ERefedOp op) {
  if (ref[0] == '@') {
    this->ref = ref.substr(1);
    this->type = ERefedType::kAddr;
  } else {
    this->ref = ref;
    this->type = ERefedType::kConst;
  }
  this->op = op;
}

Refed::Refed(unsigned int num, ERefedOp op) {
  this->num = num;
  this->op = op;
  this->type = ERefedType::kNum;
}

}

