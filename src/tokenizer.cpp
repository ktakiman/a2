#include "tokenizer.h"

#include <regex>

#include "exception.h"

constexpr std::size_t INDENT_UNIT = 2;

#define RGX_INDENT R"(( +))"
#define RGX_TAG  R"((\w+))"
#define RGX_HEX  R"(0[x|X]([0-9a-fA-F]+))"

#define RGX_BLANK R"( *)"

std::regex gRgxNamedValue((RGX_INDENT RGX_TAG RGX_BLANK ":" RGX_BLANK RGX_HEX RGX_BLANK));

namespace {

int CountIndent(const std::string& s) {
  if (s.length() % INDENT_UNIT > 0) {
    throw ParseException(EParseErrorCode::gIndentCount);
  }
  return s.length() / INDENT_UNIT; 
}

}

namespace a2 {

NamedValue TokenizeNamedValue(const std::string& s) {
  std::smatch match;
  if (std::regex_match(s, match, gRgxNamedValue)) {
    NamedValue named_value;
    named_value.indent = CountIndent(match[1].str());
    named_value.name = match[2].str();
    named_value.value = strtoul(match[3].str().c_str(), nullptr, 16);
    return named_value;
  } else {
    throw ParseException(EParseErrorCode::gRegexError);
  }
}

}
