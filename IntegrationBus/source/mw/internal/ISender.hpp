// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <tuple>

namespace SilKit {
namespace Core {

template<typename... MsgT>
class ISender
{
public:
    using SilKitSendMessagesTypes = std::tuple<MsgT...>;
};

} // namespace Core
} // namespace SilKit
