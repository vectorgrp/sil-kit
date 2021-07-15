#include "Filesystem.hpp"

#include <array>
#include <stdexcept>

#if defined(_WIN32)
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MAN
#   endif
#   include <Windows.h>
#   include <direct.h>
#   define getcwd _getcwd

namespace {
auto platform_temp_directory() -> std::string
{
    std::array<char, 4096> buffer;
    auto len = ::GetTempPathA(static_cast<DWORD>(buffer.size()), buffer.data());
    return std::string{ buffer.data(), len };
}
#else
// Assume Linux
#   include <unistd.h>
#   include <stdio.h>
namespace {
auto platform_temp_directory() -> std::string
{
    return { "/tmp" }; 
}
#endif
} //end anonymous namespace
namespace ib {
namespace filesystem {

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
        throw std::runtime_error("Couldn't get current working directory.");
    }

    return std::string(buffer.data());
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
} // namespace filesystem
} // namespace ib
