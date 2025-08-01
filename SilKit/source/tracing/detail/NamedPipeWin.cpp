// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/participant/exception.hpp"

#include "NamedPipeWin.hpp"

#include <iostream>
#include <sstream>
#include <windows.h>

namespace SilKit {
namespace Tracing {
namespace Detail {

static std::string GetPipeError()
{
    LPVOID lpMsgBuf;

    auto msgSize = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

    if (msgSize == 0)
    {
        return "FromMessageA failed!";
    }
    std::string rv(reinterpret_cast<char*>(lpMsgBuf));
    LocalFree(lpMsgBuf);
    return rv;
}

NamedPipeWin::NamedPipeWin(const std::string& name)
{
    std::stringstream ss;
    ss << "\\\\.\\pipe\\" << name;

    _pipeHandle = CreateNamedPipe(ss.str().c_str(), PIPE_ACCESS_OUTBOUND,
                                  //we use message buffering, in a blocking manner
                                  PIPE_TYPE_MESSAGE | PIPE_WAIT,
                                  //single instance only
                                  1,
                                  //allow for very large messages
                                  65536, 65536,
                                  //raise the default 50ms timeout, just in case
                                  300, NULL);
    if (!isValid())
    {
        throw SilKitError(GetPipeError());
    }
    _name = ss.str();
}

NamedPipeWin::~NamedPipeWin()
{
    try
    {
        Close();
    }
    catch (...)
    {
        // ignore error in destructor
    }
}
bool NamedPipeWin::Write(const char* buffer, size_t size)
{
    if (!_isConnected)
    {
        //explicitly synchronize here with our Pipe partner
        // this cannot be done in Open() as we would block inside of the Participant setup
        auto ok = ConnectNamedPipe(_pipeHandle, NULL);
        if (ok == 0 && (GetLastError() != ERROR_PIPE_CONNECTED))
        {
            auto msg = "NamedPipeWin: ConnecteNamedPipe failed: " + GetPipeError();
            throw SilKitError(msg);
        }
        _isConnected = true;
    }

    DWORD cbWritten = 0;
    if (size == 0)
        return false;
    if (isValid())
    {
        auto ok = WriteFile(_pipeHandle, buffer, static_cast<DWORD>(size), &cbWritten, NULL);
        if (!ok && GetLastError() == ERROR_NO_DATA)
        {
            Close();
        }
        else if (!ok || cbWritten != size)
        {
            std::stringstream msg;
            msg << "Failed to connect pipe '" << _name << "': " << GetPipeError();
            throw SilKitError(msg.str());
        }
    }
    return cbWritten == size;
}

void NamedPipeWin::Close()
{
    if (isValid())
    {
        BOOL isClosed = TRUE;
        isClosed &= FlushFileBuffers(_pipeHandle);
        isClosed &= DisconnectNamedPipe(_pipeHandle);
        isClosed &= CloseHandle(_pipeHandle);
        if (!isClosed)
        {
            std::stringstream msg;
            msg << "Failed to close pipe '" << _name << "': " << GetPipeError();
            throw SilKitError{msg.str()};
        }

        _pipeHandle = INVALID_HANDLE_VALUE;
    }
}

//public Factory
std::unique_ptr<NamedPipe> NamedPipe::Create(const std::string& name)
{
    return std::make_unique<NamedPipeWin>(name);
}

} //end namespace Detail
} //end namespace Tracing
} //end namespace SilKit
