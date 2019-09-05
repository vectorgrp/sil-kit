// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <iomanip>

#include "LinDatatypes.hpp"

#include "ib/exception.hpp"

namespace ib {
namespace sim {
namespace lin { 

inline std::string to_string(ChecksumModel model);
inline std::string to_string(FrameResponseType responseType);
inline std::string to_string(FrameResponseMode responseMode);
inline std::string to_string(FrameStatus frameStatus);
inline std::string to_string(ControllerMode mode);
inline std::string to_string(ControllerStatus status);       

inline std::ostream& operator<<(std::ostream& out, ChecksumModel model);
inline std::ostream& operator<<(std::ostream& out, FrameResponseType responseType);
inline std::ostream& operator<<(std::ostream& out, FrameResponseMode responseMode);
inline std::ostream& operator<<(std::ostream& out, FrameStatus frameStatus);       
inline std::ostream& operator<<(std::ostream& out, ControllerMode mode);           
inline std::ostream& operator<<(std::ostream& out, ControllerStatus status);

inline std::ostream& operator<<(std::ostream& out, const Frame& frame);

// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(ChecksumModel model)
{
    switch (model)
    {
    case ChecksumModel::Undefined:
        return "Undefined";
    case ChecksumModel::Enhanced:
        return "Enhanced";
    case ChecksumModel::Classic:
        return "Classic";
    }
    throw ib::type_conversion_error{};
}

std::string to_string(FrameResponseType responseType)
{
    switch (responseType)
    {
    case FrameResponseType::MasterResponse:
        return "MasterResponse";
    case FrameResponseType::SlaveResponse:
        return "SlaveResponse";
    case FrameResponseType::SlaveToSlave:
        return "SlaveToSlave";
    }
    throw ib::type_conversion_error{};
}

std::string to_string(FrameResponseMode responseMode)
{
    switch (responseMode)
    {
    case FrameResponseMode::Unused:
        return "Unused";
    case FrameResponseMode::Rx:
        return "Rx";
    case FrameResponseMode::TxUnconditional:
        return "TxUnconditional";
    }
    throw ib::type_conversion_error{};
}

std::string to_string(FrameStatus frameStatus)
{
    switch (frameStatus)
    {
    case FrameStatus::NOT_OK:
        return "NOT_OK";
    case FrameStatus::LIN_TX_OK:
        return "LIN_TX_OK";
    case FrameStatus::LIN_TX_BUSY:
        return "LIN_TX_BUSY";
    case FrameStatus::LIN_TX_HEADER_ERROR:
        return "LIN_TX_HEADER_ERROR";
    case FrameStatus::LIN_TX_ERROR:
        return "LIN_TX_ERROR";
    case FrameStatus::LIN_RX_OK:
        return "LIN_RX_OK";
    case FrameStatus::LIN_RX_BUSY:
        return "LIN_RX_BUSY";
    case FrameStatus::LIN_RX_ERROR:
        return "LIN_RX_ERROR";
    case FrameStatus::LIN_RX_NO_RESPONSE:
        return "LIN_RX_NO_RESPONSE";
    }
    throw ib::type_conversion_error{};
}

std::string to_string(ControllerMode mode)
{
    switch (mode)
    {
    case ControllerMode::Inactive:
        return "Inactive";
    case ControllerMode::Master:
        return "Master";
    case ControllerMode::Slave:
        return "Slave";
    }
    throw ib::type_conversion_error{};
}


std::string to_string(ControllerStatus status)
{
    switch (status)
    {
    case ControllerStatus::Unknown:
        return "Unknown";
    case ControllerStatus::Operational:
        return "Operational";
    case ControllerStatus::Sleep:
        return "Sleep";
    }
    throw ib::type_conversion_error{};
};

std::ostream& operator<<(std::ostream& out, ChecksumModel model)
{
    try
    {
        return out << to_string(model);
    }
    catch (const ib::type_conversion_error&)
    {
        return out << "ChecksumModel{" << static_cast<uint32_t>(model) << "}";
    }
}
std::ostream& operator<<(std::ostream& out, FrameResponseType responseType)
    {
    try
    {
        return out << to_string(responseType);
    }
    catch (const ib::type_conversion_error&)
    {
        return out << "FrameResponseType{" << static_cast<uint32_t>(responseType) << "}";
    }
}

inline std::ostream& operator<<(std::ostream& out, FrameResponseMode responseMode)
    {
    try
    {
        return out << to_string(responseMode);
    }
    catch (const ib::type_conversion_error&)
    {
        return out << "FrameResponseMode{" << static_cast<uint32_t>(responseMode) << "}";
    }
}

inline std::ostream& operator<<(std::ostream& out, FrameStatus frameStatus)
{
    try
    {
        return out << to_string(frameStatus);
    }
    catch (const ib::type_conversion_error&)
    {
        return out << "FrameStatus{" << static_cast<uint32_t>(frameStatus) << "}";
    }
}

inline std::ostream& operator<<(std::ostream& out, ControllerMode mode)
{
    try
    {
        return out << to_string(mode);
    }
    catch (const ib::type_conversion_error&)
    {
        return out << "ControllerMode{" << static_cast<uint32_t>(mode) << "}";
    }
}

inline std::ostream& operator<<(std::ostream& out, ControllerStatus status)
{
    try
    {
        return out << to_string(status);
    }
    catch (const ib::type_conversion_error&)
    {
        return out << "ControllerStatus{" << static_cast<uint32_t>(status) << "}";
    }
}

inline std::ostream& operator<<(std::ostream& out, const Frame& frame)
{
    std::ios oldState(nullptr);
    oldState.copyfmt(out);

    out
        << "Frame{id=" << static_cast<uint16_t>(frame.id)
        << ", cs=" << to_string(frame.checksumModel)
        << ", dl=" << static_cast<uint16_t>(frame.dataLength)
        << ", d={" << std::hex << std::setfill('0')
        << std::setw(2) << static_cast<uint16_t>(frame.data[0]) << " "
        << std::setw(2) << static_cast<uint16_t>(frame.data[1]) << " "
        << std::setw(2) << static_cast<uint16_t>(frame.data[2]) << " "
        << std::setw(2) << static_cast<uint16_t>(frame.data[3]) << " "
        << std::setw(2) << static_cast<uint16_t>(frame.data[4]) << " "
        << std::setw(2) << static_cast<uint16_t>(frame.data[5]) << " "
        << std::setw(2) << static_cast<uint16_t>(frame.data[6]) << " "
        << std::setw(2) << static_cast<uint16_t>(frame.data[7]) << "}}";
    out.copyfmt(oldState);
    return out;
}
    
} // namespace lin
} // namespace sim
} // namespace ib
