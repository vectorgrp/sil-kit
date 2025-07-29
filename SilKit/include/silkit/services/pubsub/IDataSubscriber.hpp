// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "PubSubDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

class IDataSubscriber
{
public:
    virtual ~IDataSubscriber() = default;

    /*! \brief Set the default handler for data reception
     *
     * The handler is executed when data is received from a matching publisher.
     * The default handler will not be invoked if a specific is available.
     */
    virtual void SetDataMessageHandler(DataMessageHandler callback) = 0;
};

} // namespace PubSub
} // namespace Services
} // namespace SilKit
