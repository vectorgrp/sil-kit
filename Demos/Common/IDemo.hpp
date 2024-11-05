#include "DemoDatatypes.hpp"

#pragma once

namespace SilKitDemo {

struct IDemo
{
    virtual ~IDemo() = default;

    virtual void CreateControllers(Context &context) = 0;

    virtual void InitControllers(SilKitDemo::Context &context) = 0;

    virtual void DoWork(Context &context) = 0;

    virtual void Teardown(Context &context) = 0;
};

} // namespace SilKitDemo