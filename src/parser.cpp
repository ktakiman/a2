#include "parser.h"

#include <stack>
#include <numeric>
#include <regex>

#include <stdlib.h> // for atoi, which can be replaced with std::stoi (except minGW/gcc?)

namespace a2 {

//------------------------------------------------------------------------------
std::unique_ptr<ConstantsData> CreateConstantsData(const std::string& name, unsigned int value) {
  std::unique_ptr<ConstantsData> cd(new ConstantsData());
  cd->name = name;
  cd->value = value;
  return cd;
}
//------------------------------------------------------------------------------
bool LineFetcher::Next(std::string& next) {
  if (rewind_) {
    next = last_;
    rewind_ = false;
    return true;
  }

  while (!stream_.eof()) {
    std::getline(stream_, last_);

    auto from = last_.find_first_not_of(' ');    // tabs are assumed to be converted to spaces with preprocessing
    auto to = last_.find_last_not_of(' ');

    if (from == std::string::npos) { continue; }

    if (last_[from] == '\'') { continue; }   // comment starts with sigle quote, check first non-blank character
    
    last_ = last_.substr(0, to + 1);         // must leave spaces in front

    //std::cout << last_ << std::endl;

    next = last_;
    return true;
  }

  return false;
}
//------------------------------------------------------------------------------
bool BlockFetcher::Next(std::unique_ptr<BlockLinesFetcher>& blf) {
  std::string line;
  if (lf_.Next(line)) {
    if (IsBlockHeader(line)) {
      // TODO: swithc to make_unique after upgrading git/git-bash
      blf.reset(new BlockLinesFetcher(lf_));
      //blf = std::make_unique<BlockLinesFetcher>(lf_);

      std::size_t start = 0;

      switch (line[0]) {
        case '_':
          cur_block_type_ = EBlockType::Constants;
          start = 1;
          break;
        case '#':
          cur_block_type_ = EBlockType::Table;
          start = 1;
          break;
        default:
          cur_block_type_ = EBlockType::Code;
          break;
      }

      auto len = line.length() - start - (line.back() == ':' ? 1 : 0);
      cur_block_name_ = line.substr(start, len);
      return true;
    } else {
      // not expecting this case, throw exception?
    }
  }

  return false;
}
//------------------------------------------------------------------------------
bool BlockLinesFetcher::Next(std::string& line) {
  std::string temp;
  if (lf_.Next(temp)) {
    if (BlockFetcher::IsBlockHeader(temp)) {
      lf_.Rewind();
      return false;
    } else {
      line = temp;
      return true;
    }
  }

  return false;
}
//------------------------------------------------------------------------------
void ParseNameValueLine(const std::string& line, std::size_t* indent, std::string* name, std::string* value) {
  name->clear();
  *indent = 0;
  value->clear();

  // TODO: replace these with regex
  auto lead  = line.find_first_not_of(' ');

  if (lead != std::string::npos) {
    *indent = lead / INDENT_UNIT;

    auto colon = line.find_first_of(':');
    if (colon != std::string::npos) {
      *name = line.substr(lead, colon - lead);

      auto value_start = line.find_first_not_of(' ', colon + 1);
      if (value_start != std::string::npos) {
        *value = line.substr(value_start, line.length() - value_start);
      }
    }
  }
}
//------------------------------------------------------------------------------
void ParseNameValueLine(const std::string& line, std::size_t* indent, std::string* name, unsigned int* value) {
  std::string s;
  ParseNameValueLine(line, indent, name, &s);

  if (!s.empty()) {
    int base = (s.length() > 2 && s.substr(0, 2) == "0x") ? 16 : 10;
    *value = strtoul(s.c_str(), nullptr, base);
  }
}
//------------------------------------------------------------------------------
void ProcConstantsBlock(const std::string& block_name, BlockLinesFetcher& blf, A2& a2) {
  auto& root = a2.constants[block_name];
  if (!root) {
    root = CreateConstantsData(block_name);
  }

  std::size_t last_indent = 0;
  std::stack<ConstantsData*> stack;
  ConstantsData* parent = root.get();
  ConstantsData* last = root.get();

  std::string line;
  while (blf.Next(line)) {

    std::size_t indent = 0;
    unsigned int value = 0;
    std::string name;

    ParseNameValueLine(line, &indent, &name, &value);

    if (line[indent * INDENT_UNIT] == '.') { 
      BitsInfo bi;
      bi.name = name.substr(1, name.length() - 1);
      bi.size = value;
      last->bits_info.push_back(bi);
      continue; 
    }

    auto temp = CreateConstantsData(name, value);
    auto p_temp = temp.get();

    if (indent == last_indent) {
      parent->children[name] = std::move(temp);
      last = p_temp;
    } else if (indent > last_indent) {
      last->children[name] = std::move(temp);
      stack.push(parent);
      parent = last;
      last = p_temp;
      last_indent = indent;
    } else if (indent < last_indent) {
      for (int i = 0; i < last_indent - indent; i++) {
        last = parent;
        parent = stack.top();
        stack.pop();
      }
      parent->children[name] = std::move(temp);
      last = p_temp;
      last_indent = indent;
    }
  }
}
//------------------------------------------------------------------------------
enum class ETokenType {
  None,
  Decimal,
  Hex,
  Reference
};

//------------------------------------------------------------------------------
inline bool IsArithmeticOp(char c) {
  return c == '+' || c == '-' || c == '*' || c == '/';
}
//------------------------------------------------------------------------------
unsigned int FindConstantValue(const std::string& s, const A2& a2) {
  std::string token;
  const ConstantsData* constants = nullptr;
  for (int i = 0; i <= s.length(); i++) {
    char c = i == s.length() ? '.' :s[i];
    if (c == '.') {
      if (constants == nullptr) {
        for (auto& pair : a2.constants) {
          auto itr = pair.second->children.find(token);
          if (itr != pair.second->children.end()) {
            constants = itr->second.get();
            token.clear();
            break;
          }
        }
      } else {
        auto itr = constants->children.find(token);
        if (itr != constants->children.end()) {
          constants = itr->second.get();
        }
      }
    } else {
      token += c;
    }
  }

  //std::cout << "FindConstant: " << s << ", " << constants->name << ": " << constants->value << std::endl;

  return constants != nullptr ? constants->value : 0;
}
//------------------------------------------------------------------------------
unsigned int ParseValue(const std::string& s, ETokenType type, const A2& a2) {
  if (type == ETokenType::Decimal) {
    return strtoul(s.c_str(), nullptr, 10);
  } else if (type == ETokenType::Hex) {
    return strtoul(s.c_str(), nullptr, 16);
  } else if (type == ETokenType::Reference) {
    return FindConstantValue(s, a2);
  }

  return 0;
}
//------------------------------------------------------------------------------
unsigned int Add(unsigned int x, unsigned int y) { return x + y; }
unsigned int Subtract(unsigned int x, unsigned int y) { return x - y; }
unsigned int Multiply(unsigned int x, unsigned int y) { return x * y; }
unsigned int Divide(unsigned int x, unsigned int y) { return x / y; }
//------------------------------------------------------------------------------
unsigned int ComputeFormula(const std::string& formula, const A2& a2) {

  std::string token;
  unsigned int value = 0;
  ETokenType type = ETokenType::None;
  unsigned int (*op)(unsigned int, unsigned int) = nullptr;

  for (auto i = 0; i <= formula.length(); i++) {
    char c = i == formula.length() ? ' ' : formula[i];

    if (IsArithmeticOp(c)) {
      if (!token.empty()) {
        int temp = ParseValue(token, type, a2);
        value = op == nullptr ? temp : op(value, temp);
        token.clear();
        op = nullptr;
        type = ETokenType::None;
      }

      if (c == '+') { op = Add; }
      if (c == '-') { op = Subtract; }
      if (c == '*') { op = Multiply; }
      if (c == '/') { op = Divide; }
    } else {
      if (type == ETokenType::None) {
        if (c != ' ') {
          if (c == '0') {
            type = ETokenType::Hex;
          } else if ('1' <= c && c <= '9') {
            type = ETokenType::Decimal;
          } else {
            type = ETokenType::Reference;
          }
          token += c;
        }
      } else {
        if (c == ' ') {
          int temp = ParseValue(token, type, a2);
          value = op == nullptr ? temp : op(value, temp);
          token.clear();
          op = nullptr;
          type = ETokenType::None;
        } else {
          if (type == ETokenType::Decimal) {
            if ('0' <= c && c <= '9') {
              token += c;
            } else {
              // error!!!
            }
          } else if (type == ETokenType::Hex) {
            if (token.length() == 1) {
              if (c == 'x') {
                token += c;
              } else {
                // error !!
              }
            } else {
              if (('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F')) {
                token += c;
              } else {
                // error !
              }
            }
          } else if (type == ETokenType::Reference) {
            token += c;
          }
        }
      }
    }
  }

  return value;
}
//------------------------------------------------------------------------------
void ProcTableBlock(BlockLinesFetcher& blf, A2& a2) {
  std::string line;
  while (blf.Next(line)) {
    std::size_t indent = 0;
    std::string name;
    std::string value;
    ParseNameValueLine(line, &indent, &name, &value);

    if (!value.empty()) {
      TableEntry entry;
      entry.name = name;
      if (value[0] == '@') {
        entry.dynamic = value.substr(1, value.length() - 1);
        entry.value = 0;
      } else {
        entry.value = ComputeFormula(value, a2);
      }

      a2.table.push_back(entry);
    }
  }
}
//------------------------------------------------------------------------------
void ProcCodeBlock(BlockLinesFetcher& blf, A2& a2) {
  std::string line;
  // Supported syntax
  // 1) ABC
  // 2) ABC(ARG1)
  // 3) ABC(ARG1, ARG2, ....)
  std::regex rgx_code(R"(\s*(\w+)(?:\((\w+)((?:\s?,\s?\w+)*)\))?)");
  std::regex rgx_code_args(R"((\w+))");
                                                           
  while (blf.Next(line)) {
    std::smatch match;
    if (std::regex_match(line, match, rgx_code)) {
      Instruction inst;
      inst.func = match[1];
      if (match[2].length() > 0) {
        inst.args.push_back(match[2]);
      }
      if (match[3].length() > 0) {
        std::string temp = match[3];
        while (std::regex_search(temp, match, rgx_code_args)) {
          inst.args.push_back(match[1]);
          temp = match.suffix();
        }
      }
      a2.instructions.push_back(std::move(inst));
    } else {
      // error !!!
    }
  }
}
//------------------------------------------------------------------------------
std::unique_ptr<A2> ParseA2(std::istream& from) {
  //auto a2 = std::unique_ptr<A2>(new A2());
  auto a2 = std::make_unique<A2>();

  auto lf = LineFetcher(from);
  auto bf = BlockFetcher(lf);

  std::unique_ptr<BlockLinesFetcher> blf;
  while (bf.Next(blf)) {
    switch (bf.GetType()) {
      case EBlockType::Constants:
        ProcConstantsBlock(bf.GetName(), *blf.get(), *a2.get());
        break;
      case EBlockType::Table:
        ProcTableBlock(*blf.get(), *a2.get());
        break;
      case EBlockType::Code:
        ProcCodeBlock(*blf.get(), *a2.get());
        break;
    }
  }

  return a2;
}
//------------------------------------------------------------------------------
void DumpConstants(const std::unordered_map<std::string, std::unique_ptr<ConstantsData>>& map, int indent) {
  std::string indent_s(indent * 2, ' ');

  for (auto& pair : map) {
    std::cout << indent_s << pair.first << ": 0x" << std::hex << pair.second->value << std::endl;

    for (auto& bit_info : pair.second->bits_info) {
      std::cout << indent_s << "  ." << bit_info.name << ": 0x" << bit_info.size << std::endl;
    }

    DumpConstants(pair.second->children, indent + 1);

    if (indent == 0) {
      std::cout << std::endl;
    }
  }
}
//------------------------------------------------------------------------------
void DumpTable(const std::vector<TableEntry>& table) {
  std::cout << std::endl << "table:" << std::endl;

  for (auto& entry : table) {
    std::cout << "  " << entry.name << ": ";
    if (entry.dynamic.empty()) { std::cout << std::hex << "0x" <<entry.value; }
    else { std::cout << entry.dynamic; }
    std::cout << std::endl;
  }
}
//------------------------------------------------------------------------------
void DumpInstructions(const std::vector<Instruction>& insts) {
  std::cout << std::endl << "instructions:" << std::endl;

  for (auto& inst : insts) {
    std::cout << "  " << inst.func << ": " <<
      std::accumulate(inst.args.begin(), inst.args.end(), std::string(), 
          [](auto& ac, auto& s) { return ac.empty() ? s : ac + ", " + s; })  << std::endl;
  }
}
//------------------------------------------------------------------------------
void DumpA2(const A2& a2) {
  DumpConstants(a2.constants, 0);
  DumpTable(a2.table);
  DumpInstructions(a2.instructions);
}

}
