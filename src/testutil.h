#pragma once

#include <iostream>
#include <iomanip>
#include <string>

namespace a2test {

void PutTestHeader(const char* header) { std::cout << std::endl << header << std::endl; }
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

}
