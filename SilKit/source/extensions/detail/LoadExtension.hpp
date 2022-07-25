/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include <string>

#if defined(_WIN32)
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif

namespace SilKit { namespace detail {

#if defined(_WIN32)

using LibraryHandle = HMODULE;

#else

using LibraryHandle = void*;

#endif

// filesystem specific constants, because we don't have C++17 filesystem, yet
extern const std::string lib_file_extension;
extern const std::string lib_prefix;
extern const std::string path_sep;

bool FileExists(const std::string& path);
LibraryHandle OpenLibrary(const std::string& path);
void* FindSymbol(LibraryHandle& hnd, const std::string& symbol_name);
void CloseLibrary(const LibraryHandle& hnd);
std::string GetProcessPath();
} //end namespace detail

} //end namespace SilKit
