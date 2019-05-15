#include "tokenizer.h"

#include <regex>
#include <iostream>
#include <iomanip>

#include "exception.h"

constexpr std::size_t INDENT_UNIT = 2;

#define RGX_INDENT R"(( *))"
#define RGX_TAG  R"((\w+))"
#define RGX_INT  R"((0[xX])?([0-9a-fA-F]+))"

#define RGX_BLANK R"( *)"

std::regex gRgxNamedValue((RGX_INDENT RGX_TAG RGX_BLANK ":" RGX_BLANK RGX_INT RGX_BLANK));

namespace {

using namespace a2;

std::size_t CountIndent(const std::string& s) {
  if ((s.length() % INDENT_UNIT) > 0) {
    throw ParseException(EParseErrorCode::kIndentCount);
  }
  return s.length() / INDENT_UNIT; 
}

}

namespace a2 {

bool NamedValue::operator==(const NamedValue& other) {
  return name == other.name && value == other.value && indent == other.indent;
}

NamedValue TokenizeNamedValue(const std::string& s) {
  std::smatch match;
  if (std::regex_match(s, match, gRgxNamedValue)) {
    NamedValue named_value;
    named_value.indent = CountIndent(match[1].str());
    named_value.name = match[2].str();
    named_value.value = strtoul(match[4].str().c_str(), nullptr, (match[3].length() > 0 ? 16 : 10));
    return named_value;
  } else {
    throw ParseException(EParseErrorCode::kRegexError);
  }
}

}


namespace a2test {

using namespace a2;

void PutTestHeader(const char* header) { std::cout << header << std::endl; }
void PutTestId(std::size_t id) { std::cout << "  " << std::setw(3) << std::setfill('0') << id << ": "; }

template<typename T>
bool AssertEqual(const char* name, const T& expected, const T& actual) {
  if (actual != expected) {
    std::cout << "* unexpected " << name << ", expecting (" << expected << "), actual (" << actual << ")" << std::endl;
    return false;
  }

  return true;
}

void Passed() { std::cout << "pass" << std::endl; }

template<typename T>
void ExceptionNotThrown(const T& expected) { std::cout << "* expected exception not thrown: " << (int)expected << std::endl; }

void UnexpectedException() { std::cout << "* unexpected exception" << std::endl; }

void TestTokenizeNamedValue(int id, const std::string& s, EParseErrorCode expected_error, const std::string& expected_name, 
  std::size_t expected_value, std::size_t expected_indent) {

  PutTestId(id);

  try {
    auto r = TokenizeNamedValue(s);
    if (expected_error == EParseErrorCode::kSuccess) {
      if (AssertEqual("name", r.name, expected_name) && 
          AssertEqual("value", r.value, expected_value) && 
          AssertEqual("indent", r.indent, expected_indent)) {
        Passed();
      }
    } else {
      ExceptionNotThrown((int)expected_error);
    }
  } catch (const ParseException& pe) {
    if (AssertEqual("exception", (int)expected_error, (int)pe.Code)) {
      Passed();
    }
  } catch (...) { UnexpectedException(); }
}

void TestTokenizer() {
  PutTestHeader("TokenizeNamedValue");
  TestTokenizeNamedValue(1, "N:0", EParseErrorCode::kSuccess, "N", 0, 0);
  TestTokenizeNamedValue(2, "  trick:10", EParseErrorCode::kSuccess, "trick", 10, 1);
  TestTokenizeNamedValue(3, "v:0x10", EParseErrorCode::kSuccess, "v", 0x10, 0);
  TestTokenizeNamedValue(4, "    hex:0x80010004", EParseErrorCode::kSuccess, "hex", 0x80010004, 2);
  TestTokenizeNamedValue(5, "N: 1", EParseErrorCode::kSuccess, "N", 1, 0);
  TestTokenizeNamedValue(6, "  booo: 0xaa ", EParseErrorCode::kSuccess, "booo", 0xaa, 1);
}

}
