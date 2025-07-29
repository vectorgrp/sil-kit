// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <iosfwd>

#include <cstdint>

namespace SilKit {
namespace Util {

//! Represents a 128 bit UUID.
struct Uuid
{
    uint64_t ab;
    uint64_t cd;

    //! Generate a random Uuid.
    static auto GenerateRandom() -> Uuid;
};

bool operator==(const Uuid& lhs, const Uuid& rhs);
bool operator!=(const Uuid& lhs, const Uuid& rhs);
bool operator<(const Uuid& lhs, const Uuid& rhs);

auto operator<<(std::ostream& ostream, const Uuid& uuid) -> std::ostream&;

auto to_string(const Uuid& uuid) -> std::string;

} // namespace Util
} // namespace SilKit