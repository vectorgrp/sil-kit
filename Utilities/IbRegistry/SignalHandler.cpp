// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "SignalHandler.hpp"
#include <thread>
#include <stdexcept>
#include <vector>

//forward
namespace {
class SignalMonitor;
}
// global signal handler
static std::unique_ptr<SignalMonitor> gSignalMonitor;

////////////////////////////////////////////
// Inline Platform Specific Implementations
////////////////////////////////////////////
#if WIN32
#include <Windows.h>

namespace {
using namespace ib::registry;

//forward
BOOL WINAPI systemHandler(DWORD ctrlType);

class SignalMonitor
{
public:
    SignalMonitor(SignalHandlerT handler)
    {
        _handler = std::move(handler);

        auto ok = CreatePipe(&_readEnd, &_writeEnd, nullptr, 0);
        if (!ok)
            throw std::runtime_error("CreatePipe failed. Cannot create Signal Handler!");
        DWORD nowait = PIPE_NOWAIT;
        ok = SetNamedPipeHandleState(_writeEnd, &nowait, nullptr, nullptr);
        if (!ok)
            throw std::runtime_error("Set pipe read end to nonblocking failed. Cannot create Signal Handler!");
        SetConsoleCtrlHandler(systemHandler, true);
        _worker = std::thread{std::bind(&SignalMonitor::workerMain, this)};
    }
    ~SignalMonitor()
    {
        SetConsoleCtrlHandler(systemHandler, false);
        Notify(-1);
        _worker.join();
        CloseHandle(_writeEnd);
        CloseHandle(_readEnd);
    }
    void Notify(int signalNumber)
    {
        // no allocs, no error handling
        _signalNumber = signalNumber;
        uint8_t buf{};
        auto ok = WriteFile(_writeEnd, &buf, sizeof(buf), nullptr, nullptr);
        (void)ok;
    }
private:

    void workerMain()
    {
        std::vector<uint8_t> buf(1);
        //blocking read until Notify() was called
        auto ok = ReadFile(_readEnd, buf.data(), buf.size(), nullptr, nullptr);
        if (!ok)
            throw std::runtime_error("SignalMonitor::workerMain: cannot read from pipe.");

        if (_handler)
        {
            _handler(_signalNumber);
        }
    }
    HANDLE _readEnd{INVALID_HANDLE_VALUE}, _writeEnd{INVALID_HANDLE_VALUE};
    SignalHandlerT _handler;
    std::thread _worker;
    int _signalNumber{-1};
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

using namespace ib::registry;

//forward
void systemHandler(int sigNum);

auto ErrorMessage()
{
    return std::string{strerror(errno)};
}

class SignalMonitor
{
public:
    SignalMonitor(SignalHandlerT handler)
    {
        _handler = std::move(handler);

        auto ok = ::pipe(_pipe);
        if (ok == -1)
            throw std::runtime_error("SignalMonitor: pipe() failed! " + ErrorMessage());
        ok = fcntl(_pipe[1], F_SETFL, O_NONBLOCK);
        if (ok == -1)
            throw std::runtime_error("SignalMonitor: cannot set write end of pipe to non blocking: " + ErrorMessage());

        signal(SIGINT, &systemHandler);
        _worker = std::thread{std::bind(&SignalMonitor::workerMain, this)};
    }
    ~SignalMonitor()
    {
        signal(SIGINT, SIG_DFL);
        Notify(-1);
        _worker.join();
        ::close(_pipe[0]);
        ::close(_pipe[1]);
    }
    
    void Notify(int signalNumber)
    {
        //in signal handler context: no allocs, no error handling
        _signalNumber = signalNumber;
        uint8_t buf{};
        auto ok = ::write(_pipe[1], &buf, sizeof(buf));
        (void)ok;
    }
private:

    void workerMain()
    {
        std::vector<uint8_t> buf(1);
        //blocking read until Notify() was called
        auto ok = ::read(_pipe[0], buf.data(), buf.size());
        if (ok == -1)
            throw std::runtime_error("SignalMonitor::workerMain: cannot read from pipe: " + ErrorMessage());

        if (_handler)
        {
            _handler(_signalNumber);
        }
    }

    int _pipe[2];
    SignalHandlerT _handler;
    std::thread _worker;
    int _signalNumber{-1};
};

void systemHandler(int sigNum)
{
    if (gSignalMonitor)
    {
        gSignalMonitor->Notify(sigNum);
    }
}

} //end anonymous namespace
#endif

namespace ib { namespace registry {

void RegisterSignalHandler(SignalHandlerT handler)
{
    gSignalMonitor.reset(new SignalMonitor(std::move(handler)));
}

}
}
