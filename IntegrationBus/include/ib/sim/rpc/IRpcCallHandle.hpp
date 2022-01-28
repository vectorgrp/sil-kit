// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

namespace ib {
namespace sim {
namespace rpc {

class IRpcCallHandle
{
public:

    virtual ~IRpcCallHandle() = default;

    virtual bool Valid() const = 0;

    virtual bool operator==(const IRpcCallHandle& other) const = 0;
    virtual bool operator!=(const IRpcCallHandle& other) const = 0;
};

} // namespace rpc
} // namespace sim
} // namespace ib
