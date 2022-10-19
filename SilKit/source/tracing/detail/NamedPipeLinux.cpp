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

#include "silkit/participant/exception.hpp"

#include "NamedPipeLinux.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <sstream>
#include <cerrno>
#include <exception>
#include <sstream>

namespace SilKit {
namespace Tracing {
namespace Detail {

NamedPipeLinux::NamedPipeLinux(const std::string& name)
    : _name(name)
{
    int err = ::mkfifo(_name.c_str(), 0644);

    if (err == -1)
    {
        std::stringstream ss;
        ss << "Error creating pipe \"" << _name << "\""
           << ": errno: " << err << ": " << strerror(errno);

        throw SilKitError(ss.str());
    }
    _isOwner = true;
}

void NamedPipeLinux::Close()
{
    if (_isOwner)
    {
        _isOwner = false;
        int err = ::unlink(_name.c_str());
        if (err == -1)
        {
            std::stringstream msg;
            msg << "Failed to delete pipe '" << _name << "':" << strerror(errno) << " (errno " << err << ")";
            throw SilKitError{msg.str()};
        }
    }
}

NamedPipeLinux::~NamedPipeLinux()
{
    try
    {
        Close();
    }
    catch (...)
    {
        //do not throw in destructor
    }
}

bool NamedPipeLinux::Write(const char* buffer, size_t bufferSize)
{
    if (!_isOpen)
    {
        // open on FIFO in WRITE mode will block us.
        _file.open(_name, std::ios_base::out | std::ios_base::binary);
        if (!_file.good())
        {
            std::stringstream ss;
            ss << "fstream open \"" << _name << "\" failed!";
            Close();

            throw SilKitError(ss.str());
        }
        _isOpen = true;
    }

    _file.write(buffer, bufferSize);
    _file.flush();
    return _file.good();
}

// public Factory
std::unique_ptr<NamedPipe> NamedPipe::Create(const std::string& name)
{
    return std::make_unique<NamedPipeLinux>(name);
}

} // namespace Detail
} // namespace Tracing
} // namespace SilKit
