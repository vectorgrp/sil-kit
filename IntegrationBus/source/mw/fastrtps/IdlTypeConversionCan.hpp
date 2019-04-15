// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "idl/CanTopicsPubSubTypes.h"
#include "idl/CanTopics.h"

#include "ib/sim/can/CanDatatypes.hpp"

namespace ib {
namespace sim {
namespace can {

inline auto to_idl(const CanMessage::CanReceiveFlags& flags) -> idl::CanMessageFlags;
inline auto to_idl(const CanMessage& msg) -> idl::CanMessage;
inline auto to_idl(CanMessage&& msg) -> idl::CanMessage;
inline auto to_idl(const CanTransmitAcknowledge& msg) -> idl::CanTransmitAcknowledge;
inline auto to_idl(const CanControllerStatus& msg)->idl::CanControllerStatus;
inline auto to_idl(const CanConfigureBaudrate& msg)->idl::CanConfigureBaudrate;
inline auto to_idl(const CanSetControllerMode& msg)->idl::CanSetControllerMode;

namespace idl {
inline auto from_idl(CanMessageFlags&& idl) -> ::ib::sim::can::CanMessage::CanReceiveFlags;
inline auto from_idl(CanMessage&& idl) -> ::ib::sim::can::CanMessage;
inline auto from_idl(CanTransmitAcknowledge&& idl) -> ::ib::sim::can::CanTransmitAcknowledge;
inline auto from_idl(CanControllerStatus&& idl) -> ::ib::sim::can::CanControllerStatus;
inline auto from_idl(CanConfigureBaudrate&& idl) -> ::ib::sim::can::CanConfigureBaudrate;
inline auto from_idl(CanSetControllerMode&& idl) -> ::ib::sim::can::CanSetControllerMode;
}

// ================================================================================
//  Inline Implementations
// ================================================================================
auto to_idl(const CanMessage::CanReceiveFlags& canReceiveFlags) -> idl::CanMessageFlags
{
    idl::CanMessageFlags idl;

    idl.ide() = (canReceiveFlags.ide == 1);
    idl.rtr() = (canReceiveFlags.rtr == 1);
    idl.fdf() = (canReceiveFlags.fdf == 1);
    idl.brs() = (canReceiveFlags.brs == 1);
    idl.esi() = (canReceiveFlags.esi == 1);

    return idl;
}

auto idl::from_idl(CanMessageFlags&& idl) -> ::ib::sim::can::CanMessage::CanReceiveFlags
{
    ::ib::sim::can::CanMessage::CanReceiveFlags msg;

    msg.ide = (idl.ide() ? 1 : 0);
    msg.rtr = (idl.rtr() ? 1 : 0);
    msg.fdf = (idl.fdf() ? 1 : 0);
    msg.brs = (idl.brs() ? 1 : 0);
    msg.esi = (idl.esi() ? 1 : 0);

    return msg;
}

auto to_idl(const CanMessage& msg) -> idl::CanMessage
{
    idl::CanMessage idl;

    idl.transmitId(msg.transmitId);
    idl.timestampNs(msg.timestamp.count());

    idl.canId(msg.canId);
    idl.flags() = to_idl(msg.flags);
    idl.dlc(msg.dlc);
    idl.dataField(msg.dataField);

    return idl;
}

auto to_idl(CanMessage&& msg) -> idl::CanMessage
{
    idl::CanMessage idl;

    idl.transmitId(msg.transmitId);
    idl.timestampNs(msg.timestamp.count());

    idl.canId(msg.canId);
    idl.flags() = to_idl(msg.flags);
    idl.dlc(msg.dlc);
    idl.dataField(std::move(msg.dataField));

    return idl;
}

auto idl::from_idl(CanMessage&& idl) -> ::ib::sim::can::CanMessage
{
    ::ib::sim::can::CanMessage msg;

    msg.transmitId = idl.transmitId();
    msg.timestamp = std::chrono::nanoseconds(idl.timestampNs());

    msg.canId = idl.canId();
    msg.flags = from_idl(std::move(idl.flags()));
    msg.dlc = idl.dlc();
    msg.dataField = std::move(idl.dataField());

    return msg;
}


auto to_idl(const CanTransmitAcknowledge& msg) -> idl::CanTransmitAcknowledge
{
    idl::CanTransmitAcknowledge idl;

    idl.transmitId(msg.transmitId);
    idl.timestampNS(msg.timestamp.count());
    idl.txStatus(static_cast<std::underlying_type_t<decltype(msg.status)>>(msg.status));

    return idl;
}

auto idl::from_idl(CanTransmitAcknowledge&& idl) -> ::ib::sim::can::CanTransmitAcknowledge
{
    ::ib::sim::can::CanTransmitAcknowledge msg;

    msg.transmitId = idl.transmitId();
    msg.timestamp = std::chrono::nanoseconds(idl.timestampNS());
    msg.status = static_cast<decltype(msg.status)>(idl.txStatus());

    return msg;
}


auto to_idl(const CanControllerStatus& msg) -> idl::CanControllerStatus
{
    idl::CanControllerStatus idl;

    idl.timestampNs(msg.timestamp.count());
    idl.controllerState(static_cast<std::underlying_type_t<decltype(msg.controllerState)>>(msg.controllerState));
    idl.errorState(static_cast<std::underlying_type_t<decltype(msg.errorState)>>(msg.errorState));

    return idl;
}

auto idl::from_idl(CanControllerStatus&& idl) -> ::ib::sim::can::CanControllerStatus
{
    ::ib::sim::can::CanControllerStatus msg;

    msg.timestamp = std::chrono::nanoseconds(idl.timestampNs());
    msg.controllerState = static_cast<decltype(msg.controllerState)>(idl.controllerState());
    msg.errorState = static_cast<decltype(msg.errorState)>(idl.errorState());

    return msg;
}


auto to_idl(const CanConfigureBaudrate& msg) -> idl::CanConfigureBaudrate
{
    idl::CanConfigureBaudrate idl;

    idl.baudRate(msg.baudRate);
    idl.fdBaudRate(msg.fdBaudRate);

    return idl;
}

auto idl::from_idl(CanConfigureBaudrate&& idl) -> ::ib::sim::can::CanConfigureBaudrate
{
    ::ib::sim::can::CanConfigureBaudrate msg;

    msg.baudRate = idl.baudRate();
    msg.fdBaudRate = idl.fdBaudRate();

    return msg;
}


auto to_idl(const CanSetControllerMode& msg) -> idl::CanSetControllerMode
{
    idl::CanSetControllerMode idl;

    idl.resetErrorHandling(msg.flags.resetErrorHandling == 1);
    idl.cancelTransmitRequests(msg.flags.cancelTransmitRequests == 1);
    idl.mode(static_cast<std::underlying_type_t<decltype(msg.mode)>>(msg.mode));

    return idl;
}

auto idl::from_idl(CanSetControllerMode&& idl) -> ::ib::sim::can::CanSetControllerMode
{
    ::ib::sim::can::CanSetControllerMode msg;

    msg.flags.resetErrorHandling = (idl.resetErrorHandling() ? 1 : 0);
    msg.flags.cancelTransmitRequests = (idl.cancelTransmitRequests() ? 1 : 0);
    msg.mode = static_cast<decltype(msg.mode)>(idl.mode());

    return msg;
}


} // namespace can
} // namespace sim
} // namespace ib
