#include "types.h"

namespace a2 {
  
std::unordered_map<ERefedType, std::string> gRefedTypeToStr = {
  { ERefedType::kNone, "kNone" },
  { ERefedType::kConst, "kConst" },
  { ERefedType::kAddr, "kAddr"},
  { ERefedType::kNum, "kNum"}};

std::unordered_map<ERefedOp, std::string> gRefedOpToStr {
  { ERefedOp::kNone, "kNone" },
  { ERefedOp::kAdd, "kAdd" },
  { ERefedOp::kSubtract, "kSubtract" }};

std::unordered_map<ERefedOp, std::string> gRefedOpToChar {
  { ERefedOp::kNone, "" },
  { ERefedOp::kAdd, "+" },
  { ERefedOp::kSubtract, "-" }};

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

