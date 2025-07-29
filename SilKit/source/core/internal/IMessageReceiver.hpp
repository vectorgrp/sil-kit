// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IServiceEndpoint.hpp"

namespace SilKit {
namespace Core {

template <typename T>
class IMessageReceiver
{
public:
    virtual void ReceiveMsg(const SilKit::Core::IServiceEndpoint* from, const T& msg) = 0;
};

} // namespace Core
} // namespace SilKit
