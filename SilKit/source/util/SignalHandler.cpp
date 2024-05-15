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
#include "SignalHandler.hpp"
#include <thread>
#include <stdexcept>
#include <vector>

//forward
namespace {
class SignalMonitor;
} // namespace

// global signal handler
static std::unique_ptr<SignalMonitor> gSignalMonitor;

////////////////////////////////////////////
// Inline Platform Specific Implementations
////////////////////////////////////////////
#if WIN32
#include <windows.h>

namespace {
using namespace SilKit::Util;

//forward
BOOL WINAPI systemHandler(DWORD ctrlType);

class SignalMonitor
{
    static constexpr int INVALID_SIGNAL_NUMBER = -1;

public:
    SignalMonitor(SignalHandler handler)
    {
        _handler = std::move(handler);

        auto ok = CreatePipe(&_readEnd, &_writeEnd, nullptr, 0);
        if (!ok)
        {
            throw std::runtime_error("SignalMonitor: Failed to create pipe for signal handler.");
        }
        DWORD nowait = PIPE_NOWAIT;
        ok = SetNamedPipeHandleState(_writeEnd, &nowait, nullptr, nullptr);
        if (!ok)
        {
            throw std::runtime_error(
                "SignalMonitor: Failed to create signal handler, cannot set read end of pipe to non-blocking.");
        }

        SetConsoleCtrlHandler(systemHandler, true);
        _worker = std::thread{std::bind(&SignalMonitor::workerMain, this)};
    }
    ~SignalMonitor()
    {
        SetConsoleCtrlHandler(systemHandler, false);
        Notify(INVALID_SIGNAL_NUMBER);
        _worker.join();
        CloseHandle(_writeEnd);
        CloseHandle(_readEnd);
    }
    void Notify(int signalNumber)
    {
        // No allocs, no error handling
        _signalNumber = signalNumber;
        uint8_t buf{};
        auto ok = WriteFile(_writeEnd, &buf, sizeof(buf), nullptr, nullptr);
        (void)ok;
    }

private:
    void workerMain()
    {
        std::vector<uint8_t> buf(1);
        // Blocking read until Notify() was called
        auto ok = ReadFile(_readEnd, buf.data(), static_cast<DWORD>(buf.size()), nullptr, nullptr);
        if (!ok)
        {
            throw std::runtime_error("SignalMonitor::workerMain: Failed to read from pipe.");
        }

        if ((_signalNumber != INVALID_SIGNAL_NUMBER) && _handler)
        {
            _handler(_signalNumber);
        }
    }

    HANDLE _readEnd{INVALID_HANDLE_VALUE}, _writeEnd{INVALID_HANDLE_VALUE};
    SignalHandler _handler;
    std::thread _worker;
    int _signalNumber{INVALID_SIGNAL_NUMBER};
};

BOOL WINAPI systemHandler(DWORD ctrlType)
{
    if (gSignalMonitor)
    {
        gSignalMonitor->Notify(static_cast<int>(ctrlType));
        return TRUE;
    }
    return FALSE;
}

} // end anonymous namespace

#else //UNIX

#include <csignal>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

namespace {

using namespace SilKit::Util;

//forward
static inline void setSignalAction(int sigNum, __sighandler_t action);
void systemHandler(int sigNum);

    auto ErrorMessage()
{
    return std::string{strerror(errno)};
}

class SignalMonitor
{
    static constexpr int INVALID_SIGNAL_NUMBER = -1;

public:
    SignalMonitor(SignalHandler handler)
    {
        _handler = std::move(handler);

        auto ok = ::pipe(_pipe);
        if (ok == -1)
        {
            throw std::runtime_error("SignalMonitor: Failed to create pipe for signal handler, " + ErrorMessage());
        }
        ok = fcntl(_pipe[1], F_SETFL, O_NONBLOCK);
        if (ok == -1)
        {
            throw std::runtime_error(
                "SignalMonitor: Failed to create signal handler, cannot set read end of pipe to non-blocking: "
                + ErrorMessage());
        }

        setSignalAction(SIGINT, &systemHandler);
        setSignalAction(SIGTERM, &systemHandler);
        _worker = std::thread{std::bind(&SignalMonitor::workerMain, this)};
    }

    ~SignalMonitor()
    {
        // Restore default actio0ns
        setSignalAction(SIGINT, SIG_DFL);
        setSignalAction(SIGTERM, SIG_DFL);
        Notify(INVALID_SIGNAL_NUMBER);
        _worker.join();
        ::close(_pipe[0]);
        ::close(_pipe[1]);
    }

    void Notify(int signalNumber)
    {
        // In signal handler context: no allocs, no error handling
        _signalNumber = signalNumber;
        uint8_t buf{};
        auto ok = ::write(_pipe[1], &buf, sizeof(buf));
        (void)ok;
    }

private:

    void workerMain()
    {
        std::vector<uint8_t> buf(1);
        // Blocking read until Notify() was called
        auto ok = ::read(_pipe[0], buf.data(), buf.size());
        if (ok == -1)
        {
            throw std::runtime_error("SignalMonitor::workerMain: Failed to read from pipe: " + ErrorMessage());
        }
        if ((_signalNumber != INVALID_SIGNAL_NUMBER) && _handler)
        {
            _handler(_signalNumber);
        }
    }

    int _pipe[2];
    SignalHandler _handler;
    std::thread _worker;
    int _signalNumber{INVALID_SIGNAL_NUMBER};
};

static inline void setSignalAction(int sigNum, __sighandler_t action)
{
    // Check current signal handler action to see if it's set to SIGNAL IGNORE
    struct sigaction oldAction{};
    sigaction(sigNum, NULL, &oldAction);
    if (oldAction.sa_handler == SIG_IGN)
    {
        // A non-job-control shell wants us to ignore this kind of signal
        return;
    }
    // Set new signal handler action to what we want
    struct sigaction newAction{};
    newAction.sa_handler = action;
    auto ret = sigaction(sigNum, &newAction, NULL);
    if (ret == -1)
    {
        throw std::runtime_error("SignalMonitor: Failed to set handler for signal " + std::to_string(sigNum) + ": " + ErrorMessage());
    }
}

void systemHandler(const int sigNum)
{
    if (gSignalMonitor)
    {
        gSignalMonitor->Notify(sigNum);
    }
}

} // namespace
#endif

namespace SilKit {
namespace Util {

void RegisterSignalHandler(SignalHandler handler)
{
    gSignalMonitor.reset(new SignalMonitor(std::move(handler)));
}

} // namespace Util
} // namespace SilKit
