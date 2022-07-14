// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <future>

#include "RpcDatatypes.hpp"

#include "silkit/util/Span.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

class IRpcClient
{
public:
    virtual ~IRpcClient() = default;

    /*! \brief Convenience method for a remote procedure call
     *
     * Creates a new std::vector with content copied from \p data. For highest efficiency,
     * use \ref Call(std::vector<uint8_t>) in combination with std::move.
     *
     * \param data A non-owning reference to an opaque block of raw data
     *
     * \return A call handle that can be used to identify this call in the CallReturnHandler
     */
    virtual auto Call(Util::Span<const uint8_t> data) -> IRpcCallHandle* = 0;

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
