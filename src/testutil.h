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

template<typename T>
bool AssertEqual(const char* name, const std::vector<T>& expected, const std::vector<T>& actual, std::ostream& out) {
  if (AssertEqual((name + std::string(" size")).c_str(), expected.size(), actual.size(), out)) {
    for (int i = 0; i < expected.size(); i++) {
      if (!AssertEqual((name + std::string(" at ") + std::to_string(i)).c_str(), expected[i], actual[i], out)) {
        return false;
      }
    }
    return true;
  }

  return false;
}

void Passed(std::ostream& out) { std::cout << "pass" << std::endl; }

template<typename T>
void ExceptionNotThrown(const T& expected, std::ostream& out) { out << "* expected exception not thrown: " << expected << std::endl; }

void UnexpectedException(std::ostream& out) { out << "* unexpected exception" << std::endl; }

}
