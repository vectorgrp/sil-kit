// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>

#include "silkit/util/HandlerId.hpp"

namespace SilKit {
namespace Services {

/*! \brief Flag indicating the direction of a message
 */
enum class TransmitDirection : uint8_t
{
    // Undefined
    Undefined = 0,
    // Transmit
    TX = 1,
    // Receive
    RX = 2,
    // Send/Receive
    TXRX = 3,
};
using DirectionMask = uint8_t;

/*! \brief A label to control the matching behavior of DataPublishers, DataSubscribers, RpcServers, and RpcClients
* 
* Used in SilKit::Services::PubSub::PubSubSpec and SilKit::Services::Rpc::RpcSpec
*/
struct MatchingLabel
{
    /*! \brief The different kinds of matching rules for labels.
    */
    enum class Kind : uint32_t
    {
//! If this label is available, its value must match.
        Optional = 1,
//! This label must be available and its value must match.
        Mandatory = 2
    };

//! The label's key.
    std::string key;  
//! The label's key.
    std::string value;
//! The matching kind to apply for this label.
    Kind kind;        
};

using SilKit::Util::HandlerId;

} // namespace Services
} // namespace SilKit
