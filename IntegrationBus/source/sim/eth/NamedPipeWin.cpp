// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "NamedPipeWin.hpp"

#include <iostream>
#include <sstream>
#include <windows.h>

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

namespace ib {
namespace sim {
namespace eth {

NamedPipeWin::NamedPipeWin(const std::string& name)
{
    std::stringstream ss;
    ss << "\\\\.\\pipe\\" << name;

    _pipeHandle = CreateNamedPipe(
        ss.str().c_str(),
        PIPE_ACCESS_OUTBOUND,
        //we use message buffering, in a blocking manner
        PIPE_TYPE_MESSAGE | PIPE_WAIT,
        //single instance only
        1,
        //allow for very large messages
        65536, 65536,
        //raise the default 50ms timeout, just in case
        300,
        NULL);
    if (!isValid())
    {
        throw std::runtime_error(GetPipeError());
    }
    ConnectNamedPipe(_pipeHandle, NULL);
}

NamedPipeWin::~NamedPipeWin()
{
    closeConnection();
}
bool NamedPipeWin::Write(const char* buffer, size_t size)
{
    DWORD cbWritten = 0;
    if (size == 0) return false;
    if (isValid())
    {
        auto ok = WriteFile(_pipeHandle, buffer, static_cast<DWORD>(size), &cbWritten, NULL);
        if (!ok && GetLastError() == ERROR_NO_DATA)
        {
            closeConnection();
        }
        else if (!ok || cbWritten != size)
        {
            throw std::runtime_error("NamedPipeWin::Write returned error: " + GetPipeError());
        }
    }
    return cbWritten == size;
}

void NamedPipeWin::closeConnection()
{
    if (isValid())
    {
        BOOL isClosed = TRUE;
        isClosed &= FlushFileBuffers(_pipeHandle);
        isClosed &= DisconnectNamedPipe(_pipeHandle);
        isClosed &= CloseHandle(_pipeHandle);
        if (!isClosed) std::cerr << "ERROR: Closing the PCAP pipe handle was not successful." << std::endl;

        _pipeHandle = INVALID_HANDLE_VALUE;
    }
}

std::unique_ptr<NamedPipe> NamedPipe::Create(const std::string& name)
{
    return std::make_unique<NamedPipeWin>(name);
}

}
}
}
