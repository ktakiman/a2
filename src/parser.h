#ifndef __PARSER_H
#define __PARSER_H

#include <memory>
#include <iostream>
#include <string>

#include "types.h"

//------------------------------------------------------------------------------
std::unique_ptr<ConstantsData> CreateConstantsData(const std::string& name, unsigned int value = 0);

//------------------------------------------------------------------------------
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
  
//------------------------------------------------------------------------------
class BlockLinesFetcher {
public:
  BlockLinesFetcher(LineFetcher& lf) : lf_(lf) {}

  bool Next(std::string& line);

private:
  LineFetcher& lf_;
};

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
std::unique_ptr<A2> ParseA2(std::istream& from);
void DumpA2(const A2& a2); 

#endif
