// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>

#include "ib/cfg/Config.hpp"

#include "GenericMessageDatatypes.hpp"

namespace ib {
namespace sim {
namespace generic {

class IGenericSubscriber
{
public:
    //! \brief Callback type for new data reception callbacks
    using CallbackT = std::function<void(IGenericSubscriber* subscriber, const std::vector<uint8_t>& data)>;

public:
    virtual ~IGenericSubscriber() = default;

    //! \brief Get the config struct used to setup this IGenericSubscriber
    virtual auto Config() const -> const cfg::GenericPort& = 0;

    /*! \brief Register a callback for new data reception
     *
     * The callback is executed when new data is received from a
     * matching publisher.
     */
    virtual void SetReceiveMessageHandler(CallbackT callback) = 0;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace generic
} // namespace sim
} // namespace ib
