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

#include "FileHelpers.hpp"

#include "silkit/participant/exception.hpp"

#include <sstream>

// Check if MSVC is used, since MINGW32/64 don't need these workarounds
#ifdef _MSC_VER
#    define NOMINMAX
#    define WIN32_LEAN_AND_MEAN
#    include "Windows.h"
#endif

namespace SilKit {
namespace Util {

#ifdef _MSC_VER

auto OpenIFStream(const std::string& path) -> std::ifstream
{
    std::wstring widePath;

    constexpr DWORD MB2WC_FLAGS = 0;

    const auto pathSize = static_cast<int>(path.size());

    // determine the size of the required wide-character string
    const int widePathSize = ::MultiByteToWideChar(CP_UTF8, MB2WC_FLAGS, path.c_str(), pathSize, NULL, 0);
    if (widePathSize <= 0)
    {
        throw SilKit::ConfigurationError{"conversion from UTF-8 to UTF-16 failed"};
    }

    widePath.resize(static_cast<size_t>(widePathSize));

    // actually convert the path into a wide-character string
    const int realWidePathSize =
        ::MultiByteToWideChar(CP_UTF8, MB2WC_FLAGS, path.c_str(), pathSize, &widePath[0], widePathSize);
    if (realWidePathSize != widePathSize)
    {
        throw SilKit::ConfigurationError{"conversion from UTF-8 to UTF-16 failed"};
    }

    return std::ifstream{widePath};
}

#else

auto OpenIFStream(const std::string& path) -> std::ifstream
{
    return std::ifstream{path};
}

#endif

auto ReadTextFile(const std::string& path) -> std::string
{
    auto fs = OpenIFStream(path);

    if (!fs.is_open())
        throw SilKit::ConfigurationError("File '" + path + "' could not be opened");

    std::stringstream buffer;
    buffer << fs.rdbuf();

    return buffer.str();
}

} // namespace Util
} // namespace SilKit
