// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <tuple>

#include "IMessageReceiver.hpp"

namespace SilKit {
namespace Core {

template <typename... MsgT>
class IReceiver : public IMessageReceiver<MsgT>...
{
public:
    using SilKitReceiveMessagesTypes = std::tuple<MsgT...>;
    virtual ~IReceiver() = default;
};

} // namespace Core
} // namespace SilKit
