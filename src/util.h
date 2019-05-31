#pragma once

#include <string>
#include <sstream>

namespace a2 {

template <typename T>
std::string ToHexStr(T n, bool add_prefix) {
  std::stringstream ss;
  if (add_prefix) {
    ss << "0x";
  }
  ss << std::hex << n;
  return ss.str();
}

}
