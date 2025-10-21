// SPDX-FileCopyrightText: 2022-2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <vector>
#include <string>

#include <cstdlib> //getenv
#include <io.h>    //access

#include "SilKitExtensions.hpp"
#include "LoadExtension.hpp"


namespace SilKit {
namespace detail {
static std::string lastErrorMessage()
{
    std::string rv{};
    DWORD e = ::GetLastError();
    if (e != 0)
    {
        LPSTR buf = nullptr;
        size_t size =
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, e, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, nullptr);

        rv.assign(buf, size);
        LocalFree(buf);
    }
    return rv;
}


//used by FindLibrary
const std::string lib_file_extension = ".dll";
const std::string lib_prefix = "";

LibraryHandle OpenLibrary(const std::string& path)
{
    LibraryHandle tmp = ::LoadLibraryA(path.c_str());
    if (tmp == nullptr)
    {
        throw ExtensionError("::LoadLibrary() failed: " + lastErrorMessage());
    }
    return tmp;
}

void CloseLibrary(const LibraryHandle& hnd)
{
    BOOL ok = ::FreeLibrary(hnd);
    if (!ok)
    {
        throw ExtensionError("Calling FreeLibrary failed: " + lastErrorMessage());
    }
}

void* FindSymbol(LibraryHandle& hnd, const std::string& symbol_name)
{
    auto* entry = (void*)::GetProcAddress(hnd, symbol_name.c_str());
    if (entry == nullptr)
    {
        throw ExtensionError("Calling GetProcAddress() failed: Could not find \"" + symbol_name + "\""
                             + lastErrorMessage());
    }
    return entry;
}

std::string GetProcessPath()
{
    std::vector<char> buf;
    buf.resize(MAX_PATH);
    ::GetModuleFileNameA(nullptr, buf.data(), static_cast<DWORD>(buf.size()));
    std::string path(buf.data(), buf.size());

    auto sep = path.find_last_of('\\');
    if (sep != path.npos)
    {
        return path.substr(0, sep);
    }
    return path;
}

} // namespace detail

} // namespace SilKit
