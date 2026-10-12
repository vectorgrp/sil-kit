// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <regex>
#include <string>


namespace VSilKit {


// IPv6 literals appear in URIs enclosed in brackets, e.g., "[::1]", while asio expects them without.

inline auto IsIpV6(const std::string& string) -> bool
{
    static std::regex regex{R"(^\[[0-9A-Fa-f:.]+(?:%[0-9A-Za-z_.-]+)?\]$)", std::regex::optimize};
    return std::regex_match(string, regex);
}

inline auto CleanIpAddress(const std::string& string) -> std::string
{
    static std::regex regex{R"(^\[([0-9A-Fa-f:.]+(?:%[0-9A-Za-z_.-]+)?)\]$)", std::regex::optimize};

    std::smatch matches;
    if (std::regex_match(string, matches, regex))
    {
        return matches[1].str();
    }

    return string;
}


} // namespace VSilKit
