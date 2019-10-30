// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "NamedPipe.hpp"

#include <iostream>
#include <sstream>
#include <windows.h>

namespace ib {
namespace sim {
namespace eth {

static std::string GetPipeError()
{
    LPVOID lpMsgBuf;

    auto msgSize = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)& lpMsgBuf, 0, NULL);

    if (msgSize == 0)
    {
        return "FromMessageA failed!";
    }
    std::string rv(reinterpret_cast<char *>(lpMsgBuf));
    LocalFree(lpMsgBuf);
    return rv;
}


class NamedPipeWin : public NamedPipe
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    NamedPipeWin(const std::string& name)
    {
        std::stringstream ss;
        ss << "\\\\.\\pipe\\" << name;

        _pipeHandle = CreateNamedPipe(
            ss.str().c_str(),
            PIPE_ACCESS_OUTBOUND,
            PIPE_TYPE_MESSAGE | PIPE_WAIT,
            1, 65536, 65536,
            300,
            NULL);
        if (!isValid())
        {
            throw std::runtime_error(GetPipeError());
        }
        ConnectNamedPipe(_pipeHandle, NULL);
    }

    ~NamedPipeWin()
    {
        if (isValid())
        {
            BOOL ok = FALSE;
            ok = FlushFileBuffers(_pipeHandle);
            ok = DisconnectNamedPipe(_pipeHandle);
            ok = CloseHandle(_pipeHandle);
        }
    }

public:
    // ----------------------------------------
    // Public interface methods
    bool Write(const char *buffer, size_t size) override
    {
        DWORD cbWritten = 0;
        if (size == 0) return false;
        if (isValid())
        {
            auto ok = WriteFile(_pipeHandle, buffer, static_cast<DWORD>(size), &cbWritten, NULL);
            if (!ok || cbWritten != size)
            {
                throw std::runtime_error("NamedPipeImpl::Write returned error: " + GetPipeError());
            }
        }
        return cbWritten == size;
    }

private:
    // ----------------------------------------
    // private members
    HANDLE _pipeHandle{INVALID_HANDLE_VALUE};

private:
    // ----------------------------------------
    // private methods
    bool isValid() const { return _pipeHandle != INVALID_HANDLE_VALUE; }
};

std::unique_ptr<NamedPipe> NamedPipe::Create(const std::string& name)
{
    return std::make_unique<NamedPipeWin>(name);
}

}
}
}
