// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <thread>
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <chrono>
#include <string>
#include <fstream>
#include <random>
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <system_error>
#include <vector>

#if defined(__unix__)
#include <errno.h>
#include <string.h>
#include <stdio.h>
#endif //__unix__

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX // keep std::min/std::max and std::numeric_limits<>::min/max usable
#endif
#include <Windows.h> //for 'HANDLE'
#endif               //__WIN32

#include "silkit/participant/exception.hpp"

namespace IntegrationTestUtils {

struct Barrier
{
    std::mutex mx;
    std::condition_variable cv;
    std::atomic_uint expected{0};
    std::atomic_uint have{0};
    std::chrono::seconds timeout{1};

    Barrier(const Barrier&) = delete;
    Barrier() = delete;

    Barrier(unsigned expectedEntries, std::chrono::seconds timeout)
        : expected{expectedEntries}
        , timeout{timeout}
    {
    }

    ~Barrier()
    {
        if (have < expected)
        {
            std::cout << "Barrier: error in destructor: have=" << have << " expected=" << expected << std::endl;
            //wakeup dormant threads
            have.store(expected);
            cv.notify_all();
        }
    }

    void Enter()
    {
        std::unique_lock<decltype(mx)> lock(mx);
        have++;
        if (have >= expected)
        {
            lock.unlock();
            cv.notify_all();
        }
        else
        {
            auto ok = cv.wait_for(lock, timeout, [this] { return have == expected; });
            if (!ok)
            {
                std::stringstream ss;
                ss << "Barrier Enter: timeout! have=" << have << " expected=" << expected;
                std::cout << ss.str() << std::endl;

                throw SilKit::SilKitError(ss.str()); //abort test!
            }
        }
    }
};

struct Pipe
{
    using buffer_t = std::vector<char>;
    Pipe() = delete;
#ifdef _WIN32
    HANDLE handle = INVALID_HANDLE_VALUE;

    Pipe(const std::string& pipeName)
    {
        auto path = R"(\\.\pipe\)" + pipeName;

        handle = CreateFileA(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (handle == INVALID_HANDLE_VALUE)
        {
            throw SilKit::SilKitError("Cannot open WIN32 pipe " + path);
        }
    }

    ~Pipe()
    {
        if (handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(handle);
            handle = INVALID_HANDLE_VALUE;
        }
    }

    buffer_t Read(size_t size)
    {
        DWORD actualRead = 0;
        buffer_t buf{};
        buf.resize(size);
        auto ok = ReadFile(handle, buf.data(), static_cast<DWORD>(buf.size()), &actualRead, nullptr);

        if (!ok)
        {
            return buffer_t{};
        }

        if (actualRead < size)
        {
            buf.resize(actualRead);
        }
        return buf;
    }

#else
    //Linux impl
    FILE* file{nullptr};

    Pipe(const std::string& pipeName)
    {
        file = fopen(pipeName.c_str(), "r");
        if (file == nullptr)
        {
            throw SilKit::SilKitError("Cannot open linux pipe " + pipeName);
        }
    }

    ~Pipe()
    {
        if (file != nullptr)
        {
            auto ok = fclose(file);
            if (ok != 0)
            {
                std::cout << "Fclose on linux pipe failed: " << strerror(errno) << std::endl;
            }
            file = nullptr;
        }
    }

    buffer_t Read(size_t size)
    {
        buffer_t buf{};
        buf.resize(size);
        auto actual = fread(buf.data(), 1, buf.size(), file);
        if (actual == 0)
        {
            if (feof(file) != 0)
            {
                return {};
            }
            else
            {
                throw SilKit::SilKitError("Read on linux pipe failed: " + std::to_string(ferror(file)));
            }
        }
        if (actual < size)
        {
            buf.resize(actual);
        }

        return buf;
    }
#endif
};

inline size_t getFileSize(const std::string& name)
{
    auto ifs = std::ifstream{name, std::ios::binary | std::ios::ate};
    return ifs.tellg();
}

inline bool fileExists(const std::string& name)
{
    auto ifs = std::ifstream{name, std::ios::in};
    return ifs.good();
}

inline void removeTempFile(const std::string& fileName)
{
#if defined(_WIN32)
    auto ok = DeleteFileA(fileName.c_str());
    if (!ok)
    {
        std::cout << "ERROR: removeTempFile failed!" << std::endl;
    }
#else
    auto ok = unlink(fileName.c_str());
    if (ok == -1)
    {
        std::cout << "ERROR: removeTempFile failed: " << strerror(errno) << std::endl;
    }
#endif
}

inline std::string randomString(size_t len)
{
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                     "abcdefghijklmnopqrstuvwxyz"
                                     "0123456789"
                                     "_-";
    static std::default_random_engine re{std::random_device{}()};
    static std::uniform_int_distribution<std::string::size_type> randPick(0, chars.size() - 1);

    std::string rv;
    rv.resize(len);
    std::generate_n(rv.begin(), len, [&]() { return chars.at(randPick(re)); });
    return rv;
}


// File sinks write "<LogName>_<ParticipantName>_<Timestamp>.txt" (or ".jsonl") into the working directory, so a log
// file can only be found by its prefix. The unique suffix keeps concurrent or leftover test runs apart.
inline auto MakeUniqueLogName(const std::string& base) -> std::string
{
    return base + "_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
}

// Tests run in parallel processes that share the working directory, so files may vanish while it is scanned. Only the
// non-throwing overloads are used: a throwing directory_iterator in a destructor would terminate the test process.
inline auto FindLogFiles(const std::string& prefix) -> std::vector<std::filesystem::path>
{
    std::vector<std::filesystem::path> logFiles;
    std::error_code ec;
    for (std::filesystem::directory_iterator it{std::filesystem::current_path(), ec}, end; !ec && it != end;
         it.increment(ec))
    {
        std::error_code statusEc;
        if (it->is_regular_file(statusEc) && it->path().filename().string().rfind(prefix, 0) == 0)
        {
            logFiles.push_back(it->path());
        }
    }
    return logFiles;
}

inline auto ReadTextFile(const std::filesystem::path& path) -> std::string
{
    std::ifstream stream{path};
    std::stringstream contents;
    contents << stream.rdbuf();
    return contents.str();
}

// Removes all log files starting with the given prefix when going out of scope.
class ScopedLogFiles
{
public:
    explicit ScopedLogFiles(std::string prefix)
        : _prefix{std::move(prefix)}
    {
    }

    ~ScopedLogFiles()
    {
        for (const auto& logFile : FindLogFiles(_prefix))
        {
            std::error_code ec;
            std::filesystem::remove(logFile, ec);
        }
    }

    ScopedLogFiles(const ScopedLogFiles&) = delete;
    ScopedLogFiles& operator=(const ScopedLogFiles&) = delete;

    auto Prefix() const -> const std::string&
    {
        return _prefix;
    }

    // Concatenated contents of all matching files. Empty if no file exists.
    auto ReadAll() const -> std::string
    {
        std::string contents;
        for (const auto& logFile : FindLogFiles(_prefix))
        {
            contents += ReadTextFile(logFile);
        }
        return contents;
    }

private:
    std::string _prefix;
};


} // end namespace IntegrationTestUtils
