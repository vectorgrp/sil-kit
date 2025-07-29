// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <chrono>
#include <functional>

#include "fwd_decl.hpp"

#include "silkit/util/Span.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

//! \brief An incoming DataMessage of a DataPublisher containing raw data and timestamp
struct DataMessageEvent
{
    //! Send timestamp of the event
    std::chrono::nanoseconds timestamp;
    //! Data field containing the payload
    Util::Span<const uint8_t> data;
};

//! \brief Callback type for new data reception callbacks
using DataMessageHandler = std::function<void(SilKit::Services::PubSub::IDataSubscriber* subscriber,
                                              const DataMessageEvent& dataMessageEvent)>;

} // namespace PubSub
} // namespace Services
} // namespace SilKit
