// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include "NamedPipe.hpp"

#include <string>

#include <windows.h>


namespace SilKit {
namespace Tracing {
namespace Detail {

class NamedPipeWin : public NamedPipe
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    NamedPipeWin(const std::string& name);

    ~NamedPipeWin();

public:
    // ----------------------------------------
    // Public interface methods
    bool Write(const char* buffer, size_t size) override;
    void Close() override;

private:
    // ----------------------------------------
    // private members
    HANDLE _pipeHandle{INVALID_HANDLE_VALUE};

private:
    // ----------------------------------------
    // private methods
    bool isValid() const
    {
        return _pipeHandle != INVALID_HANDLE_VALUE;
    }

    bool _isConnected{false};
    std::string _name;
};

} //end namespace Detail
} //end namespace Tracing
} //end namespace SilKit
