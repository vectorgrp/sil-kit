// SPDX-FileCopyrightText: 2022-2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace SilKit {
namespace detail {

#if defined(_WIN32)

using LibraryHandle = HMODULE;

#else

using LibraryHandle = void*;

#endif

// filesystem specific constants, because we don't have C++17 filesystem, yet
extern const std::string lib_file_extension;
extern const std::string lib_prefix;

LibraryHandle OpenLibrary(const std::string& path);
void* FindSymbol(LibraryHandle& hnd, const std::string& symbol_name);
void CloseLibrary(const LibraryHandle& hnd);
std::string GetProcessPath();
} //end namespace detail

} //end namespace SilKit
