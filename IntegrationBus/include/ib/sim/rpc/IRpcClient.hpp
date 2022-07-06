// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <future>

#include "RpcDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

class IRpcClient
{
public:
    virtual ~IRpcClient() = default;

    /*! \brief Dispatch a call to one or multiple corresponding RPC servers
    *
    * This will trigger the remote call on matching RPC servers and the subsequent answer in
    * the CallReturnHandler of this client. If there is no corresponding server available,
    * the CallReturnHandler is triggered immediately with RpcCallStatus::ServerNotReachable and the
    * returned call handle here is invalid.
    *
    * \param argumentData The data that should be transmitted to the RPC server for this call
    * \return A call handle that can be used to identify this call in the CallReturnHandler
    */
    virtual auto Call(std::vector<uint8_t> argumentData) -> IRpcCallHandle* = 0;

    /*! \brief Convenience method for a remote procedure call
     *
     * Creates a new std::vector with content copied from \p data. For highest efficiency,
     * use \ref Call(std::vector<uint8_t>) in combination with std::move.
     *
     * \param data C-style pointer to an opaque block of data
     * \param size Size of the data block to be copied
     */
    virtual auto Call(const uint8_t* data, std::size_t size) -> IRpcCallHandle* = 0;

    /*! \brief Overwrite the call return handler of this client
     *
     * The signature of the handler is void(IRpcClient* client, RpcCallResultEvent event).
     *
     * \param handler A std::function with the above signature
     */
    virtual void SetCallResultHandler(RpcCallResultHandler handler) = 0;
};

} // namespace Rpc
} // namespace Services
} // namespace SilKit
