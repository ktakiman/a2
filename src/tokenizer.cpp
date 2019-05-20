#include "tokenizer.h"

#include <regex>
#include <iostream>
#include <iomanip>

#include "exception.h"
#include "testutil.h"

constexpr std::size_t INDENT_UNIT = 2;

#define RGX_INDENT "( *)"
#define RGX_BLANK " *"

#define RGX_NAME "(\\w+)"                       // plain name (letter and '_')  
#define RGX_INT "([0-9]+)"
#define RGX_HEX "(?:0[xX]([0-9a-fA-F]+))"
#define RGX_NUM "(?:" RGX_INT "|" RGX_HEX ")"

// named constant
#define RGX_NC_NAME "((?:\\.?\\w+|\\.\\*))"        // can start with '.', be just ".*", or plain name

// named ref
#define RGX_ADDR_REF "(@\\w+)"
#define RGX_CONST_REF "(\\w+(?:\\.\\w+)*)"
#define RGX_ADDR_OR_CONST_REF "(?:" RGX_ADDR_REF "|" RGX_CONST_REF ")"

#define RGX_ARITH_OP "(\\+|-)"

std::regex gRgxNamedConstant(RGX_INDENT RGX_NC_NAME RGX_BLANK ":" RGX_BLANK RGX_NUM RGX_BLANK);

std::regex gRgxNamedRef(RGX_INDENT RGX_NAME ":" RGX_BLANK RGX_ADDR_OR_CONST_REF RGX_BLANK 
    "(?:" RGX_ARITH_OP RGX_BLANK RGX_ADDR_OR_CONST_REF RGX_BLANK ")?");

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

NamedConstant TokenizeNamedConstant(const std::string& s) {
  std::smatch match;
  if (std::regex_match(s, match, gRgxNamedConstant)) {
    NamedConstant named_constant;
    named_constant.indent = CountIndent(match[1].str());
    named_constant.name = match[2].str();
    if (match[3].length() > 0) {
      named_constant.value = strtoul(match[3].str().c_str(), nullptr, 10);
    } else {
      named_constant.value = strtoul(match[4].str().c_str(), nullptr, 16);
    }
    return named_constant;
  } else {
    throw ParseException(EParseErrorCode::kRegexError);
  }
}

void AppendRefed(const std::smatch& match, int index, ERefedOp op, NamedRef& named_ref) {
  if (match[index].length() > 0) {
    named_ref.refs.emplace_back(Refed {ERefedType::kAddr, op, match[index].str().substr(1)} );
  } else if (match[index + 1].length() > 0) {
    named_ref.refs.emplace_back(Refed {ERefedType::kConst, op, match[index + 1]} );
  }
}

NamedRef TokenizeNamedRef(const std::string& s) {
  std::smatch match;
  if (std::regex_match(s, match, gRgxNamedRef)) {
    NamedRef named_ref;
    named_ref.indent = CountIndent(match[1].str());
    named_ref.name = match[2];

    AppendRefed(match, 3, ERefedOp::kNone, named_ref);

    if (match[5] == "+") {
      AppendRefed(match, 6, ERefedOp::kAdd, named_ref);
    } else if (match[5] == "-") {
      AppendRefed(match, 6, ERefedOp::kSubtract, named_ref);
    }

    return named_ref;
  }
  else {
    throw ParseException(EParseErrorCode::kRegexError);
  }
}

}


