#include "tokenizer.h"

#include <regex>
#include <iostream>
#include <iomanip>

#include "exception.h"
#include "testutil.h"

#define RGX_INDENT "( *)"
#define RGX_BLANK " *"

#define RGX_NAME "[a-zA-Z_]\\w*"

#define RGX_NAME_CP "(" RGX_NAME ")"          // letter or '_' at pos0, letter or '_' or num at pos1+
#define RGX_INT "([0-9]+)"
#define RGX_HEX "(?:0[xX]([0-9a-fA-F]+))"
#define RGX_NUM "(?:" RGX_INT "|" RGX_HEX ")"


#define RGX_ADDR_REF "(@\\w+)"
#define RGX_CONST_REF "(" RGX_NAME "(?:\\." RGX_NAME ")*)"
#define RGX_ALL_REF "(?:" RGX_ADDR_REF "|" RGX_CONST_REF "|" RGX_NUM ")"

#define RGX_ALL_REF_WITH_BLANK RGX_BLANK RGX_ALL_REF RGX_BLANK

#define RGX_ARITH_OP "(\\+|-)"

// named constant
#define RGX_NC_NAME "((?:\\.?" RGX_NAME "|\\.\\*))"        // can start with '.', be just ".*", or plain name

// instruction
#define RGX_INST_NAME "([a-zA-Z]+)"
 
namespace {

std::regex gRgxNamedConstant(RGX_INDENT RGX_NC_NAME RGX_BLANK ":" RGX_BLANK RGX_NUM RGX_BLANK);

std::regex gRgxNamedRef(RGX_INDENT RGX_NAME_CP ":" RGX_ALL_REF_WITH_BLANK "(?:" RGX_ARITH_OP RGX_ALL_REF_WITH_BLANK ")?");

std::regex gRgxInst(RGX_INDENT RGX_INST_NAME "(?:\\((" RGX_ALL_REF_WITH_BLANK "(?:" RGX_BLANK "," RGX_ALL_REF_WITH_BLANK ")*" ")\\))?" RGX_BLANK);

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
    named_ref.refs.emplace_back(Refed {match[index].str(), op});
  } else if (match[index + 1].length() > 0) {
    named_ref.refs.emplace_back(Refed {match[index + 1], op});
  } else if (match[index + 2].length() > 0) {
    named_ref.refs.emplace_back(Refed {std::stoul(match[index + 2]), op});
  } else if (match[index + 3].length() > 0) {
    named_ref.refs.emplace_back(Refed {std::stoul(match[index + 3], 0, 16), op});
  }
}

NamedRef TokenizeNamedRef(const std::string& s) {
  std::smatch match;
  if (std::regex_match(s, match, gRgxNamedRef)) {
    NamedRef named_ref;
    named_ref.indent = CountIndent(match[1].str());
    named_ref.name = match[2];

    AppendRefed(match, 3, ERefedOp::kNone, named_ref);

    if (match[7] == "+") {
      AppendRefed(match, 8, ERefedOp::kAdd, named_ref);
    } else if (match[7] == "-") {
      AppendRefed(match, 8, ERefedOp::kSubtract, named_ref);
    }

    return named_ref;
  } else {
    throw ParseException(EParseErrorCode::kRegexError);
  }
}

Inst TokenizeInstruction(const std::string& s) {
  std::smatch match;
  if (std::regex_match(s, match, gRgxInst)) {
    Inst inst;
    inst.indent = CountIndent(match[1].str());
    inst.func = match[2];
    return inst;
  } else {
    throw ParseException(EParseErrorCode::kRegexError);
  }
}

}


