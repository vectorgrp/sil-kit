// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <future>

#include "ib/cfg/Config.hpp"
#include "RpcDatatypes.hpp"

namespace ib {
namespace sim {
namespace rpc {


class IRpcClient
{
public:


    virtual ~IRpcClient() = default;

    //! \brief Get the config struct used to setup this IRpcClient
    virtual auto Config() const -> const cfg::RpcPort& = 0;

    /*! \brief Detach a call to one or multiple corresponding Rpc servers
    * 
    * This will trigger the remote call on matching Rpc servers and the subsequent answer in 
    * the CallReturnHandler of this client. If there is no corresponding server available,
    * the CallReturnHandler is triggered immediately with CallStatus::ServerNotReachable and the
    * returned call handle here is invalid.
    * 
    * \param argumentData The data that should be transmitted to the Rpc server for this call
    * \return A call handle that can be used to identify this call in the CallReturnHandler
    */
    virtual IRpcCallHandle* Call(std::vector<uint8_t> argumentData) = 0;
    
    /*! \brief Convenience method for a remote procedure call
     *
     * Creates a new std::vector with content copied from \p data. For highest efficiency,
     * use \ref Call(std::vector<uint8_t>) in combination with std::move.
     *
     * \param data C-style pointer to an opaque block of data
     * \param size Size of the data block to be copied
     */
    virtual IRpcCallHandle* Call(const uint8_t* data, std::size_t size) = 0;

    /*! \brief Overwrite the call return handler of this RpcClient
     * 
     * The signature of the CallReturnHandler is 
     * (IRpcClient* client, IRpcCallHandle* callHandle, 
     * const CallStatus callStatus, const std::vector<uint8_t>& returnData)
     *
     * \param handler A std::function with the above signature
     */
    virtual void SetCallReturnHandler(CallReturnHandler handler) = 0;
    
};

} // namespace rpc
} // namespace sim
} // namespace ib
