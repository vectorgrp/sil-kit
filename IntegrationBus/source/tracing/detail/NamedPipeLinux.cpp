// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "NamedPipeLinux.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <sstream>
#include <cerrno>
#include <exception>
#include <iostream>

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

    _file.open(_name, std::ios_base::out|std::ios_base::binary);
    if(!_file.good())
    {
        std::stringstream ss;
        ss << "fstream open \"" << _name << "\" failed!";
        Close();

        throw std::runtime_error(ss.str());
    }
}

void NamedPipeLinux::Close()
{
    if(_isOwner)
    {
        int err = ::unlink(_name.c_str());
        if(err == -1)
        {
            std::cerr  << "Error deleting pipe \"" << _name << "\""
                << ": errno: " << err
                << ": " << strerror(errno)
                ;

        }
    }
    _isOwner=false;
}
NamedPipeLinux::~NamedPipeLinux()
{
    Close();
}

bool NamedPipeLinux::Write(const char* buffer, size_t bufferSize)
{
    _file.write(buffer, bufferSize);
    _file.flush();
    return _file.good();
}

std::unique_ptr<NamedPipe> NamedPipe::Create(const std::string& name)
{
    return std::make_unique<NamedPipeLinux>(name);
}
