// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include "NamedPipe.hpp"
#include <fstream>
#include <string>

namespace SilKit {
namespace Tracing {
namespace Detail {

class NamedPipeLinux final : public NamedPipe
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
    void Close() override;

private:
    // ----------------------------------------
    // private members
    std::string _name;
    std::fstream _file;
    bool _isOwner{false};
    bool _isOpen{false};
};

} // namespace Detail
} // namespace Tracing
} // namespace SilKit
