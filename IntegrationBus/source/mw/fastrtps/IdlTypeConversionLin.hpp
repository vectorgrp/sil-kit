// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/lin/LinDatatypes.hpp"

#include "idl/LinTopics.h"
#include "idl/LinTopicsPubSubTypes.h"


namespace ib {
namespace sim {
namespace lin {

inline auto to_idl(ChecksumModel msg) -> idl::ChecksumModel;
inline auto to_idl(const Frame& msg) -> idl::Frame;
inline auto to_idl(Frame&& msg) -> idl::Frame;
inline auto to_idl(FrameResponseType msg) -> idl::FrameResponseType;
inline auto to_idl(FrameResponseMode msg) -> idl::FrameResponseMode;
inline auto to_idl(const FrameResponse& msg) -> idl::FrameResponse;
inline auto to_idl(FrameStatus msg) -> idl::FrameStatus;
inline auto to_idl(ControllerMode msg) -> idl::ControllerMode;
inline auto to_idl(const ControllerConfig& msg) -> idl::ControllerConfig;
inline auto to_idl(ControllerStatus msg) -> idl::ControllerStatus;
inline auto to_idl(const Transmission& msg) -> idl::Transmission;
inline auto to_idl(SendFrameRequest&& msg) -> idl::SendFrameRequest;
inline auto to_idl(const SendFrameRequest& msg) -> idl::SendFrameRequest;
inline auto to_idl(SendFrameHeaderRequest&& msg)->idl::SendFrameHeaderRequest;
inline auto to_idl(const SendFrameHeaderRequest& msg) -> idl::SendFrameHeaderRequest;
inline auto to_idl(Transmission&& msg)->idl::Transmission;
inline auto to_idl(const Transmission& msg) -> idl::Transmission;
inline auto to_idl(Transmission&& msg)->idl::Transmission;
inline auto to_idl(const FrameResponseUpdate& msg) -> idl::FrameResponseUpdate;
inline auto to_idl(FrameResponseUpdate&& msg)->idl::FrameResponseUpdate;
inline auto to_idl(const ControllerStatusUpdate& msg) -> idl::ControllerStatusUpdate;
inline auto to_idl(ControllerStatusUpdate&& msg)->idl::ControllerStatusUpdate;
inline auto to_idl(const WakeupPulse& msg) -> idl::WakeupPulse;
inline auto to_idl(WakeupPulse&& msg)->idl::WakeupPulse;

namespace idl {
inline auto from_idl(ChecksumModel msg) -> lin::ChecksumModel;
inline auto from_idl(Frame&& msg) -> lin::Frame;
inline auto from_idl(FrameResponseType msg) -> lin::FrameResponseType;
inline auto from_idl(FrameResponseMode msg) -> lin::FrameResponseMode;
inline auto from_idl(FrameResponse&& msg) -> lin::FrameResponse;
inline auto from_idl(FrameStatus msg) -> lin::FrameStatus;
inline auto from_idl(ControllerMode msg) -> lin::ControllerMode;
inline auto from_idl(ControllerConfig&& msg) -> lin::ControllerConfig;
inline auto from_idl(ControllerStatus msg) -> lin::ControllerStatus;
inline auto from_idl(SendFrameRequest&& msg) -> lin::SendFrameRequest;
inline auto from_idl(SendFrameHeaderRequest&& msg) -> lin::SendFrameHeaderRequest;
inline auto from_idl(Transmission&& msg) -> lin::Transmission;
inline auto from_idl(FrameResponseUpdate&& msg) -> lin::FrameResponseUpdate;
inline auto from_idl(ControllerStatusUpdate&& msg) -> lin::ControllerStatusUpdate;
inline auto from_idl(WakeupPulse&& msg) -> lin::WakeupPulse;
} // namespace idl

// ================================================================================
//  Inline Implementations
// ================================================================================
auto to_idl(ChecksumModel msg) -> idl::ChecksumModel
{
    switch (msg)
    {
    case ChecksumModel::Undefined:
        return idl::Undefined;
    case ChecksumModel::Enhanced:
        return idl::Enhanced;
    case ChecksumModel::Classic:
        return idl::Classic;
    }
    throw std::runtime_error("conversion errror: Unknown lin::ChecksumModel");
}
auto idl::from_idl(idl::ChecksumModel idl) -> lin::ChecksumModel
{
    switch (idl)
    {
    case idl::Undefined:
        return lin::ChecksumModel::Undefined;
    case idl::Enhanced:
        return lin::ChecksumModel::Enhanced;
    case idl::Classic:
        return lin::ChecksumModel::Classic;
    }
    throw std::runtime_error("conversion errror: Unknown lin::ChecksumModel");
}

auto to_idl(const Frame& msg) -> idl::Frame
{
    idl::Frame idl;

    idl.id(msg.id);
    idl.checksumModel(to_idl(msg.checksumModel));
    idl.dataLength(msg.dataLength);
    idl.data(msg.data);

    return idl;
}
auto to_idl(Frame&& msg) -> idl::Frame
{
    return to_idl(msg);
}
auto idl::from_idl(idl::Frame&& idl) -> lin::Frame
{
    lin::Frame msg;

    msg.id = idl.id();
    msg.checksumModel = from_idl(idl.checksumModel());
    msg.dataLength = idl.dataLength();
    msg.data = idl.data();

    return msg;
}

auto to_idl(FrameResponseType msg) -> idl::FrameResponseType
{
    switch (msg)
    {
    case FrameResponseType::MasterResponse:
        return idl::MasterResponse;
    case FrameResponseType::SlaveResponse:
        return idl::SlaveResponse;
    case FrameResponseType::SlaveToSlave:
        return idl::SlaveToSlave;
    }
    throw std::runtime_error("conversion errror: Unknown lin::FrameResponseType");
}
auto idl::from_idl(idl::FrameResponseType idl) -> lin::FrameResponseType
{
    switch (idl)
    {
    case idl::MasterResponse:
        return lin::FrameResponseType::MasterResponse;
    case idl::SlaveResponse:
        return lin::FrameResponseType::SlaveResponse;
    case idl::SlaveToSlave:
        return lin::FrameResponseType::SlaveToSlave;
    }
    throw std::runtime_error("conversion errror: Unknown lin::FrameResponseType");
}

auto to_idl(FrameResponseMode msg) -> idl::FrameResponseMode
{
    switch (msg)
    {
    case FrameResponseMode::Unused:
        return idl::Unused;
    case FrameResponseMode::Rx:
        return idl::Rx;
    case FrameResponseMode::TxUnconditional:
        return idl::TxUnconditional;
    }
    throw std::runtime_error("conversion errror: Unknown lin::FrameResponseMode");
}
auto idl::from_idl(idl::FrameResponseMode idl) -> lin::FrameResponseMode
{
    switch (idl)
    {
    case idl::Unused:
        return lin::FrameResponseMode::Unused;
    case idl::Rx:
        return lin::FrameResponseMode::Rx;
    case idl::TxUnconditional:
        return lin::FrameResponseMode::TxUnconditional;
    }
    throw std::runtime_error("conversion errror: Unknown lin::FrameResponseMode");
}

auto to_idl(const FrameResponse& msg) -> idl::FrameResponse
{
    idl::FrameResponse idl;

    idl.frame(to_idl(msg.frame));
    idl.responseMode(to_idl(msg.responseMode));

    return idl;
}
auto idl::from_idl(idl::FrameResponse&& idl) -> lin::FrameResponse
{
    lin::FrameResponse msg;

    msg.frame = from_idl(std::move(idl.frame()));
    msg.responseMode = from_idl(idl.responseMode());

    return msg;
}

auto to_idl(FrameStatus msg) -> idl::FrameStatus
{
    switch (msg)
    {
    case FrameStatus::NOT_OK:
        return idl::NOT_OK;
    case FrameStatus::LIN_TX_OK:
        return idl::LIN_TX_OK;
    case FrameStatus::LIN_TX_BUSY:
        return idl::LIN_TX_BUSY;
    case FrameStatus::LIN_TX_HEADER_ERROR:
        return idl::LIN_TX_HEADER_ERROR;
    case FrameStatus::LIN_TX_ERROR:
        return idl::LIN_TX_ERROR;
    case FrameStatus::LIN_RX_OK:
        return idl::LIN_RX_OK;
    case FrameStatus::LIN_RX_BUSY:
        return idl::LIN_RX_BUSY;
    case FrameStatus::LIN_RX_ERROR:
        return idl::LIN_RX_ERROR;
    case FrameStatus::LIN_RX_NO_RESPONSE:
        return idl::LIN_RX_NO_RESPONSE;
    }
    throw std::runtime_error("conversion errror: Unknown lin::FrameStatus");
}
auto idl::from_idl(idl::FrameStatus idl) -> lin::FrameStatus
{
    switch (idl)
    {
    case idl::NOT_OK:
        return lin::FrameStatus::NOT_OK;
    case idl::LIN_TX_OK:
        return lin::FrameStatus::LIN_TX_OK;
    case idl::LIN_TX_BUSY:
        return lin::FrameStatus::LIN_TX_BUSY;
    case idl::LIN_TX_HEADER_ERROR:
        return lin::FrameStatus::LIN_TX_HEADER_ERROR;
    case idl::LIN_TX_ERROR:
        return lin::FrameStatus::LIN_TX_ERROR;
    case idl::LIN_RX_OK:
        return lin::FrameStatus::LIN_RX_OK;
    case idl::LIN_RX_BUSY:
        return lin::FrameStatus::LIN_RX_BUSY;
    case idl::LIN_RX_ERROR:
        return lin::FrameStatus::LIN_RX_ERROR;
    case idl::LIN_RX_NO_RESPONSE:
        return lin::FrameStatus::LIN_RX_NO_RESPONSE;
    }
    throw std::runtime_error("conversion errror: Unknown lin::FrameStatus");
}

auto to_idl(ControllerMode msg) -> idl::ControllerMode
{
    switch (msg)
    {
    case ControllerMode::Inactive:
        return idl::Inactive;
    case ControllerMode::Master:
        return idl::Master;
    case ControllerMode::Slave:
        return idl::Slave;
    }
    throw std::runtime_error("conversion errror: Unknown lin::ControllerMode");
}
auto idl::from_idl(idl::ControllerMode idl) -> lin::ControllerMode
{
    switch (idl)
    {
    case idl::Inactive:
        return lin::ControllerMode::Inactive;
    case idl::Master:
        return lin::ControllerMode::Master;
    case idl::Slave:
        return lin::ControllerMode::Slave;
    }
    throw std::runtime_error("conversion errror: Unknown lin::ControllerMode");
}

auto to_idl(const ControllerConfig& msg) -> idl::ControllerConfig
{
    idl::ControllerConfig idl;

    idl.controllerMode(to_idl(msg.controllerMode));
    idl.baudRate(msg.baudRate);
    for (auto& response : msg.frameResponses)
        idl.frameResponses().push_back(to_idl(response));

    return idl;
}
auto idl::from_idl(idl::ControllerConfig&& idl) -> lin::ControllerConfig
{
    lin::ControllerConfig msg;

    msg.controllerMode = from_idl(idl.controllerMode());
    msg.baudRate = idl.baudRate();
    for (auto& response : idl.frameResponses())
        msg.frameResponses.emplace_back(from_idl(std::move(response)));

    return msg;
}

auto to_idl(ControllerStatus msg) -> idl::ControllerStatus
{
    switch (msg)
    {
    case ControllerStatus::Unknown:
        return idl::Unknown;
    case ControllerStatus::Operational:
        return idl::Operational;
    case ControllerStatus::Sleep:
        return idl::Sleep;
    case ControllerStatus::SleepPending:
        return idl::SleepPending;
    }
    throw std::runtime_error("conversion errror: Unknown lin::ControllerStatus");
}
auto idl::from_idl(idl::ControllerStatus idl) -> lin::ControllerStatus
{
    switch (idl)
    {
    case idl::Unknown:
        return lin::ControllerStatus::Unknown;
    case idl::Operational:
        return lin::ControllerStatus::Operational;
    case idl::Sleep:
        return lin::ControllerStatus::Sleep;
    case idl::SleepPending:
        return lin::ControllerStatus::SleepPending;
    }
    throw std::runtime_error("conversion errror: Unknown lin::ControllerStatus");
}

auto to_idl(SendFrameRequest&& msg) -> idl::SendFrameRequest
{
    idl::SendFrameRequest idl;
    idl.frame(to_idl(std::move(msg.frame)));
    idl.responseType(to_idl(msg.responseType));
    return idl;
}
auto to_idl(const SendFrameRequest& msg) -> idl::SendFrameRequest
{
    idl::SendFrameRequest idl;
    idl.frame(to_idl(msg.frame));
    idl.responseType(to_idl(msg.responseType));
    return idl;
}
auto idl::from_idl(SendFrameRequest&& idl) -> lin::SendFrameRequest
{
    lin::SendFrameRequest msg;

    msg.frame = from_idl(std::move(idl.frame()));
    msg.responseType = from_idl(idl.responseType());

    return msg;
}

auto to_idl(SendFrameHeaderRequest&& msg) -> idl::SendFrameHeaderRequest
{
    idl::SendFrameHeaderRequest idl;
    idl.id(msg.id);
    return idl;
}
auto to_idl(const SendFrameHeaderRequest& msg) -> idl::SendFrameHeaderRequest
{
    idl::SendFrameHeaderRequest idl;
    idl.id(msg.id);
    return idl;
}
auto idl::from_idl(SendFrameHeaderRequest&& idl) -> lin::SendFrameHeaderRequest
{
    lin::SendFrameHeaderRequest msg;

    msg.id = idl.id();

    return msg;
}


auto to_idl(const Transmission& msg) -> idl::Transmission
{
    idl::Transmission idl;

    idl.timestampNs(std::chrono::duration_cast<std::chrono::nanoseconds>(msg.timestamp).count());
    idl.frame(to_idl(msg.frame));
    idl.status(to_idl(msg.status));

    return idl;
}
auto to_idl(Transmission&& msg) -> idl::Transmission
{
    return to_idl(msg);
}
auto idl::from_idl(idl::Transmission&& idl) -> lin::Transmission
{
    lin::Transmission msg;

    msg.timestamp = std::chrono::nanoseconds{idl.timestampNs()};
    msg.frame = from_idl(std::move(idl.frame()));
    msg.status = from_idl(idl.status());

    return msg;
}

auto to_idl(const FrameResponseUpdate& msg) -> idl::FrameResponseUpdate
{
    idl::FrameResponseUpdate idl;

    for (auto& response : msg.frameResponses)
        idl.frameResponses().push_back(to_idl(response));

    return idl;
}
auto to_idl(FrameResponseUpdate&& msg) -> idl::FrameResponseUpdate
{
    return to_idl(msg);
}
auto idl::from_idl(idl::FrameResponseUpdate&& idl) -> lin::FrameResponseUpdate
{
    lin::FrameResponseUpdate msg;

    for (auto&& response : idl.frameResponses())
        msg.frameResponses.emplace_back(from_idl(std::move(response)));

    return msg;
}

auto to_idl(const ControllerStatusUpdate& msg) -> idl::ControllerStatusUpdate
{
    idl::ControllerStatusUpdate idl;

    idl.timestampNs(std::chrono::duration_cast<std::chrono::nanoseconds>(msg.timestamp).count());
    idl.status(to_idl(msg.status));

    return idl;
}
auto to_idl(ControllerStatusUpdate&& msg) -> idl::ControllerStatusUpdate
{
    return to_idl(msg);
}
auto idl::from_idl(idl::ControllerStatusUpdate&& idl) -> lin::ControllerStatusUpdate
{
    lin::ControllerStatusUpdate msg;

    msg.timestamp = std::chrono::nanoseconds{idl.timestampNs()};
    msg.status = from_idl(idl.status());

    return msg;
}

auto to_idl(const WakeupPulse& msg) -> idl::WakeupPulse
{
    idl::WakeupPulse idl;

    idl.timestampNs(std::chrono::duration_cast<std::chrono::nanoseconds>(msg.timestamp).count());

    return idl;
}
auto to_idl(WakeupPulse&& msg) -> idl::WakeupPulse
{
    return to_idl(msg);
}
auto idl::from_idl(idl::WakeupPulse&& idl) -> lin::WakeupPulse
{
    lin::WakeupPulse msg;

    msg.timestamp = std::chrono::nanoseconds{idl.timestampNs()};

    return msg;
}


} // namespace lin
} // namespace sim
} // namespace ib
