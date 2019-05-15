#pragma once

#include <exception>

namespace a2 {

enum class EParseErrorCode {
  kSuccess,
  kRegexError,
  kIndentCount
};

class ParseException : std::exception {
  public:
    ParseException(EParseErrorCode code) : Code(code) {}
    EParseErrorCode Code;
};

}
