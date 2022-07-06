// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>
#include <sstream>

#include "silkit/exception.hpp"
#include "silkit/util/PrintableHexString.hpp"
#include "silkit/services/string_utils.hpp"

#include "CanDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Can {

inline std::string to_string(CanControllerState state);
inline std::string to_string(CanErrorState state);
inline std::string to_string(CanFrame::CanFrameFlags flags);
inline std::string to_string(CanTransmitStatus status);
inline std::string to_string(const CanFrame& msg);
inline std::string to_string(const CanFrameEvent& msg);
inline std::string to_string(const CanControllerStatus& status);
inline std::string to_string(const CanFrameTransmitEvent& ack);
inline std::string to_string(const CanConfigureBaudrate& rate);
inline std::string to_string(const CanSetControllerMode& mode);

inline std::ostream& operator<<(std::ostream& out, CanControllerState state);
inline std::ostream& operator<<(std::ostream& out, CanErrorState state);
inline std::ostream& operator<<(std::ostream& out, CanFrame::CanFrameFlags flags);
inline std::ostream& operator<<(std::ostream& out, CanTransmitStatus status);
inline std::ostream& operator<<(std::ostream& out, const CanFrame& msg);
inline std::ostream& operator<<(std::ostream& out, const CanFrameEvent& msg);
inline std::ostream& operator<<(std::ostream& out, const CanControllerStatus& status);
inline std::ostream& operator<<(std::ostream& out, const CanFrameTransmitEvent& status);
inline std::ostream& operator<<(std::ostream& out, const CanConfigureBaudrate& rate);
inline std::ostream& operator<<(std::ostream& out, const CanSetControllerMode& mode);


// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(CanControllerState state)
{
    switch (state)
    {
    case CanControllerState::Uninit:
        return "Uninit";
    case CanControllerState::Stopped:
        return "Stopped";
    case CanControllerState::Started:
        return "Started";
    case CanControllerState::Sleep:
        return "Sleep";
    };
    throw SilKit::TypeConversionError{};
}

std::string to_string(CanErrorState state)
{
    switch (state)
    {
    case CanErrorState::NotAvailable:
        return "NotAvailable";
    case CanErrorState::ErrorActive:
        return "ErrorActive";
    case CanErrorState::ErrorPassive:
        return "ErrorPassive";
    case CanErrorState::BusOff:
        return "BusOff";
    }
    throw SilKit::TypeConversionError{};
}

std::string to_string(CanFrame::CanFrameFlags flags)
{
    std::stringstream outStream;
    outStream << flags;
    return outStream.str();
}

std::string to_string(CanTransmitStatus status)
{
    switch (status)
    {
    case CanTransmitStatus::Transmitted:
        return "Transmitted";
    case CanTransmitStatus::Canceled:
        return "Canceled";
    case CanTransmitStatus::TransmitQueueFull:
        return "TransmitQueueFull";
    case CanTransmitStatus::DuplicatedTransmitId:
        return "DuplicatedTransmitId";
    }
    throw SilKit::TypeConversionError{};
}

std::string to_string(const CanFrame& msg)
{
    std::stringstream outStream;
    outStream << msg;
    return outStream.str();
}

std::string to_string(const CanFrameEvent& msg)
{
    std::stringstream outStream;
    outStream << msg;
    return outStream.str();
}

std::string to_string(const CanControllerStatus& status)
{
    std::stringstream outStream;
    outStream << status;
    return outStream.str();
}

std::string to_string(const CanFrameTransmitEvent& ack)
{
    std::stringstream outStream;
    outStream << ack;
    return outStream.str();
}

std::string to_string(const CanConfigureBaudrate& rate)
{
    std::stringstream outStream;
    outStream << rate;
    return outStream.str();
}

std::string to_string(const CanSetControllerMode& mode)
{
    std::stringstream outStream;
    outStream << mode;
    return outStream.str();
}



std::ostream& operator<<(std::ostream& out, CanControllerState state)
{
    return out << to_string(state);
}

std::ostream& operator<<(std::ostream& out, CanErrorState state)
{
    return out << to_string(state);
}

std::ostream& operator<<(std::ostream& out, CanFrame::CanFrameFlags flags)
{
    out << "["
        << (flags.ide ? "ide," : "")
        << (flags.rtr ? "rtr," : "")
        << (flags.fdf ? "fdf," : "")
        << (flags.brs ? "brs," : "")
        << (flags.esi ? "esi" : "")
        << "]";

    return out;
}

std::ostream& operator<<(std::ostream& out, CanTransmitStatus status)
{
    return out << to_string(status);
}

std::ostream& operator<<(std::ostream& out, const CanFrame& msg)
{
    return out
        << "Can::CanFrame{"
        << ", canId=" << msg.canId
        << ", flags=" << msg.flags
        << ", dlc=" << static_cast<uint32_t>(msg.dlc)
        << ", data=[" << Util::AsHexString(msg.dataField).WithSeparator(" ").WithMaxLength(8)
        << "], data.size=" << msg.dataField.size() << "}";
}


std::ostream& operator<<(std::ostream& out, const CanFrameEvent& msg)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg.timestamp);
    return out
        << "Can::CanFrameEvent{txId=" << msg.transmitId 
        << ", userContext=" << msg.userContext
        << ", direction=" << msg.direction
        << ", frame=" << msg.frame
        << " @" << timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, const CanControllerStatus& status)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(status.timestamp);
    return out
        << "Can::CanControllerStatus{CtrlState=" << status.controllerState
        << ", ErrorState=" << status.errorState
        << " @" << timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, const CanFrameTransmitEvent& status)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(status.timestamp);
    return out
        << "Can::CanTtransmitAcknowledge{txId=" << status.transmitId
        << ", status=" << status.status
        << " @" << timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, const CanConfigureBaudrate& rate)
{
    return out
        << "Can::CanConfigureBaudrate{" << rate.baudRate
        << ", " << rate.fdBaudRate
        << "}";
}

std::ostream& operator<<(std::ostream& out, const CanSetControllerMode& mode)
{
    out << "Can::CanSetControllerMode{" << mode.mode;

    if (mode.flags.cancelTransmitRequests)
        out << ", cancelTX";
    if (mode.flags.resetErrorHandling)
        out << ", resetErrorHandling";
    out << "}";
    return out;
}




}
}
}
