// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

namespace ib {
namespace util {
namespace uuid {

// UUID implementation based on 2x64bit random numbers
struct UUID
{
    uint64_t ab;
    uint64_t cd;
};

inline bool operator==(const UUID& lhs, const UUID& rhs) { return lhs.ab == rhs.ab && lhs.cd == rhs.cd; }
inline bool operator!=(const UUID& lhs, const UUID& rhs) { return lhs.ab != rhs.ab || lhs.cd != rhs.cd; }

UUID        generate();
std::string to_string(const UUID& u);

} // namespace uuid
} // namespace util
} // namespace ib