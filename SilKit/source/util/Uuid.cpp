// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "Uuid.hpp"

#include <limits>
#include <random>
#include <iomanip>
#include <sstream>

namespace SilKit {
namespace Util {

bool operator==(const Uuid& lhs, const Uuid& rhs)
{
    return lhs.ab == rhs.ab && lhs.cd == rhs.cd;
}
bool operator!=(const Uuid& lhs, const Uuid& rhs)
{
    return lhs.ab != rhs.ab || lhs.cd != rhs.cd;
}

bool operator<(const Uuid& lhs, const Uuid& rhs)
{
    if (lhs.ab < rhs.ab)
    {
        return true;
    }
    else if (lhs.ab > rhs.ab)
    {
        return false;
    }
    else
    {
        return lhs.cd < rhs.cd;
    }
}

// Version 4, variant 8 UUID Generator
auto Uuid::GenerateRandom() -> Uuid
{
    thread_local static std::mt19937 gen(std::random_device{}()); // Seed with random_device
    thread_local static std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());

    Uuid res;
    res.ab = dist(gen);
    res.cd = dist(gen);

    // First entry of third block is UUID Version (Fixed to type 4)
    res.ab = (res.ab & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
    // First entry of fourth block is UUID Variant (8,9,A,B)
    res.cd = (res.cd & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

    return res;
}

auto operator<<(std::ostream& ostream, const Uuid& uuid) -> std::ostream&
{
    return ostream << to_string(uuid);
}

auto to_string(const Uuid& u) -> std::string
{
    std::stringstream ss;
    ss << std::hex << std::nouppercase << std::setfill('0');

    uint32_t a = (u.ab >> 32);
    uint32_t b = (u.ab & 0xFFFFFFFF);
    uint32_t c = (u.cd >> 32);
    uint32_t d = (u.cd & 0xFFFFFFFF);

    ss << std::setw(8) << (a) << '-';
    ss << std::setw(4) << (b >> 16) << '-';
    ss << std::setw(4) << (b & 0xFFFF) << '-';
    ss << std::setw(4) << (c >> 16) << '-';
    ss << std::setw(4) << (c & 0xFFFF);
    ss << std::setw(8) << d;

    return ss.str();
}

} // namespace Util
} // namespace SilKit