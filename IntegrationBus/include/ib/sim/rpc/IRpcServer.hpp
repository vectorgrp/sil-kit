// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>

#include "RpcDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

class IRpcServer
{
public:
    virtual ~IRpcServer() = default;

    /*! \brief Answers an already received call from remote with arbitrary data
    *
    * Using the call handle obtained in the call handler, the result is send back to the calling client.
    * This can happen directly in the call handler or at a later point in time.
    *
    * \param callHandle A unique identifier of this call
    * \param resultData The byte vector to be returned to the client
    */
    virtual void SubmitResult(IRpcCallHandle* callHandle, std::vector<uint8_t> resultData) = 0;

    /*! \brief Overwrite the call handler of this server
     *
     * The signature of the call handler is void(IRpcServer* server, RpcCallEvent event).
     *
     * \param handler A std::function with the above signature
     */
    virtual void SetCallHandler(RpcCallHandler handler) = 0;
};

} // namespace Rpc
} // namespace Services
} // namespace SilKit
