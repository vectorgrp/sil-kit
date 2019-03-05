// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbMessageReceiver.hpp"
#include <tuple>

namespace ib {
namespace mw {

template<typename... MsgT>
class IIbReceiver : public IIbMessageReceiver<MsgT>...
{
public:
    using IbReceiveMessagesTypes = std::tuple<MsgT...>;
};

} // namespace mw
} // namespace ib
