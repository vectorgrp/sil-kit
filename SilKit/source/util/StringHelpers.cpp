// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "StringHelpers.hpp"

#include <chrono>
#include <sstream>
#include <cctype>

#include "fmt/chrono.h"


namespace SilKit {
namespace Util {


namespace {

template <typename F>
void DoEscape(const std::string& string, F f)
{
    for (const char ch : string)
    {
        switch (ch)
        {
        case '\b':
            f('\\');
            f('b');
            break;
        case '\t':
            f('\\');
            f('t');
            break;
        case '\n':
            f('\\');
            f('n');
            break;
        case '\f':
            f('\\');
            f('f');
            break;
        case '\r':
            f('\\');
            f('r');
            break;
        case '"':
            f('\\');
            f('"');
            break;
        case '\\':
            f('\\');
            f('\\');
            break;
        default:
            f(ch);
            break;
        }
    }
}

} // namespace


auto EscapeString(const std::string& input) -> std::string
{
    std::size_t count{0};
    DoEscape(input, [&count](const auto) { ++count; });

    std::string result;
    result.reserve(count);
    DoEscape(input, [&result](const auto ch) { result.push_back(ch); });

    return result;
}


auto operator<<(std::ostream& ostream, const EscapedJsonString& self) -> std::ostream&
{
    DoEscape(self.string, [&ostream](const auto ch) { ostream.put(ch); });
    return ostream;
}


auto CurrentTimestampString() -> std::string
{
    auto time = std::time(nullptr);

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif

    return fmt::format("{:%FT%H-%M-%S}", tm);
}

auto LowerCase(std::string input) -> std::string
{
    std::transform(input.begin(), input.end(), input.begin(),
                   [](unsigned char c) { return (unsigned char)std::tolower(c); });
    return input;
}

auto PrintableString(const std::string& participantName) -> std::string
{
    std::string safeName;
    for (const auto& ch : participantName)
    {
        // do not use std::isalnum, as it may sensitive to the current locale
        const bool isAlphaNumeric{('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9')
                                  || (ch == '_' || ch == '-' || ch == '.' || ch == '~')};

        if (isAlphaNumeric)
        {
            safeName.push_back(ch);
        }
        else
        {
            safeName += fmt::format("{:02X}", static_cast<unsigned char>(ch));
        }
    }
    return safeName;
}

auto SplitString(std::string_view input, const std::string_view& separator) -> std::vector<std::string>
{
    std::vector<std::string> tokens;
    for (auto i = input.find(separator); i != input.npos; i = input.find(separator))
    {
        tokens.emplace_back(input.substr(0, i));
        input = input.substr(i + 1);
    }
    if (!input.empty())
    {
        tokens.emplace_back(std::move(input));
    }
    return tokens;
}

} // namespace Util
} // namespace SilKit
