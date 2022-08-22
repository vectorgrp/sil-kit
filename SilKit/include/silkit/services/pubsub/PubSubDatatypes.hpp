/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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
using DataMessageHandler =
    std::function<void(SilKit::Services::PubSub::IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent)>;

} // namespace PubSub
} // namespace Services
} // namespace SilKit

