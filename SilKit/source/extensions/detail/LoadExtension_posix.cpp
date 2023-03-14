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
#include <cstring>
#include <cstdlib>

#include <dlfcn.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>

#include "SilKitExtensions.hpp"
#include "LoadExtension.hpp"


namespace SilKit { namespace detail {

//used by FindLibrary
#if defined(__APPLE__)
const std::string lib_file_extension=".dylib";
#else
const std::string lib_file_extension=".so";
#endif
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
        return ".";
    }
    auto unb = static_cast<size_t>(nb);
    buf.at(unb) = '\0';
    auto* processDir = dirname(buf.data());
    return std::string{processDir};
}

}//detail

}//silkit
