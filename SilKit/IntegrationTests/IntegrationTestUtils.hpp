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

#if __unix__
#include <errno.h>
#include <string.h>
#include <stdio.h>
#endif //__unix__

#if _WIN32
#include <Windows.h> //for 'HANDLE'
#endif//__WIN32

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
    {}

    ~Barrier()
    {
        if(have < expected)
        {
            std::cout << "Barrier: error in destructor: have=" 
                << have
                << " expected=" << expected
                << std::endl;
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
            auto ok = cv.wait_for(lock, timeout, [this] {return have == expected; });
            if (!ok)
            {
                std::stringstream ss;
                ss << "Barrier Enter: timeout! have="
                    << have << " expected=" << expected;
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
#ifdef WIN32
    HANDLE handle = INVALID_HANDLE_VALUE;

    Pipe(const std::string& pipeName)
    {
        auto path = R"(\\.\pipe\)" + pipeName;

        handle = CreateFileA(path.c_str(),
            GENERIC_READ,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr);
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
        auto ok = ReadFile(handle, buf.data(), buf.size(), &actualRead, nullptr);

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
                std::cout << "Fclose on linux pipe failed: " << strerror(errno)
                    << std::endl;
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

size_t getFileSize(const std::string& name)
{
    auto ifs = std::ifstream{name, std::ios::binary | std::ios::ate};
    return ifs.tellg();
}

bool fileExists(const std::string& name)
{
    auto ifs = std::ifstream{name, std::ios::in};
    return ifs.good();
}

void removeTempFile(const std::string& fileName)
{
#if WIN32
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

std::string randomString(size_t len)
{
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789"
        "_-"
        ;
    static std::default_random_engine re{std::random_device{}()};
    static std::uniform_int_distribution<std::string::size_type> randPick(0, chars.size()-1);

    std::string rv;
    rv.resize(len);
    std::generate_n(rv.begin(), len, [&]() { return chars.at(randPick(re)); });
    return rv;
}



} // end namespace IntegrationTestUtils
