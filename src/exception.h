#pragma once

#include <exception>
#include <string>
#include <unordered_map>

namespace a2 {

enum class EParseErrorCode {
  kSuccess,
  kRegexError,
  kIndentCount,
  kUnexpectedArithOp,
  kUnexpected
};

extern std::unordered_map<EParseErrorCode, std::string> gEParseErrorCodeToStr;


class ParseException : std::exception {
  public:
    ParseException(EParseErrorCode code) : Code(code) {}
    EParseErrorCode Code;
};

}
