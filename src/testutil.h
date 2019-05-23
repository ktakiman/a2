#pragma once

#include <iostream>
#include <iomanip>
#include <string>

namespace a2test {

void PutTestHeader(const char* header, std::ostream& out) { out << std::endl << "== " << header << " ==" << std::endl; }
void PutTestId(std::size_t id, std::ostream& out) { out  << "  " << std::setw(3) << std::setfill('0') << id << ": "; }

template<typename T>
bool AssertEqual(const char* name, const T& expected, const T& actual, std::ostream& out) {
  if (actual != expected) {
    out << "* unexpected " << name << ", expecting (" << expected << "), actual (" << actual << ")" << std::endl;
    return false;
  }

  return true;
}

void Passed(std::ostream& out) { std::cout << "pass" << std::endl; }

template<typename T>
void ExceptionNotThrown(const T& expected, std::ostream& out) { out << "* expected exception not thrown: " << (int)expected << std::endl; }

void UnexpectedException(std::ostream& out) { out << "* unexpected exception" << std::endl; }

}
