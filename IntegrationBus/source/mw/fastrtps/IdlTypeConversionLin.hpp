// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/lin/LinDatatypes.hpp"

#include "idl/LinTopics.h"
#include "idl/LinTopicsPubSubTypes.h"


namespace ib {
namespace sim {
namespace lin {

inline auto to_idl(ControllerMode msg) -> idl::ControllerMode;
inline auto to_idl(ChecksumModel msg) -> idl::ChecksumModel;
inline auto to_idl(ResponseMode msg) -> idl::ResponseMode;
inline auto to_idl(MessageStatus msg) -> idl::MessageStatus;
inline auto to_idl(const LinMessage& msg) -> idl::LinMessage;
inline auto to_idl(LinMessage&& msg) -> idl::LinMessage;
inline auto to_idl(const RxRequest& msg) -> idl::RxRequest;
inline auto to_idl(const TxAcknowledge& msg) -> idl::TxAcknowledge;
inline auto to_idl(const WakeupRequest& msg) -> idl::WakeupRequest;
inline auto to_idl(const ControllerConfig& msg) -> idl::ControllerConfig;
inline auto to_idl(const SlaveResponseConfig& msg) -> idl::SlaveResponseConfig;
inline auto to_idl(const SlaveConfiguration& msg) -> idl::SlaveConfiguration;
inline auto to_idl(const SlaveResponse& msg) -> idl::SlaveResponse;

namespace idl {
inline auto from_idl(ControllerMode idl) -> lin::ControllerMode;
inline auto from_idl(ChecksumModel idl) -> lin::ChecksumModel;
inline auto from_idl(ResponseMode idl) -> lin::ResponseMode;
inline auto from_idl(MessageStatus idl) -> lin::MessageStatus;
inline auto from_idl(LinMessage&& idl) -> lin::LinMessage;
inline auto from_idl(RxRequest&& idl) -> lin::RxRequest;
inline auto from_idl(TxAcknowledge&& idl) -> lin::TxAcknowledge;
inline auto from_idl(WakeupRequest&& idl) -> lin::WakeupRequest;
inline auto from_idl(const ControllerConfig& idl) -> lin::ControllerConfig;
inline auto from_idl(const SlaveResponseConfig& idl) -> lin::SlaveResponseConfig;
inline auto from_idl(const SlaveConfiguration& idl) -> lin::SlaveConfiguration;
inline auto from_idl(const SlaveResponse& idl) -> lin::SlaveResponse;
} // namespace idl

// ================================================================================
//  Inline Implementations
// ================================================================================
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
    case ControllerMode::Sleep:
        return idl::Sleep;
    default:
        throw std::runtime_error("conversion errror: Unknown lin::ControllerMode");
    }
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
    case idl::Sleep:
        return lin::ControllerMode::Sleep;
    default:
        throw std::runtime_error("conversion errror: Unknown lin::ControllerMode");
    }
}

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
    default:
        throw std::exception();
    }
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
    default:
        throw std::exception();
    }
}

auto to_idl(ResponseMode msg) -> idl::ResponseMode
{
    switch (msg)
    {
    case ResponseMode::Unused:
        return idl::Unused;
    case ResponseMode::Rx:
        return idl::Rx;
    case ResponseMode::TxUnconditional:
        return idl::TxUnconditional;
    default:
        throw std::exception();
    }
}

auto idl::from_idl(idl::ResponseMode idl) -> lin::ResponseMode
{
    switch (idl)
    {
    case idl::Unused:
        return lin::ResponseMode::Unused;
    case idl::Rx:
        return lin::ResponseMode::Rx;
    case idl::TxUnconditional:
        return lin::ResponseMode::TxUnconditional;
    default:
        throw std::exception();
    }
}

auto to_idl(MessageStatus msg) -> idl::MessageStatus
{
    switch (msg)
    {
    case MessageStatus::TxSuccess:
        return idl::TxSuccess;
    case MessageStatus::RxSuccess:
        return idl::RxSuccess;
    case MessageStatus::TxResponseError:
        return idl::TxResponseError;
    case MessageStatus::RxResponseError:
        return idl::RxResponseError;
    case MessageStatus::RxNoResponse:
        return idl::RxNoResponse;
    case MessageStatus::HeaderError:
        return idl::HeaderError;
    case MessageStatus::Canceled:
        return idl::Canceled;
    case MessageStatus::Busy:
        return idl::Busy;
    default:
        throw std::exception();
    }
}

auto idl::from_idl(idl::MessageStatus idl) -> lin::MessageStatus
{
    switch (idl)
    {
    case idl::TxSuccess:
        return lin::MessageStatus::TxSuccess;
    case idl::RxSuccess:
        return lin::MessageStatus::RxSuccess;
    case idl::TxResponseError:
        return lin::MessageStatus::TxResponseError;
    case idl::RxResponseError:
        return lin::MessageStatus::RxResponseError;
    case idl::RxNoResponse:
        return lin::MessageStatus::RxNoResponse;
    case idl::HeaderError:
        return lin::MessageStatus::HeaderError;
    case idl::Canceled:
        return lin::MessageStatus::Canceled;
    case idl::Busy:
        return lin::MessageStatus::Busy;
    default:
        throw std::exception();
    }
}

