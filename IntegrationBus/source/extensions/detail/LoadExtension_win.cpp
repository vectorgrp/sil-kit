// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include <vector>
#include <string>

#include <cstdlib> //getenv
#include <io.h> //access

#include "IbExtensions.hpp"
#include "LoadExtension.hpp"


#if !defined( F_OK )
#   define F_OK 00 //not provided by io.h, but used in _access()
#endif


namespace ib { namespace extensions { namespace detail {
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

}//detail
}//extensions
}//ib
