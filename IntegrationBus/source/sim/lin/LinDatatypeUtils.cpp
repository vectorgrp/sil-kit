// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "LinDatatypeUtils.hpp"

namespace ib {
namespace sim {
namespace lin { 

bool operator==(const LinMessage& lhs, const LinMessage& rhs)
{
    return lhs.status == rhs.status
        && lhs.linId == rhs.linId
        && lhs.payload == rhs.payload
        && lhs.checksumModel == rhs.checksumModel;
}

bool operator==(const RxRequest& lhs, const RxRequest& rhs)
{
    return lhs.linId == rhs.linId
        && lhs.payloadLength == rhs.payloadLength
        && lhs.checksumModel == rhs.checksumModel;
}

bool operator==(const TxAcknowledge& lhs, const TxAcknowledge& rhs)
{
    return lhs.linId == rhs.linId
        && lhs.status == rhs.status;
}

bool operator==(const ControllerConfig& lhs, const ControllerConfig& rhs)
{
    return lhs.baudrate == rhs.baudrate
        && lhs.controllerMode == rhs.controllerMode;
}

bool operator==(const SlaveResponseConfig& lhs, const SlaveResponseConfig& rhs)
{
    return lhs.responseMode == rhs.responseMode
        && lhs.checksumModel == rhs.checksumModel
        && lhs.payloadLength == rhs.payloadLength;
}

bool operator==(const SlaveResponse& lhs, const SlaveResponse& rhs)
{
    return lhs.linId == rhs.linId
        && lhs.payload == rhs.payload
        && lhs.checksumModel == rhs.checksumModel;
}


std::ostream& operator<<(std::ostream& out, MessageStatus status)
{
    switch (status)
    {
    case MessageStatus::TxSuccess:
        return out << "TxSuccess";
    case MessageStatus::RxSuccess:
        return out << "RxSuccess";
    case MessageStatus::TxResponseError:
        return out << "TxResponseError";
    case MessageStatus::RxResponseError:
        return out << "RxResponseError";
    case MessageStatus::RxNoResponse:
        return out << "RxNoResponse";
    case MessageStatus::HeaderError:
        return out << "HeaderError";
    case MessageStatus::Canceled:
        return out << "Canceled";
    case MessageStatus::Busy:
        return out << "Busy";
    default:
        return out << "MessageStatus=" << static_cast<unsigned int>(status);
    }
}

std::ostream& operator<<(std::ostream& out, ChecksumModel model)
{
    switch (model)
    {
    case ChecksumModel::Undefined:
        return out << "Undefined";
    case ChecksumModel::Enhanced:
        return out << "Enhanced";
    case ChecksumModel::Classic:
        return out << "Classic";
    default:
        return out << "ChecksumModel=" << static_cast<unsigned int>(model);
    }
}

std::ostream& operator<<(std::ostream& out, const Payload& payload)
{
    std::ios oldFormat(nullptr);
    oldFormat.copyfmt(out);

    out << "{";
    if (payload.size)
    {
        out.fill('0');
        out.width(2);
        out << std::hex << static_cast<uint32_t>(payload.data[0]);
        for (int i = 1; i < payload.size; i++)
        {
            out << " ";
            out.fill('0');
            out.width(2);
            out << static_cast<uint32_t>(payload.data[i]);
        }
    }
    out << "}";

    out.copyfmt(oldFormat);
    return out;
}

std::ostream& operator<<(std::ostream& out, const LinMessage& msg)
{
    using FloatMs = std::chrono::duration<double, std::milli>;
    auto printTime = std::chrono::duration_cast<FloatMs>(msg.timestamp);
    out << "LinMessage{s=" << msg.status
        << ", t=" << printTime.count() << "ms"
        << ", id=" << static_cast<uint32_t>(msg.linId)
        << ", p=" << msg.payload
        << ", csm=" << msg.checksumModel
        << "}";

    return out;
}


} // namespace lin
} // namespace sim
} // namespace ib
