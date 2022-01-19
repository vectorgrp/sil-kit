// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/cfg/Config.hpp"

#include "DataMessageDatatypes.hpp"

namespace ib {
namespace sim {
namespace data {

class IDataSubscriber
{

public:
    virtual ~IDataSubscriber() = default;

    /*! \brief Get the config struct used to setup this IDataSubscriber
     *
     * The protocolType and definitionUri are as configured at the corresponding
     * \ref IDataPublisher.
     */
    virtual auto Config() const -> const cfg::DataPort& = 0;

    /*! \brief Register a callback for new data reception
     *
     * The callback is executed when new data is received from a
     * matching publisher.
     */
    virtual void SetReceiveMessageHandler(CallbackExchangeFormatT callback) = 0;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace data
} // namespace sim
} // namespace ib
