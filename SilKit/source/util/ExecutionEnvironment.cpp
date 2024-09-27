// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ExecutionEnvironment.hpp"

#include "Assert.hpp"
#include "FileHelpers.hpp"

#include <array>
#include <string>
#include <thread>
#include <vector>

#include "asio.hpp"

#include "fmt/format.h"

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#include <sys/utsname.h>
#include <pwd.h>
#endif

#ifdef __linux__
#include <sys/sysinfo.h>
#include <sys/types.h>
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


// Utilities

#if defined(__unix__) || defined(__APPLE__)

auto GetUtsname() -> const struct ::utsname&
{
    static const auto result = [] {
        struct ::utsname buffer = {};
        SILKIT_ASSERT(::uname(&buffer) == 0);
        return buffer;
    }();

    return result;
}

#endif

#ifdef _WIN32

struct Win32ProcessorInformation
{
    std::string pageSize;
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

            return info;
        }();

        return result;
    }
};

#endif


// Function: GetUsername

#if defined(__unix__) || defined(__APPLE__)

auto GetUsername() -> std::string
{
    static const auto result = []() -> std::string {
        const auto uid = ::getuid();
        const auto pwd = ::getpwuid(uid);
        if (pwd == nullptr)
        {
            return UNKNOWN_VALUE;
        }

        return pwd->pw_name;
    }();

    return result;
}

#endif

#ifdef _WIN32

auto GetUsername() -> std::string
{
    static const auto result = []() -> std::string {
        std::array<char, UNLEN + 1> username{};
        auto usernameLength = static_cast<DWORD>(username.size());
        if (::GetUserNameA(username.data(), &usernameLength) == 0)
        {
            return UNKNOWN_VALUE;
        }
        return std::string{username.data()};
    }();

    return result;
}

#endif


// Function: GetProcessExecutable

#ifdef __unix__
#ifdef __linux__

auto GetProcessExecutable() -> std::string
{
    static const auto result = []() -> std::string {
        std::vector<char> buffer;
        buffer.resize(4096, '\0');

        auto pathLength = ::readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
        if (pathLength <= 0)
        {
            return UNKNOWN_VALUE;
        }

        buffer.resize(static_cast<std::size_t>(pathLength));
        return std::string{buffer.begin(), buffer.end()};
    }();

    return result;
}
#else

auto GetProcessExecutable() -> std::string
{
    // should be getprogname on xBSD and macOS, and qh_get_progname on QNX
    return UNKNOWN_VALUE;
}

#endif
#endif


#if defined(__APPLE__)

auto GetProcessExecutable() -> std::string
{
    return {getprogname()};
}
#endif

#ifdef _WIN32

auto GetProcessExecutable() -> std::string
{
    static const auto result = []() -> std::string {
        std::vector<char> buffer;
        buffer.resize(4096, '\0');

        const auto pathLength = ::GetModuleFileNameA(NULL, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (pathLength <= 0 || pathLength > buffer.size())
        {
            return UNKNOWN_VALUE;
        }

        buffer.resize(static_cast<std::size_t>(pathLength));
        return std::string{buffer.begin(), buffer.end()};
    }();

    return result;
}

#endif


// Function: GetPageSize

#if defined(__unix__) || defined(__APPLE__)

auto GetPageSize() -> std::string
{
    static const auto result = std::to_string(::sysconf(_SC_PAGESIZE));
    return result;
}

#endif

#ifdef _WIN32

auto GetPageSize() -> std::string
{
    return Win32ProcessorInformation::Get().pageSize;
}

#endif


// Function: GetPhysicalMemoryMB

#if defined(__unix__) || defined(__APPLE__)
#if defined(__linux__)

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
#endif

#ifdef _WIN32

auto GetPhysicalMemoryMB() -> std::string
{
    const auto result = []() -> std::string {
        ULONGLONG value{};
        if (::GetPhysicallyInstalledSystemMemory(&value) != TRUE)
        {
            return UNKNOWN_VALUE;
        }
        return std::to_string(value / 1024);
    }();

    return result;
}

#endif


// Function: GetProcessorArchitecture

#if defined(__unix__) || defined(__APPLE__)

auto GetProcessorArchitecture() -> std::string
{
    return GetUtsname().machine;
}

#endif

#ifdef _WIN32

auto GetProcessorArchitecture() -> std::string
{
    return Win32ProcessorInformation::Get().architecture;
}

#endif


// Function: GetProcessorArchitecture

#if defined(__unix__) || defined(__APPLE__)

auto GetOperatingSystem() -> std::string
{
    return fmt::format("{} {}", GetUtsname().sysname, GetUtsname().release);
}

#endif

#ifdef _WIN32

auto GetOperatingSystem() -> std::string
{
    return "Windows";
}

#endif


// Function: GetProcessorCount

auto GetProcessorCount() -> std::string
{
    static const auto result = std::to_string(std::thread::hardware_concurrency());
    return result;
}


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
