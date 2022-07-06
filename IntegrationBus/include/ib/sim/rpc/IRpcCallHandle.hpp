// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

namespace SilKit {
namespace Services {
namespace Rpc {

class IRpcCallHandle
{
public:

    virtual ~IRpcCallHandle() = default;

    virtual bool operator==(const IRpcCallHandle& other) const = 0;
    virtual bool operator!=(const IRpcCallHandle& other) const = 0;
};

} // namespace Rpc
} // namespace Services
} // namespace SilKit
