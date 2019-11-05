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
            ss.str().c_str(),              /* The unique pipe name. Must have form \.\pipe<i>pipename. The pipename: is case-insensitive
                                              and no backslashes are allowed. The pipename can be up to 256 characters long. */
            PIPE_ACCESS_OUTBOUND,          /* Open mode: PIPE_ACCESS_OUTBOUND -> The flow of data in the pipe goes from server to client only. */
            PIPE_TYPE_MESSAGE | PIPE_WAIT, /* Pipe mode: PIPE_TYPE_MESSAGE -> Data is written as a stream of messages.
                                                         PIPE_WAIT -> Blocking mode is enabled. WriteFile and ConnectNamedPipe are blocking the process */
            1,                             /* Max number of instances that can be created for this pipe */
            65536, 65536,                  /* Number of bytes to reserve for input and output buffer */
            300,                           /* Timeout for the NamedPipe. A value of zero will result in a default timeout of 50 ms */
            NULL);                         /* Pointer to security attributes. NULL -> default security descriptor */
        if (!isValid())
        {
            throw std::runtime_error(GetPipeError());
        }
        ConnectNamedPipe(_pipeHandle, NULL);
    }

    ~NamedPipeWin()
    {
        closeConnection();
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

private:
    // ----------------------------------------
    // private members
    HANDLE _pipeHandle{INVALID_HANDLE_VALUE};

private:
    // ----------------------------------------
    // private methods
    bool isValid() const { return _pipeHandle != INVALID_HANDLE_VALUE; }
    void closeConnection()
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
};

std::unique_ptr<NamedPipe> NamedPipe::Create(const std::string& name)
{
    return std::make_unique<NamedPipeWin>(name);
}

}
}
}
