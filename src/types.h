#ifndef __TYPES_H
#define __TYPES_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

constexpr int INDENT_UNIT = 2;

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

struct BitsInfo {
  std::string name;
  unsigned int size;
};

struct ConstantsData {
  std::string name;
  unsigned int value;
  std::unordered_map<std::string, std::unique_ptr<ConstantsData>> children;
  std::vector<BitsInfo> bits_info;
};

struct TableEntry {
  std::string name;
  unsigned int value;
  std::string dynamic;  // takes the address of a function with name specified by this
};

struct Instruction {
  std::string func;
  std::vector<std::string> args;
};

struct A2 {
  std::unordered_map<std::string, std::unique_ptr<ConstantsData>> constants;
  std::vector<TableEntry> table;
  std::vector<Instruction> instructions;
};

}

#endif
