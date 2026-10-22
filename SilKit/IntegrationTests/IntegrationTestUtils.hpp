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
#include <cstdint>

#if !defined(_WIN32)
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h> // open
#include <unistd.h> // unlink, read, close
#endif

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

// Reads everything written to a named pipe on a background thread, until the writer closes it. The reader waits up
// to 10s for the pipe to be created, so it can be started before the writer.
// On Windows, pipeName is the name below \\.\pipe\, elsewhere it is the path of the FIFO.
class PipeReader
{
public:
    explicit PipeReader(std::string pipeName)
        : _thread{[this, pipeName] { Read(pipeName); }}
    {
    }

    ~PipeReader()
    {
        _abort = true;
        if (_thread.joinable())
        {
            _thread.join();
        }
    }

    auto Join() -> std::vector<uint8_t>
    {
        _thread.join();
        return _bytes;
    }

private:
    void Read(const std::string& pipeName)
    {
        using namespace std::chrono_literals;
        const auto deadline = std::chrono::steady_clock::now() + 10s;
#if defined(_WIN32)
        const auto path = R"(\\.\pipe\)" + pipeName;
        HANDLE handle = INVALID_HANDLE_VALUE;
        while (handle == INVALID_HANDLE_VALUE && !_abort && std::chrono::steady_clock::now() < deadline)
        {
            handle = CreateFileA(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
            if (handle == INVALID_HANDLE_VALUE)
            {
                std::this_thread::sleep_for(10ms);
            }
        }
        if (handle == INVALID_HANDLE_VALUE)
        {
            return;
        }
        // Reading ends with ERROR_BROKEN_PIPE when the writer closes the pipe.
        char buffer[65536];
        DWORD numRead = 0;
        while (ReadFile(handle, buffer, sizeof(buffer), &numRead, nullptr) || GetLastError() == ERROR_MORE_DATA)
        {
            _bytes.insert(_bytes.end(), buffer, buffer + numRead);
            numRead = 0;
        }
        CloseHandle(handle);
#else
        while (!std::filesystem::exists(pipeName) && !_abort && std::chrono::steady_clock::now() < deadline)
        {
            std::this_thread::sleep_for(10ms);
        }
        const int fd = ::open(pipeName.c_str(), O_RDONLY);
        if (fd < 0)
        {
            return;
        }
        char buffer[65536];
        ssize_t numRead = 0;
        while ((numRead = ::read(fd, buffer, sizeof(buffer))) > 0)
        {
            _bytes.insert(_bytes.end(), buffer, buffer + numRead);
        }
        ::close(fd);
#endif
    }

    std::atomic<bool> _abort{false};
    std::vector<uint8_t> _bytes;
    std::thread _thread;
};

inline size_t getFileSize(const std::string& name)
{
    auto ifs = std::ifstream{name, std::ios::binary | std::ios::ate};
    return static_cast<size_t>(ifs.tellg());
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

// The prefix of the files a File sink with the given LogName writes, for all participants or for one participant.
inline auto LogFilePrefix(const std::string& logName) -> std::string
{
    return logName + "_";
}

inline auto LogFilePrefix(const std::string& logName, const std::string& participantName) -> std::string
{
    return LogFilePrefix(logName) + participantName + "_";
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
