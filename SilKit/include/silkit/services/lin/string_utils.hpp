/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include <iomanip>
#include <sstream>

#include "LinDatatypes.hpp"

#include "silkit/participant/exception.hpp"
#include "silkit/util/PrintableHexString.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

inline std::string to_string(LinChecksumModel model);
inline std::string to_string(LinFrameResponseType responseType);
inline std::string to_string(LinFrameResponseMode responseMode);
inline std::string to_string(LinFrameStatus frameStatus);
inline std::string to_string(LinControllerMode mode);
inline std::string to_string(LinControllerStatus status);

inline std::string to_string(const LinFrame& frame);
inline std::string to_string(const LinControllerConfig& controllerConfig);

inline std::ostream& operator<<(std::ostream& out, LinChecksumModel model);
inline std::ostream& operator<<(std::ostream& out, LinFrameResponseType responseType);
inline std::ostream& operator<<(std::ostream& out, LinFrameResponseMode responseMode);
inline std::ostream& operator<<(std::ostream& out, LinFrameStatus frameStatus);
inline std::ostream& operator<<(std::ostream& out, LinControllerMode mode);
inline std::ostream& operator<<(std::ostream& out, LinControllerStatus status);

inline std::ostream& operator<<(std::ostream& out, const LinFrame& frame);
inline std::ostream& operator<<(std::ostream& out, const LinControllerConfig& controllerConfig);

// ================================================================================
//  Inline Implementations
// ================================================================================

std::string to_string(LinChecksumModel model)
{
    switch (model)
    {
    case LinChecksumModel::Unknown:
        return "Undefined";
    case LinChecksumModel::Enhanced:
        return "Enhanced";
    case LinChecksumModel::Classic:
        return "Classic";
    }
    throw SilKit::TypeConversionError{};
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
    throw SilKit::TypeConversionError{};
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
    throw SilKit::TypeConversionError{};
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
    throw SilKit::TypeConversionError{};
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
    throw SilKit::TypeConversionError{};
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
    throw SilKit::TypeConversionError{};
}


std::string to_string(const LinFrame& frame)
{
    std::stringstream out;
    out << frame;
    return out.str();
}

std::string to_string(const LinControllerConfig& controllerConfig)
{
    std::stringstream out;
    out << controllerConfig;
    return out.str();
}

std::ostream& operator<<(std::ostream& out, LinChecksumModel model)
{
    try
    {
        return out << to_string(model);
    }
    catch (const SilKit::TypeConversionError&)
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
    catch (const SilKit::TypeConversionError&)
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
    catch (const SilKit::TypeConversionError&)
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
    catch (const SilKit::TypeConversionError&)
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
    catch (const SilKit::TypeConversionError&)
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
    catch (const SilKit::TypeConversionError&)
    {
        return out << "LinControllerStatus{" << static_cast<uint32_t>(status) << "}";
    }
}

std::ostream& operator<<(std::ostream& out, const LinFrame& frame)
{
    //instead of ios::copyfmt (which set badbit) we use a temporary stream
    std::stringstream buf;
    buf
        << "Lin::LinFrame{id=" << static_cast<uint16_t>(frame.id)
        << ", cs=" << to_string(frame.checksumModel)
        << ", dl=" << static_cast<uint16_t>(frame.dataLength)
        << ", d={" << Util::AsHexString(frame.data).WithSeparator(" ")
        << "}}";
    return out << buf.str();
}

std::ostream& operator<<(std::ostream& out, const LinControllerConfig& controllerConfig)
{
    out << "Lin::LinControllerConfig{br=" << controllerConfig.baudRate
        << ", mode=" << controllerConfig.controllerMode
        << ", responses=[";
    if (controllerConfig.frameResponses.size() > 0)
    {
        out << static_cast<uint16_t>(controllerConfig.frameResponses[0].frame.id);
        for (auto i = 1u; i < controllerConfig.frameResponses.size(); ++i)
        {
            out << "," << static_cast<uint16_t>(controllerConfig.frameResponses[i].frame.id);
        }
    }
    out << "]}";
    return out;
}

} // namespace Lin
} // namespace Services
} // namespace SilKit
