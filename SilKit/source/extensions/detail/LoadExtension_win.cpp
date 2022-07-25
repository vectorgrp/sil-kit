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

#include <vector>
#include <string>

#include <cstdlib> //getenv
#include <io.h> //access

#include "SilKitExtensions.hpp"
#include "LoadExtension.hpp"


#if !defined( F_OK )
#   define F_OK 00 //not provided by io.h, but used in _access()
#endif


namespace SilKit {  namespace detail {
static std::string lastErrorMessage()
{
    std::string rv{};
    DWORD e = ::GetLastError();
    if(e != 0)
    {
        LPSTR buf = nullptr;
        size_t size = FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_FROM_SYSTEM
                | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                e,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPSTR)&buf,
                0,
                nullptr);

        rv.assign(buf, size);
        LocalFree(buf);
    }
    return rv;
}


//used by FindLibrary
const std::string lib_file_extension=".dll";
const std::string lib_prefix="";
const std::string path_sep="\\";

bool FileExists(const std::string& path)
{
    return ::_access(path.c_str(), F_OK) == 0;
}

LibraryHandle OpenLibrary(const std::string& path)
{
    LibraryHandle tmp = ::LoadLibraryA(path.c_str());
    if(tmp == nullptr)
    {
        throw ExtensionError("::LoadLibrary() failed: " + lastErrorMessage());
    }
    return tmp;
}

void CloseLibrary(const LibraryHandle& hnd)
{
    BOOL ok = ::FreeLibrary(hnd);
    if(!ok)
    {
        throw ExtensionError(
                    "Calling FreeLibrary failed: " + lastErrorMessage());
    }
}

void* FindSymbol(LibraryHandle& hnd, const std::string& symbol_name)
{
    auto* entry = (void*)::GetProcAddress(hnd, symbol_name.c_str());
    if(entry == nullptr)
    {
        throw ExtensionError(
                "Calling GetProcAddress() failed: Could not find \""
                + symbol_name +"\"" + lastErrorMessage()
        );
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

}//detail

}//silkit
