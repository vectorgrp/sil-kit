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

    /*! \brief Set the default handler for data reception
     *
     * The handler is executed when data is received from a matching publisher.
     * The default handler will not be invoked if a specific is available.
     */
    virtual void SetDefaultReceiveMessageHandler(DataHandlerT callback) = 0;

    /*! \brief Register a handler for data received by DataPublishers matching specific criteria
     * 
     * Specific data handlers can be used to distinguish publications on a single topic. If all of the labels provided
     * here appear in the labels of a DataPublisher, publications are redirected to the specific handler. A label key 
     * must match, an empty string in the value of a label given here is a wildcard. Likewise, the 
     * dataExchangeFormat must match the one given by a DataPublisher. An empty string in a field of the 
     * dataExchangeFormat provided here also is a wildcard.
     */
    virtual void RegisterSpecificDataHandler(const DataExchangeFormat& dataExchangeFormat,
                                             const std::map<std::string, std::string>& labels,
                                             DataHandlerT callback) = 0;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace data
} // namespace sim
} // namespace ib
