// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <tuple>

namespace SilKit {
namespace Core {

template <typename... MsgT>
class ISender
{
public:
    using SilKitSendMessagesTypes = std::tuple<MsgT...>;
};

} // namespace Core
} // namespace SilKit
