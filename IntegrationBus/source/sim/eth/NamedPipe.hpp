// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <string>

namespace ib {
namespace sim {
namespace eth {

class NamedPipe
{
public:
    using Ptr = std::unique_ptr<NamedPipe>;

    // ----------------------------------------
    // Base Destructor
    virtual ~NamedPipe() {}

    // ----------------------------------------
    // Public interface method
    virtual bool Write(const char *buffer, size_t bufferSize) = 0;

    // ----------------------------------------
    // Factory method
    static auto make(const std::string& name) -> Ptr;
};

}
}
}