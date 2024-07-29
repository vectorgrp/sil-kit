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

#include "fmt/chrono.h"


namespace SilKit {
namespace Util {


auto EscapeString(const std::string& input) -> std::string
{
    std::string result;
    result.reserve(input.size() * 2); // Reserve enough memory for the result

    size_t i = 0;
    while (i < input.size())
    {
        char character = input[i];
        switch (character)
        {
            case '\b':
            {
                result += "\\";
                result += 'b';
                break;
            }
            case '\t':
            {
                result += "\\";
                result += 't';
                break;
            }
            case '\n':
            {
                result += "\\";
                result += 'n';
                break;
            }
            case '\f':
            {
                result += "\\";
                result += 'f';
                break;
            }
            case '\r':
            {
                result += "\\";
                result += 'r';
                break;
            }
            case '\\':
            {
                result += "\\";
                result += character;
                break;
            }
            case '\"':
            {
                result += "\\";
                result += character;
                break;
            }
            default:
            {
                result += character;
                break;
            }
        }
        i++;
    }
    return result;
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


} // namespace Util
} // namespace SilKit
