// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <tuple>

#include "IMessageReceiver.hpp"

namespace SilKit {
namespace Core {

template<typename... MsgT>
class IReceiver : public IMessageReceiver<MsgT>...
{
public:
    using SilKitReceiveMessagesTypes = std::tuple<MsgT...>;
    virtual ~IReceiver() = default;
};

} // namespace Core
} // namespace SilKit
