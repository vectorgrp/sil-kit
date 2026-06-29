// SPDX-FileCopyrightText: 2023-2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

#include <cctype>


namespace SilKit {
namespace Util {


struct EscapedJsonString
{
    const std::string& string;

    friend auto operator<<(std::ostream& ostream, const EscapedJsonString& self) -> std::ostream&;
};

auto EscapeString(const std::string& input) -> std::string;

auto CurrentTimestampString() -> std::string;

auto LowerCase(std::string input) -> std::string;

auto PrintableString(const std::string& input) -> std::string;

auto SplitString(std::string_view input, const std::string_view& separator) -> std::vector<std::string>;

} // namespace Util
} // namespace SilKit


namespace SilKit {
namespace Util {

inline auto LowerCase(std::string input) -> std::string
{
    // Note: std::tolower has undefined behavior if the argument is not representable as unsigned char.
    std::transform(input.begin(), input.end(), input.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(static_cast<int>(static_cast<unsigned char>(c))));
    });
    return input;
}

} // namespace Util
} // namespace SilKit
