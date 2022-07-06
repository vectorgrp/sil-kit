// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once
#include "NamedPipe.hpp"
#include <windows.h>

namespace SilKit {
namespace tracing {
namespace detail {

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
    bool isValid() const { return _pipeHandle != INVALID_HANDLE_VALUE; }

    bool _isConnected{false};
};

} //end namespace detail
} //end namespace tracing
} //end namespace SilKit
