#include "exception.h"

namespace a2 {

std::unordered_map<EParseErrorCode, std::string> gEParseErrorCodeToStr = {
  { EParseErrorCode::kSuccess, "kSuccess" },
  { EParseErrorCode::kRegexError, "kRegexError" },
  { EParseErrorCode::kIndentCount, "kIndentCount" },
  { EParseErrorCode::kUnexpectedArithOp, "kUnexpectedArithOp" },
  { EParseErrorCode::kUnexpected, "kUnexpected" }
};

}