namespace a2test {

using namespace a2;
using CSR = const std::string&;

template<typename F>
void Test(int id, EParseErrorCode exp_error, std::ostream& out, F f) {
  std::stringstream ss;

  bool pass = false;
  PutTestId(id, ss);
  try {
    if (f(ss)) {
      pass = true;
    }
  } catch (const ParseException& pe) {
    if (AssertEqual("exception", (int)exp_error, (int)pe.Code, ss)) {
      pass = true;
    }
  } catch (...) { UnexpectedException(ss); }

  if (pass) { out << "."; }
  else { out << std::endl << ss.str(); }
}

bool VerifyRefed(int ref_index, const Refed& expected, const Refed& actual, std::ostream& out) {
  auto s_ref = "ref" + std::to_string(ref_index);
  return 
    AssertEqual((s_ref + " op").c_str(), (int)expected.op, (int)actual.op, out) && 
    AssertEqual((s_ref + " type").c_str(), (int)expected.type, (int)actual.type, out) &&
    ((expected.type != ERefedType::kConst && expected.type != ERefedType::kAddr) || 
      AssertEqual((s_ref + " val").c_str(), expected.ref, actual.ref, out)) &&
    (expected.type != ERefedType::kNum || 
     AssertEqual((s_ref + " num").c_str(), expected.num, actual.num, out));
}

bool VerifyRefed(const std::vector<Refed>& expected, const std::vector<Refed>& actual, std::ostream& out) {
  if (!AssertEqual("arg_ct", expected.size(), actual.size(), out)) { return false; }
  for (std::size_t i = 0; i < expected.size(); i++) {
    if (!VerifyRefed(i, expected[i], actual[i], out)) { return false; }
  }
  return true;
}

// ----------------------------------------------------------------------------
// Test TokenizeNamedConstant
// ----------------------------------------------------------------------------
void TestTnc(int id, CSR s, EParseErrorCode exp_error, std::size_t exp_indent, CSR exp_name, unsigned int exp_value) {

  Test(id, exp_error, std::cout, [&](std::ostream& out) {
    auto r = TokenizeNamedConstant(s);
    if (exp_error == EParseErrorCode::kSuccess) {
      return AssertEqual("name", exp_name, r.name, out) && 
          AssertEqual("value", exp_value, r.value, out) && 
          AssertEqual("indent", exp_indent, r.indent, out);
    }
    ExceptionNotThrown((int)exp_error, out);
    return false;
  });
}

void TestTnc(int id, CSR s, EParseErrorCode exp_error) {
  TestTnc(id, s, exp_error, 0, "", 0);
}

// ----------------------------------------------------------------------------
// Test TokenizeNamedRef
// ----------------------------------------------------------------------------
void TestTnr(int id, CSR s, EParseErrorCode exp_error, std::size_t exp_indent, CSR exp_name, const std::vector<Refed>& expected_args) {
  Test(id, exp_error, std::cout, [&](std::ostream& out) {
    auto r = TokenizeNamedRef(s);
    if (exp_error == EParseErrorCode::kSuccess) {
      return AssertEqual("indent", exp_indent, r.indent, out) &&
          AssertEqual("name", exp_name, r.name, out) &&
          VerifyRefed(expected_args, r.refs, out);
    }

    ExceptionNotThrown((int)exp_error, out);
    return false;
  });
}

void TestTnr(int id, CSR s, EParseErrorCode exp_error) {
  TestTnr(id, s, exp_error, 0, "", {});
}

// ----------------------------------------------------------------------------
// Test TokenizeInstruction
// ----------------------------------------------------------------------------
void TestTni(int id, CSR s, EParseErrorCode exp_error, std::size_t exp_indent, CSR exp_func, const std::vector<Refed>& exp_args) {
  Test(id, exp_error, std::cout, [&](std::ostream& out) {
    auto inst = TokenizeInstruction(s);
    if (exp_error == EParseErrorCode::kSuccess) {
      return 
        AssertEqual("indent", exp_indent, inst.indent, out) && 
        AssertEqual("func", exp_func, inst.func, out) &&
        VerifyRefed(exp_args, inst.args, out);
    }

    ExceptionNotThrown((int)exp_error, out);
    return false;
  });
}

void TestTni(int id, CSR s, EParseErrorCode exp_error, std::size_t exp_indent, CSR exp_func) {
  TestTni(id, s, exp_error, exp_indent, exp_func, {});
}

void TestTni(int id, CSR s, EParseErrorCode exp_error) {
  TestTni(id, s, exp_error, 0, "");
}


void TestTokenizer() {
  PutTestHeader("TokenizeNamedConstant", std::cout);
  TestTnc(1, "N:0", EParseErrorCode::kSuccess, 0, "N", 0);
  TestTnc(2, "n:1", EParseErrorCode::kSuccess, 0, "n", 1);
  TestTnc(3, "  trick:10", EParseErrorCode::kSuccess, 1, "trick", 10);
  TestTnc(4, "v:0x10", EParseErrorCode::kSuccess, 0, "v", 0x10);
  TestTnc(5, "    hex:0x80010004", EParseErrorCode::kSuccess, 2, "hex", 0x80010004);
  TestTnc(6, "N: 1", EParseErrorCode::kSuccess, 0, "N", 1);
  TestTnc(7, "  booo: 0xaa ", EParseErrorCode::kSuccess, 1, "booo", 0xaa);
  TestTnc(8, ".iopaen: 0x1", EParseErrorCode::kSuccess, 0, ".iopaen", 0x1);
  TestTnc(9, ".*: 0x3", EParseErrorCode::kSuccess, 0, ".*", 0x3);
  TestTnc(10, "a_b:0x200", EParseErrorCode::kSuccess, 0, "a_b", 0x200);

  TestTnc(20, "c0", EParseErrorCode::kRegexError);
  TestTnc(21, "c.", EParseErrorCode::kRegexError);   // '.' only allowed at the beginning of name
  TestTnc(22, ".**", EParseErrorCode::kRegexError);   // '.*' is the only valid usage of '*'
  TestTnc(25, " c:0", EParseErrorCode::kIndentCount);
  std::cout << std::endl;

  PutTestHeader("TokenizeNamedRef", std::cout);
  TestTnr(1, "r:@reset", EParseErrorCode::kSuccess, 0, "r", {{"@reset"}});
  TestTnr(2, "r: @reset", EParseErrorCode::kSuccess, 0, "r", {{"@reset"}});
  TestTnr(3, "abc:io.pina", EParseErrorCode::kSuccess, 0, "abc", {{"io.pina"}});
  TestTnr(5, "abc:a.b.c.d", EParseErrorCode::kSuccess, 0, "abc", {{"a.b.c.d"}});
  TestTnr(6, "  abc:@reset", EParseErrorCode::kSuccess, 1, "abc", {{"@reset"}});
  TestTnr(7, "r:3", EParseErrorCode::kSuccess, 0, "r", {{3}});
  TestTnr(8, "r:0xa3", EParseErrorCode::kSuccess, 0, "r", {{0xa3}});
  TestTnr(9, "  int_handler:@int + 1", EParseErrorCode::kSuccess, 1, "int_handler", {{"@int"}, {1, ERefedOp::kAdd}});


  TestTnr(10, "R:a+b", EParseErrorCode::kSuccess, 0, "R", {{"a"}, {"b", ERefedOp::kAdd}});
  TestTnr(11, "R:a-b", EParseErrorCode::kSuccess, 0, "R", {{"a"}, {"b", ERefedOp::kSubtract}});
  TestTnr(12, "R: @a + b ", EParseErrorCode::kSuccess, 0, "R", {{"@a"}, {"b", ERefedOp::kAdd}});
  TestTnr(13, "  R:  a - @b", EParseErrorCode::kSuccess, 1, "R", {{"a"}, {"@b", ERefedOp::kSubtract}});
  TestTnr(15, "  stack_addr: flash_addr + flash_sz", EParseErrorCode::kSuccess, 1, "stack_addr", {{"flash_addr"}, {"flash_sz", ERefedOp::kAdd}});

  TestTnr(20, " r:@ext1", EParseErrorCode::kIndentCount);
  TestTnr(21, "r:@@ext1", EParseErrorCode::kRegexError);
  TestTnr(22, "r:ext@", EParseErrorCode::kRegexError);
  TestTnr(23, "r:a..b", EParseErrorCode::kRegexError);
  TestTnr(24, "r:.a", EParseErrorCode::kRegexError);
  TestTnr(30, "r:@a+", EParseErrorCode::kRegexError);
  TestTnr(31, "r:+@a", EParseErrorCode::kRegexError);
  TestTnr(32, "r:a++b", EParseErrorCode::kRegexError);
  TestTnr(33, "r:a + b + c", EParseErrorCode::kRegexError);
  std::cout << std::endl;

  PutTestHeader("TokenizeInstruction", std::cout);
  TestTni(1, "A", EParseErrorCode::kSuccess, 0, "A");
  TestTni(2, "    ABC", EParseErrorCode::kSuccess, 2, "ABC");
  /* TestTni(3, "ABC(1)", EParseErrorCode::kSuccess, 0, "ABC", {{1u}}); */
  /* TestTni(4, "SET(@boo)", EParseErrorCode::kSuccess, 0, "SET", {{"@boo"}}); */
  /* TestTni(5, "SET(x.y.z)", EParseErrorCode::kSuccess, 0, "SET", {{"x.y.z"}}); */
  /* TestTni(6, "ABC(@iopa, 0x16)", EParseErrorCode::kSuccess, 0, "ABC", {{"@iopa"}}); */
  /* TestTni(7, "ABC(1, 2)", EParseErrorCode::kSuccess, 0, "ABC", {{1u}, {2u}}); */

  TestTni(50, " A", EParseErrorCode::kIndentCount);
  TestTni(51, "A(", EParseErrorCode::kRegexError);
  TestTni(52, "A)", EParseErrorCode::kRegexError);
  std::cout << std::endl;
}

}
