// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ExecutionEnvironment.hpp"

#include "Assert.hpp"
#include "FileHelpers.hpp"

#include <array>
#include <string>
#include <vector>

#include "asio.hpp"

#include "fmt/format.h"

#ifdef __unix__
#include <unistd.h>
#include <sys/utsname.h>
#endif

#ifdef __linux__
#include <sys/sysinfo.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <lmcons.h>
#include <psapi.h>
#endif

namespace VSilKit {

namespace {

constexpr const char* UNKNOWN_VALUE = "<unknown>";

#ifdef __unix__

auto GetUsername() -> std::string
{
    std::array<char, 512> username{};
    SILKIT_ASSERT(::getlogin_r(username.data(), sizeof(username) - 1) == 0);
    return std::string{username.data()};
}

#ifdef __linux__

auto GetProcessExecutable() -> std::string
{
    std::vector<char> buffer;
    buffer.resize(4096, '\0');

    auto pathLength = ::readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    SILKIT_ASSERT(pathLength > 0);

    buffer.resize(static_cast<std::size_t>(pathLength));
    return std::string{buffer.begin(), buffer.end()};
}

#else

auto GetProcessExecutable() -> std::string
{
    return UNKNOWN_VALUE;
}

#endif

auto GetProcessorCount() -> std::string
{
    static const auto result = std::to_string(::get_nprocs());
    return result;
}

auto GetPageSize() -> std::string
{
    static const auto result = std::to_string(::sysconf(_SC_PAGESIZE));
    return result;
}

#ifdef __linux__

auto GetPhysicalMemoryMB() -> std::string
{
    static const auto result = [] {
        const auto pageSize = static_cast<std::uint64_t>(::sysconf(_SC_PAGESIZE));
        const auto pageCount = static_cast<std::uint64_t>(::sysconf(_SC_PHYS_PAGES));
        const auto physicalMemoryMiB = ((pageCount / 1024) * static_cast<std::uint64_t>(pageSize)) / 1024;
        return std::to_string(physicalMemoryMiB);
    }();
    return result;
}

#else

auto GetPhysicalMemoryMB() -> std::string
{
    return UNKNOWN_VALUE;
}

#endif

auto GetUtsname() -> const struct ::utsname&
{
    static const auto result = [] {
        struct ::utsname buffer = {};
        SILKIT_ASSERT(::uname(&buffer) == 0);
        return buffer;
    }();

    return result;
}

auto GetProcessorArchitecture() -> std::string
{
    return GetUtsname().machine;
}

auto GetOperatingSystem() -> std::string
{
    return fmt::format("{} {}", GetUtsname().sysname, GetUtsname().release);
}

#endif

#ifdef _WIN32

auto GetUsername() -> std::string
{
    static const auto result = [] {
        std::array<char, UNLEN + 1> username{};
        auto usernameLength = static_cast<DWORD>(username.size());
        SILKIT_ASSERT(::GetUserNameA(username.data(), &usernameLength) != 0);
        return std::string{username.data()};
    }();

    return result;
}

auto GetProcessExecutable() -> std::string
{
    static const auto result = [] {
        std::vector<char> buffer;
        buffer.resize(4096, '\0');

        const auto pathLength = ::GetModuleFileNameA(NULL, buffer.data(), static_cast<DWORD>(buffer.size()));
        SILKIT_ASSERT(pathLength != 0);
        SILKIT_ASSERT(pathLength <= buffer.size());

        buffer.resize(static_cast<std::size_t>(pathLength));
        return std::string{buffer.begin(), buffer.end()};
    }();

    return result;
}

struct Win32ProcessorInformation
{
    std::string pageSize;
    std::string processorCount;
    std::string architecture;

    static auto Get() -> const Win32ProcessorInformation&
    {
        static const auto result = [] {
            ::SYSTEM_INFO systemInfo{};
            ::GetNativeSystemInfo(&systemInfo);

            Win32ProcessorInformation info;

            info.pageSize = std::to_string(systemInfo.dwPageSize);

            switch (systemInfo.wProcessorArchitecture)
            {
            case PROCESSOR_ARCHITECTURE_AMD64:
                info.architecture = "x86_64";
                break;
            case PROCESSOR_ARCHITECTURE_ARM:
                info.architecture = "arm";
                break;
            case PROCESSOR_ARCHITECTURE_ARM64:
                info.architecture = "arm64";
                break;
            case PROCESSOR_ARCHITECTURE_INTEL:
                info.architecture = "x86";
                break;
            default:
                info.architecture = "<unknown>";
                break;
            }

            info.processorCount = std::to_string(systemInfo.dwNumberOfProcessors);

            return info;
        }();

        return result;
    }
};

auto GetProcessorCount() -> std::string
{
    return Win32ProcessorInformation::Get().processorCount;
}

auto GetProcessorArchitecture() -> std::string
{
    return Win32ProcessorInformation::Get().architecture;
}

auto GetPageSize() -> std::string
{
    return Win32ProcessorInformation::Get().pageSize;
}

auto GetPhysicalMemoryMB() -> std::string
{
    const auto result = [] {
        ULONGLONG value{};
        SILKIT_ASSERT(::GetPhysicallyInstalledSystemMemory(&value) == TRUE);
        return std::to_string(value / 1024);
    }();
    return result;
}

auto GetOperatingSystem() -> std::string
{
    return "Windows";
}

#endif

} // namespace

auto GetExecutionEnvironment() -> ExecutionEnvironment
{
    ExecutionEnvironment ee;
    ee.hostname = asio::ip::host_name();
    ee.username = GetUsername();
    ee.executable = GetProcessExecutable();
    ee.processorArchitecture = GetProcessorArchitecture();
    ee.processorCount = GetProcessorCount();
    ee.pageSize = GetPageSize();
    ee.physicalMemoryMiB = GetPhysicalMemoryMB();
    ee.operatingSystem = GetOperatingSystem();

    return ee;
}

} // namespace VSilKit
