// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "NamedPipeLinux.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <sstream>
#include <cerrno>
#include <exception>
#include <sstream>

namespace ib {
namespace tracing {
namespace detail {

NamedPipeLinux::NamedPipeLinux(const std::string& name)
:_name(name)
{
    int err = ::mkfifo(_name.c_str(), 0644);

    if(err == -1)
    {
        std::stringstream ss;
        ss  << "Error creating pipe \"" << _name << "\""
            << ": errno: " << err
            << ": " << strerror(errno)
            ;

        throw std::runtime_error(ss.str());
    }
    _isOwner = true;
}

void NamedPipeLinux::Close()
{
    if(_isOwner)
    {
        _isOwner=false;
        int err = ::unlink(_name.c_str());
        if(err == -1)
        {
            std::stringstream msg;
            msg  << "NamedPipeLinux: Error deleting pipe \"" << _name << "\""
                << ": errno: " << err
                << ": " << strerror(errno)
                ;
            throw std::runtime_error{msg.str()};

        }
    }
}
NamedPipeLinux::~NamedPipeLinux()
{
    try {
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

            throw std::runtime_error(ss.str());
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


} //end namespace detail
} //end namespace tracing
} //end namespace ib
