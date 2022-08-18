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

#include "silkit/participant/exception.hpp"

#include "Filesystem.hpp"

#include <array>
#include <sstream>

#if defined(_WIN32)
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MAN
#   endif
#   include <windows.h>
#   include <direct.h>
#   define getcwd _getcwd
#   define chdir _chdir
#   define mkdir(X, Y) _mkdir(X)

namespace {
auto platform_temp_directory() -> std::string
{
    std::array<char, 4096> buffer;
    auto len = ::GetTempPathA(static_cast<DWORD>(buffer.size()), buffer.data());
    return std::string{ buffer.data(), len };
}
#else
// Assume Linux/POSIX
#   include <unistd.h>
#   include <stdio.h>
#   include <sys/stat.h>
#   include <sys/types.h>
#   include <errno.h>
#   include <string.h>
namespace {
auto platform_temp_directory() -> std::string
{
    return { "/tmp" }; 
}
#endif
} //end anonymous namespace
namespace SilKit {
namespace Filesystem {

path::path(const string_type& source)
    :_path{ source }
{
}
auto path::string() const -> std::string
{
    return _path;
}
auto path::c_str() const noexcept -> const value_type*
{
    return _path.c_str();
}
auto path::native() const noexcept -> const string_type&
{
    return _path;
}


// Functions
path current_path()
{
    std::array<char, 4096> buffer;
    if (::getcwd(buffer.data(), static_cast<int>(buffer.size())) == nullptr)
    {
        throw SilKitError("Couldn't get current working directory.");
    }

    return std::string(buffer.data());
}

void current_path(const path& newPath)
{
    if (chdir(newPath.c_str()) != 0)
    {
        std::stringstream msg;
        msg << "filsystem::current_path: Couldn't set the current working directory to \""
        << newPath.string()
        << "\""
        ;
        throw SilKitError(msg.str());
    }
}
path temp_directory_path()
{
    return platform_temp_directory();
}

bool remove(const path& p)
{
    int e = ::remove(p.c_str());
    return e == 0;
}

void rename(const path& old_p, const path& new_p)
{
#if defined(_WIN32)
    // Attempt to mirror the behavior of the POSIX remove function (https://en.cppreference.com/w/cpp/filesystem/rename)
    // which removes new_p prior to moving old_p into new_p. Failure of remove includes non-existing new_p, therefore
    // we ignore the failure.
    (void)remove(new_p);
#endif

    // NB: On POSIX systems rename already removes new_p and handles the rename atomically.
    if (::rename(old_p.c_str(), new_p.c_str()) != 0)
    {
        std::perror("rename");
        throw SilKitError("filesystem::rename: Couldn't rename the old filename to the new filename");
    }
}

bool create_directory(const path& where)
{
    auto status = ::mkdir(where.c_str(), 0755);
    if(status == 0)
    {
        return status == 0;
    }
    else
    {
        return errno == EEXIST;
    }
}
} // namespace Filesystem
} // namespace SilKit
