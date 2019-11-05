// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "idl/EthernetTopicsPubSubTypes.h"
#include "idl/EthernetTopics.h"

#include "ib/sim/eth/EthDatatypes.hpp"

namespace ib {
namespace sim {
namespace eth {

inline auto to_idl(const EthTagControlInformation& msg) -> idl::EthTagControlInformation;
inline auto to_idl(const EthMessage& msg) -> idl::EthMessage;
inline auto to_idl(EthMessage&& msg) -> idl::EthMessage;
inline auto to_idl(const EthTransmitAcknowledge& msg) -> idl::EthTransmitAcknowledge;
inline auto to_idl(const EthStatus& msg) -> idl::EthStatus;
inline auto to_idl(const EthSetMode& msg) -> idl::EthSetMode;

namespace idl {
inline auto from_idl(EthTagControlInformation&& idl) -> ::ib::sim::eth::EthTagControlInformation;
inline auto from_idl(EthMessage&& idl) -> ::ib::sim::eth::EthMessage;
inline auto from_idl(EthTransmitAcknowledge&& idl) -> ::ib::sim::eth::EthTransmitAcknowledge;
inline auto from_idl(EthStatus&& idl) -> ::ib::sim::eth::EthStatus;
inline auto from_idl(EthSetMode&& idl) -> ::ib::sim::eth::EthSetMode;

}

// ================================================================================
//  Inline Implementations
// ================================================================================
 auto to_idl(const EthTagControlInformation& msg) -> idl::EthTagControlInformation
 {
     idl::EthTagControlInformation idl;

     idl.pcp(msg.pcp);
     idl.dei(msg.dei);
     idl.vid(msg.vid);

     return idl;
 }

 auto idl::from_idl(EthTagControlInformation&& idl) -> ::ib::sim::eth::EthTagControlInformation
 {
     ::ib::sim::eth::EthTagControlInformation msg;

     msg.pcp = idl.pcp();
     msg.dei = idl.dei();
     msg.vid = idl.vid();

     return msg;
 }

auto to_idl(const EthMessage& msg) -> idl::EthMessage
{
    idl::EthMessage idl;

    idl.transmitId(msg.transmitId);
    idl.timestampNs(msg.timestamp.count());
    idl.rawFrame(msg.ethFrame.RawFrame());

    return idl;
}

auto to_idl(EthMessage&& msg) -> idl::EthMessage
{
    idl::EthMessage idl;

    idl.transmitId(msg.transmitId);
    idl.timestampNs(msg.timestamp.count());
    idl.rawFrame(std::move(msg.ethFrame.RawFrame()));

    return idl;
}

auto idl::from_idl(EthMessage&& idl) -> ::ib::sim::eth::EthMessage
{
    ::ib::sim::eth::EthMessage msg;
    ::ib::sim::eth::EthFrame rawFrame(std::move(idl.rawFrame()));

    msg.transmitId = idl.transmitId();
    msg.timestamp = std::chrono::nanoseconds{idl.timestampNs()};
    msg.ethFrame = std::move(rawFrame);

    return msg;
}


auto to_idl(const EthTransmitAcknowledge& msg) -> idl::EthTransmitAcknowledge
{
    idl::EthTransmitAcknowledge idl;

    idl.transmitId(msg.transmitId);
    //idl.sourceMac(msg.sourceMac);
    idl.timestampNs(msg.timestamp.count());
    idl.status(static_cast<std::decay<decltype(idl.status())>::type>(msg.status));

    return idl;
}

auto idl::from_idl(EthTransmitAcknowledge&& idl) -> ::ib::sim::eth::EthTransmitAcknowledge
{
    ::ib::sim::eth::EthTransmitAcknowledge msg;

    msg.transmitId = idl.transmitId();
    //msg.sourceMac = idl.sourceMac();
    msg.timestamp = std::chrono::nanoseconds{idl.timestampNs()};
    msg.status = static_cast<decltype(msg.status)>(idl.status());

    return msg;
}


auto to_idl(const EthStatus& msg) -> idl::EthStatus
{
    idl::EthStatus idl;

    idl.timestampNs(msg.timestamp.count());
    idl.state(static_cast<std::decay<decltype(idl.state())>::type>(msg.state));
    idl.bitRate(msg.bitRate);

    return idl;
}

auto idl::from_idl(EthStatus&& idl) -> ::ib::sim::eth::EthStatus
{
    ::ib::sim::eth::EthStatus msg;

    msg.timestamp = std::chrono::nanoseconds{ idl.timestampNs() };
    msg.state = static_cast<decltype(msg.state)>(idl.state());
    msg.bitRate = idl.bitRate();

    return msg;
}

auto to_idl(const EthSetMode& msg) -> idl::EthSetMode
{
    idl::EthSetMode idl;

    idl.mode(static_cast<std::decay<decltype(idl.mode())>::type>(msg.mode));

    return idl;
}

auto idl::from_idl(EthSetMode&& idl) -> ::ib::sim::eth::EthSetMode
{
    ::ib::sim::eth::EthSetMode msg;

    msg.mode = static_cast<decltype(msg.mode)>(idl.mode());

    return msg;
}


} // namespace eth
} // namespace sim
} // namespace ib
