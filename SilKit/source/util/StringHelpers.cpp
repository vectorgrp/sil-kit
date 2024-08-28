// Copyright (c) 2022 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "StringHelpers.hpp"

#include <chrono>
#include <sstream>

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


} // namespace Util
} // namespace SilKit
