// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <tuple>

namespace ib {
namespace mw {

template<typename... MsgT>
class IIbSender
{
public:
    using IbSendMessagesTypes = std::tuple<MsgT...>;
};

} // namespace mw
} // namespace ib
