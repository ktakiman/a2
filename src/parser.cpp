#include "parser.h"

#include <stack>
#include <numeric>
#include <algorithm>
#include <regex>

#include "tokenizer.h"
#include "util.h"

namespace {

using namespace a2;

std::unique_ptr<ConstantsData> CreateConstantsData(const std::string& name, unsigned int value = 0) {
  auto cd = std::make_unique<ConstantsData>();
  cd->name = name;
  cd->value = value;
  return cd;
}

class LineFetcher {
public:
  LineFetcher(std::istream& stream) : stream_(stream), rewind_(false) {}
  bool Next(std::string& next);
  void Rewind() { rewind_ = true; }
  
private:
  std::istream& stream_;
  std::string last_;
  bool rewind_;
};
  
class BlockLinesFetcher {
public:
  BlockLinesFetcher(LineFetcher& lf) : lf_(lf) {}
  bool Next(std::string& line);

private:
  LineFetcher& lf_;
};

class BlockFetcher {
public:
  BlockFetcher(LineFetcher& lf) : lf_(lf) {}

  static bool IsBlockHeader(std::string& line) { return line[0] != ' '; }

  bool Next(std::unique_ptr<BlockLinesFetcher>& blf);

  const std::string& GetName() const { return cur_block_name_; }

  EBlockType GetType() const { return cur_block_type_; }

private:
  LineFetcher& lf_;
  std::string cur_block_name_;
  EBlockType cur_block_type_ = EBlockType::None;
};

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

bool BlockFetcher::Next(std::unique_ptr<BlockLinesFetcher>& blf) {
  std::string line;
  if (lf_.Next(line)) {
    if (IsBlockHeader(line)) {
      blf = std::make_unique<BlockLinesFetcher>(lf_);

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

} // end anonymous namespace

namespace a2 {

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

    auto nv = TokenizeNamedConstant(line);

    if (line[nv.indent * INDENT_UNIT] == '.') { 
      BitsInfo bi;
      bi.name = nv.name.substr(1, nv.name.length() - 1);
      bi.size = nv.value;
      last->bits_info.push_back(bi);
      continue; 
    }

    auto temp = CreateConstantsData(nv.name, nv.value);
    auto p_temp = temp.get();

    if (nv.indent == last_indent) {
      parent->children[nv.name] = std::move(temp);
      last = p_temp;
    } else if (nv.indent > last_indent) {
      last->children[nv.name] = std::move(temp);
      stack.push(parent);
      parent = last;
      last = p_temp;
      last_indent = nv.indent;
    } else if (nv.indent < last_indent) {
      for (int i = 0; i < last_indent - nv.indent; i++) {
        last = parent;
        parent = stack.top();
        stack.pop();
      }
      parent->children[nv.name] = std::move(temp);
      last = p_temp;
      last_indent = nv.indent;
    }
  }
}

void ProcTableBlock(BlockLinesFetcher& blf, A2& a2) {
  std::string line;
  while (blf.Next(line)) {
    a2.table.push_back(TokenizeNamedRef(line));
  }
}

void ProcCodeBlock(BlockLinesFetcher& blf, A2& a2) {
  std::string line;
  std::string last_tag;
  while (blf.Next(line)) {
    bool is_named_tag = false;
    std::string tag;
    std::tie(is_named_tag, tag) = TryTokenizeNamedTag(line);
    if (is_named_tag) {
      last_tag = tag;
    } else {
      auto inst = TokenizeInstruction(line);
      inst.tag = last_tag;
      a2.instructions.push_back(inst);
      last_tag.clear();
    }
  }
}

std::unique_ptr<A2> ParseA2(std::istream& from) {
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

std::string RefedToStr(const Refed& refed) {
  auto op = gRefedOpToChar[refed.op];
  std::string s = op + std::string(op.length(), ' ');

  switch (refed.type) {
    case ERefedType::kNum:
      s += ToHexStr(refed.num, true);
      break;
    case ERefedType::kAddr:
      s += "@" + refed.ref;
      break;
    case ERefedType::kConst:
      s += refed.ref;
      break;
    default:
      break;
  }

  return s;
}

std::string ArithSeriesToStr(const std::vector<Refed>& arith_series) {
  return std::accumulate(arith_series.begin(), arith_series.end(), std::string(), 
    [](auto& ac, auto& refed) { 
      return ac + (ac.empty() ? "" : " ") + RefedToStr(refed);
  });
}

std::string ArithSeriesArgsToStr(const std::vector<std::vector<Refed>>& args) {
  return std::accumulate(args.begin(), args.end(), std::string(), 
    [](auto& ac, auto& arith_series) { 
      return ac + (ac.empty() ? "" : ", ") + ArithSeriesToStr(arith_series);
  });
}

void DumpTable(const std::vector<NamedRef>& table) {
  std::cout << std::endl << "table:" << std::endl;

  for (auto& entry : table) {
    std::cout << "  " << entry.name << ": " << ArithSeriesToStr(entry.value) << std::endl;
  }
}

void DumpInstructions(const std::vector<Instruction>& insts) {
  std::cout << std::endl << "instructions:" << std::endl;

  for (auto& inst : insts) {
    if (!inst.tag.empty()) {
      std::cout << "  " << inst.tag << std::endl;
    }
    std::cout << "    " << inst.func << ": " << ArithSeriesArgsToStr(inst.args) << std::endl;
  }
}

void DumpA2(const A2& a2) {
  DumpConstants(a2.constants, 0);
  DumpTable(a2.table);
  DumpInstructions(a2.instructions);
}

}
