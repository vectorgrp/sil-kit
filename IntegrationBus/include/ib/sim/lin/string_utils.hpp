// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <iomanip>
#include <sstream>

#include "LinDatatypes.hpp"

#include "ib/exception.hpp"
#include "ib/util/PrintableHexString.hpp"

namespace ib {
namespace sim {
namespace lin { 

inline std::string to_string(LinChecksumModel model);
inline std::string to_string(LinFrameResponseType responseType);
inline std::string to_string(LinFrameResponseMode responseMode);
inline std::string to_string(LinFrameStatus frameStatus);
inline std::string to_string(LinControllerMode mode);
inline std::string to_string(LinControllerStatus status);

inline std::string to_string(const LinFrame& frame);
inline std::string to_string(const LinSendFrameRequest& request);
inline std::string to_string(const LinSendFrameHeaderRequest& request);
inline std::string to_string(const LinTransmission& transmission);
inline std::string to_string(const LinWakeupPulse& pulse);
inline std::string to_string(const LinControllerConfig& controllerConfig);
inline std::string to_string(const LinControllerStatusUpdate& controllerStatusUpdate);
inline std::string to_string(const LinFrameResponseUpdate& frameResponseUpdate);

inline std::ostream& operator<<(std::ostream& out, LinChecksumModel model);
inline std::ostream& operator<<(std::ostream& out, LinFrameResponseType responseType);
inline std::ostream& operator<<(std::ostream& out, LinFrameResponseMode responseMode);
inline std::ostream& operator<<(std::ostream& out, LinFrameStatus frameStatus);       
inline std::ostream& operator<<(std::ostream& out, LinControllerMode mode);           
inline std::ostream& operator<<(std::ostream& out, LinControllerStatus status);

inline std::ostream& operator<<(std::ostream& out, const LinFrame& frame);
inline std::ostream& operator<<(std::ostream& out, const LinSendFrameRequest& request);
inline std::ostream& operator<<(std::ostream& out, const LinSendFrameHeaderRequest& request);
inline std::ostream& operator<<(std::ostream& out, const LinTransmission& transmission);
inline std::ostream& operator<<(std::ostream& out, const LinWakeupPulse& pulse);
inline std::ostream& operator<<(std::ostream& out, const LinControllerConfig& controllerConfig);
inline std::ostream& operator<<(std::ostream& out, const LinControllerStatusUpdate& controllerStatusUpdate);
inline std::ostream& operator<<(std::ostream& out, const LinFrameResponseUpdate& frameResponseUpdate);


// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(LinChecksumModel model)
{
    switch (model)
    {
    case LinChecksumModel::Undefined:
        return "Undefined";
    case LinChecksumModel::Enhanced:
        return "Enhanced";
    case LinChecksumModel::Classic:
        return "Classic";
    }
    throw ib::TypeConversionError{};
}

std::string to_string(LinFrameResponseType responseType)
{
    switch (responseType)
    {
    case LinFrameResponseType::MasterResponse:
        return "MasterResponse";
    case LinFrameResponseType::SlaveResponse:
        return "SlaveResponse";
    case LinFrameResponseType::SlaveToSlave:
        return "SlaveToSlave";
    }
    throw ib::TypeConversionError{};
}

std::string to_string(LinFrameResponseMode responseMode)
{
    switch (responseMode)
    {
    case LinFrameResponseMode::Unused:
        return "Unused";
    case LinFrameResponseMode::Rx:
        return "Rx";
    case LinFrameResponseMode::TxUnconditional:
        return "TxUnconditional";
    }
    throw ib::TypeConversionError{};
}

std::string to_string(LinFrameStatus frameStatus)
{
    switch (frameStatus)
    {
    case LinFrameStatus::NOT_OK:
        return "NOT_OK";
    case LinFrameStatus::LIN_TX_OK:
        return "LIN_TX_OK";
    case LinFrameStatus::LIN_TX_BUSY:
        return "LIN_TX_BUSY";
    case LinFrameStatus::LIN_TX_HEADER_ERROR:
        return "LIN_TX_HEADER_ERROR";
    case LinFrameStatus::LIN_TX_ERROR:
        return "LIN_TX_ERROR";
    case LinFrameStatus::LIN_RX_OK:
        return "LIN_RX_OK";
    case LinFrameStatus::LIN_RX_BUSY:
        return "LIN_RX_BUSY";
    case LinFrameStatus::LIN_RX_ERROR:
        return "LIN_RX_ERROR";
    case LinFrameStatus::LIN_RX_NO_RESPONSE:
        return "LIN_RX_NO_RESPONSE";
    }
    throw ib::TypeConversionError{};
}

std::string to_string(LinControllerMode mode)
{
    switch (mode)
    {
    case LinControllerMode::Inactive:
        return "Inactive";
    case LinControllerMode::Master:
        return "Master";
    case LinControllerMode::Slave:
        return "Slave";
    }
    throw ib::TypeConversionError{};
}


std::string to_string(LinControllerStatus status)
{
    switch (status)
    {
    case LinControllerStatus::Unknown:
        return "Unknown";
    case LinControllerStatus::Operational:
        return "Operational";
    case LinControllerStatus::Sleep:
        return "Sleep";
    case LinControllerStatus::SleepPending:
        return "SleepPending";
    }
    throw ib::TypeConversionError{};
}


std::string to_string(const LinFrame& frame)
{
    std::stringstream out;
    out << frame;
    return out.str();
}

std::string to_string(const LinSendFrameRequest& request)
{
    std::stringstream out;
    out << request;
    return out.str();
}

std::string to_string(const LinSendFrameHeaderRequest& request)
{
    std::stringstream out;
    out << request;
    return out.str();
}

std::string to_string(const LinTransmission& transmission)
{
    std::stringstream out;
    out << transmission;
    return out.str();
}

std::string to_string(const LinWakeupPulse& pulse)
{
    std::stringstream out;
    out << pulse;
    return out.str();
}

std::string to_string(const LinControllerConfig& controllerConfig)
{
    std::stringstream out;
    out << controllerConfig;
    return out.str();
}

std::string to_string(const LinControllerStatusUpdate& controllerStatusUpdate)
{
    std::stringstream out;
    out << controllerStatusUpdate;
    return out.str();
}

std::string to_string(const LinFrameResponseUpdate& frameResponseUpdate)
{
    std::stringstream out;
    out << frameResponseUpdate;
    return out.str();
}



std::ostream& operator<<(std::ostream& out, LinChecksumModel model)
{
    try
    {
        return out << to_string(model);
    }
    catch (const ib::TypeConversionError&)
    {
        return out << "LinChecksumModel{" << static_cast<uint32_t>(model) << "}";
    }
}
std::ostream& operator<<(std::ostream& out, LinFrameResponseType responseType)
    {
    try
    {
        return out << to_string(responseType);
    }
    catch (const ib::TypeConversionError&)
    {
        return out << "LinFrameResponseType{" << static_cast<uint32_t>(responseType) << "}";
    }
}

std::ostream& operator<<(std::ostream& out, LinFrameResponseMode responseMode)
    {
    try
    {
        return out << to_string(responseMode);
    }
    catch (const ib::TypeConversionError&)
    {
        return out << "LinFrameResponseMode{" << static_cast<uint32_t>(responseMode) << "}";
    }
}

std::ostream& operator<<(std::ostream& out, LinFrameStatus frameStatus)
{
    try
    {
        return out << to_string(frameStatus);
    }
    catch (const ib::TypeConversionError&)
    {
        return out << "LinFrameStatus{" << static_cast<uint32_t>(frameStatus) << "}";
    }
}

std::ostream& operator<<(std::ostream& out, LinControllerMode mode)
{
    try
    {
        return out << to_string(mode);
    }
    catch (const ib::TypeConversionError&)
    {
        return out << "LinControllerMode{" << static_cast<uint32_t>(mode) << "}";
    }
}

std::ostream& operator<<(std::ostream& out, LinControllerStatus status)
{
    try
    {
        return out << to_string(status);
    }
    catch (const ib::TypeConversionError&)
    {
        return out << "LinControllerStatus{" << static_cast<uint32_t>(status) << "}";
    }
}

std::ostream& operator<<(std::ostream& out, const LinFrame& frame)
{
    //instead of ios::copyfmt (which set badbit) we use a temporary stream 
    std::stringstream buf;
    buf
        << "lin::LinFrame{id=" << static_cast<uint16_t>(frame.id)
        << ", cs=" << to_string(frame.checksumModel)
        << ", dl=" << static_cast<uint16_t>(frame.dataLength)
        << ", d={" << util::AsHexString(frame.data).WithSeparator(" ")
        << "}}";
    return out << buf.str();
}

std::ostream& operator<<(std::ostream& out, const LinSendFrameRequest& request)
{
    return out
        << "lin::LinSendFrameRequest{fr=" << request.frame
        << ", rt=" << request.responseType
        << "}";
}

std::ostream& operator<<(std::ostream& out, const LinSendFrameHeaderRequest& request)
{
    return out << "lin::LinSendFrameHeaderRequest{id=" << static_cast<uint16_t>(request.id) << "}";
}

std::ostream& operator<<(std::ostream& out, const LinTransmission& transmission)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(transmission.timestamp);
    return out
        << "lin::LinTransmission{" << transmission.frame
        << ", status=" << transmission.status
        << ", time=" << timestamp.count() << "ms}";
}
std::ostream& operator<<(std::ostream& out, const LinWakeupPulse& pulse)
{
    return out << "lin::LinWakeupPulse{@" << pulse.timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, const LinControllerConfig& controllerConfig)
{
    out << "lin::LinControllerConfig{br=" << controllerConfig.baudRate
        << ", mode=" << controllerConfig.controllerMode
        << ", responses=[";
    if (controllerConfig.frameResponses.size() > 0)
    {
        out << static_cast<uint16_t>(controllerConfig.frameResponses[0].frame.id);
        for (auto i = 1u; i < controllerConfig.frameResponses.size(); ++i)
        {
            out << "," << static_cast<uint16_t>(controllerConfig.frameResponses[1].frame.id);
        }
    }
    out << "]}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const LinControllerStatusUpdate& controllerStatusUpdate)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(controllerStatusUpdate.timestamp);
    return out
        << "lin::LinControllerStatusUpdate{" << controllerStatusUpdate.status
        << " @" << timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, const LinFrameResponseUpdate& frameResponseUpdate)
{
    auto& responses = frameResponseUpdate.frameResponses;
    out << "lin::LinFrameResponseUpdate{ids=[";

    if (responses.size() > 0)
    {
        out << static_cast<uint16_t>(responses[0].frame.id);
        for (auto i = 1u; i < responses.size(); ++i)
        {
            out << "," << static_cast<uint16_t>(responses[1].frame.id);
        }
    }

    out << "]}";
    return out;
}


    
} // namespace lin
} // namespace sim
} // namespace ib
