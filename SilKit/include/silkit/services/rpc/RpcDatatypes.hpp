// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
    Success = SilKit_RpcCallStatus_Success,                       //!< Call was successful
    ServerNotReachable = SilKit_RpcCallStatus_ServerNotReachable, //!< No server matching the RpcSpec was found
    UndefinedError = SilKit_RpcCallStatus_UndefinedError,         //!< An unidentified error occured
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

//! \brief The callback function invoked when a call is to be handled by the IRpcServer.
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

//! \brief The callback function invoked when a call result is delivered to the IRpcClient.
using RpcCallResultHandler = std::function<void(IRpcClient* client, const RpcCallResultEvent& event)>;

} // namespace Rpc
} // namespace Services
} // namespace SilKit
