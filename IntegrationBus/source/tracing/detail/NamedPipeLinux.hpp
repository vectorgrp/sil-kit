// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once
#include "NamedPipe.hpp"
#include <fstream>
#include <string>

class NamedPipeLinux: public NamedPipe
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    NamedPipeLinux(const std::string& name);
    ~NamedPipeLinux();
public:
    // ----------------------------------------
    // Public interface methods
    bool Write(const char* buffer, size_t bufferSize) override;
private:
    // ----------------------------------------
    // private members
    std::string _name;
    std::fstream _file;
    bool _isOwner{false};
private:
    // ----------------------------------------
    // private methods
    void Close();
};

