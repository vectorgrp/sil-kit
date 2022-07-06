// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IServiceEndpoint.hpp"

namespace SilKit {
namespace Core {

template<typename T>
class IMessageReceiver
{
public:
    virtual void ReceiveSilKitMessage(const SilKit::Core::IServiceEndpoint* from, const T& msg) = 0;
};

} // namespace Core
} // namespace SilKit