auto to_idl(const LinMessage& msg) -> idl::LinMessage
{
    idl::LinMessage idl;

    idl.status(to_idl(msg.status));
    idl.timestamp(msg.timestamp.count());

    idl.linId(msg.linId);
    idl.payloadLength(msg.payload.size);
    idl.payload(msg.payload.data);
    idl.checksumModel(to_idl(msg.checksumModel));

    return idl;
}

auto to_idl(LinMessage&& msg) -> idl::LinMessage
{
    return to_idl(msg);
}

auto idl::from_idl(idl::LinMessage&& idl) -> lin::LinMessage
{
    lin::LinMessage msg;

    msg.status = from_idl(idl.status());
    msg.timestamp = std::chrono::nanoseconds{idl.timestamp()};

    msg.linId = idl.linId();
    msg.payload.size = idl.payloadLength();
    msg.payload.data = idl.payload();
    msg.checksumModel = from_idl(idl.checksumModel());

    return msg;
}

auto to_idl(const RxRequest& msg) -> idl::RxRequest
{
    idl::RxRequest idl;

    idl.linId(msg.linId);
    idl.payloadLength(msg.payloadLength);
    idl.checksumModel(to_idl(msg.checksumModel));

    return idl;
}

auto idl::from_idl(idl::RxRequest&& idl) -> lin::RxRequest
{
    lin::RxRequest msg;

    msg.linId = idl.linId();
    msg.payloadLength = idl.payloadLength();
    msg.checksumModel = from_idl(idl.checksumModel());

    return msg;
}

auto to_idl(const TxAcknowledge& msg) -> idl::TxAcknowledge
{
    idl::TxAcknowledge idl;

    idl.timestamp(msg.timestamp.count());
    idl.linId(msg.linId);
    idl.status(to_idl(msg.status));

    return idl;
}

auto idl::from_idl(idl::TxAcknowledge&& idl) -> lin::TxAcknowledge
{
    lin::TxAcknowledge msg;

    msg.timestamp = std::chrono::nanoseconds{idl.timestamp()};

    msg.linId = idl.linId();
    msg.status = from_idl(idl.status());

    return msg;
}

auto to_idl(const WakeupRequest& msg) -> idl::WakeupRequest
{
    idl::WakeupRequest idl;

    idl.timestampNs(std::chrono::duration_cast<std::chrono::nanoseconds>(msg.timestamp).count());

    return idl;
}

auto idl::from_idl(idl::WakeupRequest&& idl) -> lin::WakeupRequest
{
    lin::WakeupRequest msg;

    msg.timestamp = std::chrono::nanoseconds{idl.timestampNs()};

    return msg;
}

auto to_idl(const ControllerConfig& msg) -> idl::ControllerConfig
{
    idl::ControllerConfig idl;

    idl.controllerMode(to_idl(msg.controllerMode));
    idl.baudrate(msg.baudrate);

    return idl;
}

auto idl::from_idl(const idl::ControllerConfig& idl) -> lin::ControllerConfig
{
    lin::ControllerConfig msg;

    msg.controllerMode = from_idl(idl.controllerMode());
    msg.baudrate = idl.baudrate();

    return msg;
}


auto to_idl(const SlaveResponseConfig& msg) -> idl::SlaveResponseConfig
{
    idl::SlaveResponseConfig idl;

    idl.linId(msg.linId);
    idl.responseMode(to_idl(msg.responseMode));
    idl.checksumModel(to_idl(msg.checksumModel));
    idl.payloadLength(msg.payloadLength);

    return idl;
}

auto idl::from_idl(const idl::SlaveResponseConfig& idl) -> lin::SlaveResponseConfig
{
    lin::SlaveResponseConfig msg;

    msg.linId = idl.linId();
    msg.responseMode = from_idl(idl.responseMode());
    msg.checksumModel = from_idl(idl.checksumModel());
    msg.payloadLength = idl.payloadLength();

    return msg;
}

auto to_idl(const SlaveConfiguration& msg) -> idl::SlaveConfiguration
{
    std::vector<idl::SlaveResponseConfig> idlResponses;
    idlResponses.reserve(msg.responseConfigs.size());

    for (auto&& response : msg.responseConfigs)
    {
        idlResponses.push_back(to_idl(response));
    }

    idl::SlaveConfiguration idl;
    idl.responseConfigs(std::move(idlResponses));
    return idl;
}

auto idl::from_idl(const idl::SlaveConfiguration& idl) -> lin::SlaveConfiguration
{
    lin::SlaveConfiguration msg;
    msg.responseConfigs.reserve(idl.responseConfigs().size());

    for (auto&& idlResponse : idl.responseConfigs())
    {
        msg.responseConfigs.push_back(from_idl(idlResponse));
    }

    return msg;
}

auto to_idl(const SlaveResponse& msg) -> idl::SlaveResponse
{
    idl::SlaveResponse idl;

    idl.linId(msg.linId);
    idl.payloadLength(msg.payload.size);
    idl.payload(msg.payload.data);
    idl.checksumModel(to_idl(msg.checksumModel));

    return idl;
}

auto idl::from_idl(const idl::SlaveResponse& idl) -> lin::SlaveResponse
{
    lin::SlaveResponse msg;

    msg.linId = idl.linId();
    msg.payload.size = idl.payloadLength();
    msg.payload.data = idl.payload();
    msg.checksumModel = from_idl(idl.checksumModel());

    return msg;
}


} // namespace lin
} // namespace sim
} // namespace ib
