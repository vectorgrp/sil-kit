// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>

#include "ib/cfg/Config.hpp"

#include "RpcDatatypes.hpp"

namespace ib {
namespace sim {
namespace rpc {


class IRpcServer
{

  public:
    virtual ~IRpcServer() = default;

    /*! \brief Get the config struct used to setup this IRpcServer
     */
    virtual auto Config() const -> const cfg::RpcPort& = 0;

    /*! \brief Answers an already received call from remote with arbitrary data
    * 
    * Using the call handle obtained in the Rpc handler, the result is send back to the calling RpcClient.
    * This can happen directly in the Rpc handler or at a later point in time.
    *
    * \param callHandle A unique identifier of this call
    * \param resultData The byte vector to be returned to the RpcClient
    */
    virtual void SubmitResult(IRpcCallHandle* callHandle, std::vector<uint8_t> resultData) = 0;

    /*! \brief Overwrite the Rpc handler of this RpcClient
     *
     * The signature of the CallProcessor is
     * (IRpcServer* server, IRpcCallHandle* callHandle, const std::vector<uint8_t>& argumentData)
     *
     * \param handler A std::function with the above signature
     */
    virtual void SetRpcHandler(CallProcessor handler) = 0;

};

} // namespace rpc
} // namespace sim
} // namespace ib
