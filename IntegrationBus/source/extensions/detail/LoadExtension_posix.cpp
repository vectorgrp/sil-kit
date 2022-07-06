// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.


#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>

#include <dlfcn.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>

#include "SilKitExtensions.hpp"
#include "LoadExtension.hpp"


namespace SilKit { namespace detail {

//used by FindLibrary
const std::string lib_file_extension=".so";
const std::string lib_prefix="lib";
const std::string path_sep = "/";

bool FileExists(const std::string& path)
{
    return ::access(path.c_str(), F_OK) == 0;
}

LibraryHandle OpenLibrary(const std::string& path)
{
    void* tmp = ::dlopen(path.c_str(), RTLD_NOW);
    if(tmp == nullptr)
    {
        throw ExtensionError("::dlopen() failed: " + std::string{dlerror()});
    }
    return tmp;
}

void* FindSymbol(LibraryHandle& hnd, const std::string& symbol_name)
{
    void* sym = dlsym(hnd, symbol_name.c_str()); 
    if(sym == nullptr)
    {
        throw ExtensionError("Calling dlsym() failed: Could not find \""
                + symbol_name +"\"" + std::string{dlerror()}
        );
    }
    return sym;
}

void CloseLibrary(const LibraryHandle& hnd)
{
    int e = ::dlclose(hnd);
    if (e != 0)
    {
        std::string tmp{dlerror()};
        throw ExtensionError("Calling ::dlclose failed: " + tmp);
    }
}

// Linux specific executable image path
std::string GetProcessPath()
{
    std::vector<char> buf;
    buf.resize(PATH_MAX);
    auto proc = std::string("/proc/") + std::to_string(getpid()) + "/exe";
    auto nb = readlink(proc.c_str(), buf.data(), buf.size());
    if ( nb < 0)
    {
        return "."; //XXX better than empty string
    }
    auto unb = static_cast<size_t>(nb);
    buf.at(unb) = '\0';
    return std::string(buf.data(), unb);
}

}//detail

}//silkit
