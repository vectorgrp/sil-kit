// SPDX-FileCopyrightText: 2023-2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <string_view>
#include <vector>


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
