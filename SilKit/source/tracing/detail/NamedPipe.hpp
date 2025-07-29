// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

namespace SilKit {
namespace Tracing {
namespace Detail {

class NamedPipe
{
public:
    using Ptr = std::unique_ptr<NamedPipe>;

    // ----------------------------------------
    // Base Destructor
    virtual ~NamedPipe() {}

    // ----------------------------------------
    // Public interface method
    virtual bool Write(const char* buffer, size_t bufferSize) = 0;

    // ----------------------------------------
    // Close the pipe, throw exception if closing failed
    virtual void Close() = 0;

    // ----------------------------------------
    // Factory method
    static auto Create(const std::string& name) -> Ptr;
};

} // namespace Detail
} // namespace Tracing
} // namespace SilKit
