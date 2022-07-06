// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

namespace SilKit {
namespace Util {
namespace Uuid {

// UUID implementation based on 2x64bit random numbers
struct UUID
{
    uint64_t ab;
    uint64_t cd;
};

bool        operator==(const UUID& lhs, const UUID& rhs);
bool        operator!=(const UUID& lhs, const UUID& rhs);
bool        operator<(const UUID& lhs, const UUID& rhs);

UUID        generate();
std::string to_string(const UUID& u);

} // namespace Uuid
} // namespace Util
} // namespace SilKit