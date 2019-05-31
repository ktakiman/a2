#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

constexpr std::size_t INDENT_UNIT = 2;

namespace a2 {

enum class EBlockType {
  None,
  Constants,
  Table,
  Code
};

enum class ConstantsType {
  ValueOnly,
  HasChildren,
  HasBitsInfo
};

enum class ERefedType {
  kNone,
  kConst,
  kAddr,
  kNum
};

enum class ERefedOp {
  kNone,
  kAdd,
  kSubtract
};

extern std::unordered_map<ERefedType, std::string> gRefedTypeToStr;
extern std::unordered_map<ERefedOp, std::string> gRefedOpToStr;
extern std::unordered_map<ERefedOp, std::string> gRefedOpToChar;

struct BitsInfo {
  std::string name;
  unsigned int size = 0;
};

struct ConstantsData {
  std::string name;
  unsigned int value = 0;;
  std::unordered_map<std::string, std::unique_ptr<ConstantsData>> children;
  std::vector<BitsInfo> bits_info;
};

struct Refed {
  ERefedType type = ERefedType::kNone;
  ERefedOp op = ERefedOp::kNone;
  std::string ref;
  unsigned int num = 0;

  Refed() = default;
  Refed(const std::string& ref, ERefedOp op = ERefedOp::kNone);
  Refed(unsigned int num, ERefedOp op = ERefedOp::kNone);
};

struct NamedRef {
  std::string name;
  std::vector<Refed> value;
  unsigned int indent = 0;
};

struct Instruction {
  std::string tag;
  std::string func;
  std::vector<std::vector<Refed>> args;
  unsigned int indent = 0;
};

struct A2 {
  std::unordered_map<std::string, std::unique_ptr<ConstantsData>> constants;
  std::vector<NamedRef> table;
  std::vector<Instruction> instructions;
};

}