namespace a2test {

using namespace a2;


void TestTokenizeNamedConstant(int id, const std::string& s, EParseErrorCode exp_error, 
    std::size_t exp_indent, const std::string& exp_name, unsigned int exp_value) {

  PutTestId(id);

  try {
    auto r = TokenizeNamedConstant(s);
    if (exp_error == EParseErrorCode::kSuccess) {
      if (AssertEqual("name", exp_name, r.name) && 
          AssertEqual("value", exp_value, r.value) && 
          AssertEqual("indent", exp_indent, r.indent)) {
        Passed();
      }
    } else {
      ExceptionNotThrown((int)exp_error);
    }
  } catch (const ParseException& pe) {
    if (AssertEqual("exception", (int)exp_error, (int)pe.Code)) {
      Passed();
    }
  } catch (...) { UnexpectedException(); }
}

void TestTokenizeNamedConstant(int id, const std::string& s, EParseErrorCode exp_error) {
  TestTokenizeNamedConstant(id, s, exp_error, 0, "", 0);
}

void TestTokenizeNamedRef(int id, const std::string& s, EParseErrorCode exp_error, std::size_t exp_indent, 
    const std::string& exp_name, std::size_t exp_refct, const std::string& exp_val1, ERefedType exp_type1, 
    ERefedOp exp_op2, const std::string& exp_val2, ERefedType exp_type2) {
  PutTestId(id);

  try {
    auto r = TokenizeNamedRef(s);
    if (exp_error == EParseErrorCode::kSuccess) {
      if (AssertEqual("indent", exp_indent, r.indent) &&
          AssertEqual("name", exp_name, r.name) &&
          AssertEqual("ref ct", exp_refct, r.refs.size()) &&
          AssertEqual("ref1 val", exp_val1, r.refs[0].ref) &&
          AssertEqual("ref1 type", (int)exp_type1, (int)r.refs[0].type) &&
          AssertEqual("ref1 op", (int)ERefedOp::kNone, (int)r.refs[0].op)) {
        Passed();
      }
    } else {
      ExceptionNotThrown((int)exp_error);
    }
  } catch (const ParseException& pe) {
    if (AssertEqual("exception", (int)exp_error, (int)pe.Code)) {
      Passed();
    }
  } catch (...) { UnexpectedException(); }
}

void TestTokenizeNamedRef(int id, const std::string& s, EParseErrorCode exp_error, std::size_t exp_indent, 
    const std::string& exp_name, const std::string& exp_val, ERefedType exp_type) {
  TestTokenizeNamedRef(id, s, exp_error, exp_indent, exp_name, 1, exp_val, exp_type, 
      ERefedOp::kNone, "", ERefedType::kNone);
}

void TestTokenizeNamedRef(int id, const std::string& s, EParseErrorCode exp_error) {
  TestTokenizeNamedRef(id, s, exp_error, 0, "", "", ERefedType::kNone);
}

void TestTokenizer() {
  PutTestHeader("TokenizeNamedConstant");
  TestTokenizeNamedConstant(1, "N:0", EParseErrorCode::kSuccess, 0, "N", 0);
  TestTokenizeNamedConstant(2, "n:1", EParseErrorCode::kSuccess, 0, "n", 1);
  TestTokenizeNamedConstant(3, "  trick:10", EParseErrorCode::kSuccess, 1, "trick", 10);
  TestTokenizeNamedConstant(4, "v:0x10", EParseErrorCode::kSuccess, 0, "v", 0x10);
  TestTokenizeNamedConstant(5, "    hex:0x80010004", EParseErrorCode::kSuccess, 2, "hex", 0x80010004);
  TestTokenizeNamedConstant(6, "N: 1", EParseErrorCode::kSuccess, 0, "N", 1);
  TestTokenizeNamedConstant(7, "  booo: 0xaa ", EParseErrorCode::kSuccess, 1, "booo", 0xaa);
  TestTokenizeNamedConstant(8, ".iopaen: 0x1", EParseErrorCode::kSuccess, 0, ".iopaen", 0x1);
  TestTokenizeNamedConstant(9, ".*: 0x3", EParseErrorCode::kSuccess, 0, ".*", 0x3);
  TestTokenizeNamedConstant(10, "a_b:0x200", EParseErrorCode::kSuccess, 0, "a_b", 0x200);

  TestTokenizeNamedConstant(20, "c0", EParseErrorCode::kRegexError);
  TestTokenizeNamedConstant(21, "c.", EParseErrorCode::kRegexError);   // '.' only allowed at the beginning of name
  TestTokenizeNamedConstant(22, ".**", EParseErrorCode::kRegexError);   // '.*' is the only valid usage of '*'
  TestTokenizeNamedConstant(25, " c:0", EParseErrorCode::kIndentCount);

  PutTestHeader("TokenizeNamedRef");
  TestTokenizeNamedRef(1, "r:@reset", EParseErrorCode::kSuccess, 0, "r", "reset", ERefedType::kAddr);
  TestTokenizeNamedRef(2, "r: @reset", EParseErrorCode::kSuccess, 0, "r", "reset", ERefedType::kAddr);
  TestTokenizeNamedRef(3, "abc:io.pina", EParseErrorCode::kSuccess, 0, "abc", "io.pina", ERefedType::kConst);
  TestTokenizeNamedRef(4, "abc:io.pina", EParseErrorCode::kSuccess, 0, "abc", "io.pina", ERefedType::kConst);
  TestTokenizeNamedRef(5, "abc:a.b.c.d", EParseErrorCode::kSuccess, 0, "abc", "a.b.c.d", ERefedType::kConst);
  TestTokenizeNamedRef(6, "  abc:@reset", EParseErrorCode::kSuccess, 1, "abc", "reset", ERefedType::kAddr);

  TestTokenizeNamedRef(10, "R:a+b", EParseErrorCode::kSuccess, 0, "R", 2, "a", ERefedType::kConst, ERefedOp::kAdd, "b", ERefedType::kConst);
  TestTokenizeNamedRef(11, "R:a-b", EParseErrorCode::kSuccess, 0, "R", 2, "a", ERefedType::kConst, ERefedOp::kSubtract, "b", ERefedType::kConst);
  TestTokenizeNamedRef(12, "R: @a + b ", EParseErrorCode::kSuccess, 0, "R", 2, "a", ERefedType::kAddr, ERefedOp::kAdd, "b", ERefedType::kConst);
  TestTokenizeNamedRef(13, "  R:  a - @b", EParseErrorCode::kSuccess, 1, "R", 2, "a", ERefedType::kConst, ERefedOp::kSubtract, "b", ERefedType::kAddr);
  TestTokenizeNamedRef(15, "  stack_addr: flash_addr + flash_sz", EParseErrorCode::kSuccess, 1, 
      "stack_addr", 2, "flash_addr", ERefedType::kConst, ERefedOp::kAdd, "flash_sz", ERefedType::kConst);

  TestTokenizeNamedRef(20, " r:@ext1", EParseErrorCode::kIndentCount);
  TestTokenizeNamedRef(21, "r:@@ext1", EParseErrorCode::kRegexError);
  TestTokenizeNamedRef(22, "r:ext@", EParseErrorCode::kRegexError);
  TestTokenizeNamedRef(23, "r:a..b", EParseErrorCode::kRegexError);
  TestTokenizeNamedRef(24, "r:.a", EParseErrorCode::kRegexError);
  TestTokenizeNamedRef(30, "r:@a+", EParseErrorCode::kRegexError);
  TestTokenizeNamedRef(31, "r:+@a", EParseErrorCode::kRegexError);
  TestTokenizeNamedRef(32, "r:a++b", EParseErrorCode::kRegexError);
  TestTokenizeNamedRef(33, "r:a + b + c", EParseErrorCode::kRegexError);
}

}
