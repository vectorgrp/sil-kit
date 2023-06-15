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

#include <functional>
#include <chrono>

#include <cstdint>

#include "fwd_decl.hpp"

#include "silkit/util/Span.hpp"

#include "silkit/capi/Rpc.h"

namespace SilKit {
namespace Services {
namespace Rpc {

//! \brief The status of a RpcCallResultEvent. Informs whether a call was successful.
enum class RpcCallStatus : SilKit_RpcCallStatus
{
    Success = SilKit_RpcCallStatus_Success, //!< Call was successful
    ServerNotReachable = SilKit_RpcCallStatus_ServerNotReachable, //!< No server matching the RpcSpec was found
    UndefinedError = SilKit_RpcCallStatus_UndefinedError, //!< An unidentified error occured
    /*! \brief The Call lead to an internal RpcServer error.
     * This might happen if no CallHandler was specified for the RpcServer.
     */
    InternalServerError = SilKit_RpcCallStatus_InternalServerError, 
    /*! \brief The Call did run into a timeout and was canceled.
     * This might happen if a corresponding server crashed, ran into an error or took too long to answer the call
     */
    Timeout = SilKit_RpcCallStatus_Timeout
};

//! \brief An incoming rpc call from a RpcClient with call data and timestamp
struct RpcCallEvent
{
    //! Send timestamp of the event
    std::chrono::nanoseconds timestamp;
    //! Handle of the rpc call by which the call can be identified and its result can be submitted
    IRpcCallHandle* callHandle;
    //! Data of the rpc call as provided by the client on call
    Util::Span<const uint8_t> argumentData;
};

using RpcCallHandler = std::function<void(IRpcServer* server, const RpcCallEvent& event)>;

//! \brief An incoming rpc call result of a RpcServer containing result data and timestamp
struct RpcCallResultEvent
{
    //! Send timestamp of the event
    std::chrono::nanoseconds timestamp;
    //! The user context pointer as it was provided when the call was triggered
    void* userContext;
    //! The status of the call, resultData is only valid if callStats == RpcCallStatus::Success
    RpcCallStatus callStatus;
    //! Data of the rpc call result as provided by the server
    Util::Span<const uint8_t> resultData;
};

using RpcCallResultHandler = std::function<void(IRpcClient* client, const RpcCallResultEvent& event)>;

} // namespace Rpc
} // namespace Services
} // namespace SilKit
