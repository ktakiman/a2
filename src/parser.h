#pragma once

#include <memory>
#include <iostream>
#include <string>

#include "types.h"

namespace a2 {

std::unique_ptr<A2> ParseA2(std::istream& from);

void DumpA2(const A2& a2); 

}
