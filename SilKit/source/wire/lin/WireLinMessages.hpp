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

#include "silkit/services/lin/LinDatatypes.hpp"
#include "silkit/services/lin/string_utils.hpp"
#include "silkit/experimental/services/lin/LinDatatypesExtensions.hpp"

#include "SharedVector.hpp"

#include <chrono>
#include <vector>

namespace SilKit {
namespace Services {
namespace Lin {

//! \brief Data type representing a finished LIN transmission, independent of success or error.
struct LinTransmission
{
    std::chrono::nanoseconds timestamp; //!< Time at the end of the transmission. Only valid in detailed simulation.
    LinFrame frame; //!< The transmitted frame
    LinFrameStatus status; //!< The status of the transmitted frame
};

/*! \brief Data type representing a request to perform an AUTOSAR SendFrame operation.
 *
 * Used internally.
 */
struct LinSendFrameRequest
{
    LinFrame frame; //!< Provide the LIN ID, checksum model, expected data length and optional data.
    LinFrameResponseType responseType; //!< Determines whether to provide a frame response or not.
};

//! \brief Data type representing a LIN WakeUp pulse.
struct LinWakeupPulse
{
    std::chrono::nanoseconds timestamp; //!< Time of the WakeUp pulse. Only valid in detailed Simulation.
    TransmitDirection direction; //!< The direction of the wakeup pulse.
};

//! \brief Data type used to inform other LIN participants (LIN controllers and detailed Simulator) about changed LinControllerStatus.
struct LinControllerStatusUpdate
{
    std::chrono::nanoseconds timestamp; //!< Time of the controller status change.
    LinControllerStatus status; //!< The new controller status
};

//! \brief Data type used to inform other LIN participants about changed LinFrameResponse data.
struct LinFrameResponseUpdate
{
    std::vector<LinFrameResponse> frameResponses; //!< Vector of new FrameResponses.
};


//! \brief Data type for configuring a LinController. Contains members for DynamicResponse and Default simulationmode.
struct WireLinControllerConfig
{
    //! Used to configure the simulation mode of the LinController.
    enum class SimulationMode : uint8_t
    {
        //! The LIN controller sets frame responses using SetFrameResponses in advance.
        Default = 0,
        //! The LIN controller does not send frame responses automatically, users must call SendDynamicResponse.
        Dynamic = 1,
    };

    //! Configure as LIN master or LIN slave
    LinControllerMode controllerMode{LinControllerMode::Inactive};
    //! The operational baud rate of the controller. Used in a detailed simulation.
    LinBaudRate baudRate{0};
    //! Optional LinFrameResponse configuration.
    //!
    //! FrameResponses can also be configured at a later point using
    //! ILinController::UpdateTxBuffer() and
    //! ILinController::SetFrameResponses().
    std::vector<LinFrameResponse> frameResponses;

    //! The LinController's simulation mode.
    SimulationMode simulationMode{SimulationMode::Default};
};

inline bool operator==(const LinTransmission& lhs, const LinTransmission& rhs);
inline bool operator==(const LinSendFrameRequest& lhs, const LinSendFrameRequest& rhs);
inline bool operator==(const LinSendFrameHeaderRequest& lhs, const LinSendFrameHeaderRequest& rhs);
inline bool operator==(const LinWakeupPulse& lhs, const LinWakeupPulse& rhs);
inline bool operator==(const LinControllerStatusUpdate& lhs, const LinControllerStatusUpdate& rhs);
inline bool operator==(const LinFrameResponseUpdate& lhs, const LinFrameResponseUpdate& rhs);
inline bool operator==(const WireLinControllerConfig& lhs, const WireLinControllerConfig& rhs);

inline std::string to_string(const LinTransmission& transmission);
inline std::string to_string(const LinSendFrameRequest& request);
inline std::string to_string(const LinSendFrameHeaderRequest& request);
inline std::string to_string(const LinWakeupPulse& pulse);
inline std::string to_string(const LinControllerStatusUpdate& controllerStatusUpdate);
inline std::string to_string(const LinFrameResponseUpdate& frameResponseUpdate);
inline std::string to_string(const WireLinControllerConfig& frameResponseUpdate);

inline std::ostream& operator<<(std::ostream& out, const LinTransmission& transmission);
inline std::ostream& operator<<(std::ostream& out, const LinSendFrameRequest& request);
inline std::ostream& operator<<(std::ostream& out, const LinSendFrameHeaderRequest& request);
inline std::ostream& operator<<(std::ostream& out, const LinWakeupPulse& pulse);
inline std::ostream& operator<<(std::ostream& out, const LinControllerStatusUpdate& controllerStatusUpdate);
inline std::ostream& operator<<(std::ostream& out, const LinFrameResponseUpdate& frameResponseUpdate);
inline std::ostream& operator<<(std::ostream& out, const WireLinControllerConfig& frameResponseUpdate);

// ================================================================================
//  Inline Implementations
// ================================================================================

//! \brief operator== for LinTransmission
bool operator==(const LinTransmission& lhs, const LinTransmission& rhs)
{
    return lhs.timestamp == rhs.timestamp && lhs.frame == rhs.frame && lhs.status == rhs.status;
}

//! \brief operator== for LinSendFrameRequest
bool operator==(const LinSendFrameRequest& lhs, const LinSendFrameRequest& rhs)
{
    return lhs.frame == rhs.frame && lhs.responseType == rhs.responseType;
}

//! \brief operator== for LinSendFrameHeaderRequest
bool operator==(const LinSendFrameHeaderRequest& lhs, const LinSendFrameHeaderRequest& rhs)
{
    return lhs.id == rhs.id;
}

//! \brief operator== for LinWakeupPulse
bool operator==(const LinWakeupPulse& lhs, const LinWakeupPulse& rhs)
{
    return lhs.timestamp == rhs.timestamp;
}

//! \brief operator== for LinControllerStatusUpdate
bool operator==(const LinControllerStatusUpdate& lhs, const LinControllerStatusUpdate& rhs)
{
    return lhs.timestamp == rhs.timestamp && lhs.status == rhs.status;
}

//! \brief operator== for LinFrameResponseUpdate
bool operator==(const LinFrameResponseUpdate& lhs, const LinFrameResponseUpdate& rhs)
{
    return lhs.frameResponses == rhs.frameResponses;
}

//! \brief operator== for WireLinControllerConfig
bool operator==(const WireLinControllerConfig& lhs, const WireLinControllerConfig& rhs)
{
    return lhs.baudRate == rhs.baudRate
        && lhs.controllerMode == rhs.controllerMode
        && lhs.frameResponses == rhs.frameResponses
        && lhs.simulationMode == rhs.simulationMode
        ;
}

std::ostream& operator<<(std::ostream& out, const LinTransmission& transmission)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(transmission.timestamp);
    return out << "lin::LinTransmission{" << transmission.frame << ", status=" << transmission.status
               << ", time=" << timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, const LinSendFrameRequest& request)
{
    return out << "lin::LinSendFrameRequest{fr=" << request.frame << ", rt=" << request.responseType << "}";
}

std::ostream& operator<<(std::ostream& out, const LinSendFrameHeaderRequest& request)
{
    return out << "lin::LinSendFrameHeaderRequest{id=" << static_cast<uint16_t>(request.id) << "}";
}

std::ostream& operator<<(std::ostream& out, const LinWakeupPulse& pulse)
{
    return out << "lin::LinWakeupPulse{@" << pulse.timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, const LinControllerStatusUpdate& controllerStatusUpdate)
{
    auto timestamp =
        std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(controllerStatusUpdate.timestamp);
    return out << "lin::LinControllerStatusUpdate{" << controllerStatusUpdate.status << " @" << timestamp.count()
               << "ms}";
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
            out << "," << static_cast<uint16_t>(responses[i].frame.id);
        }
    }

    out << "]}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const WireLinControllerConfig& config)
{
    out << "lin::WireLinControllerConfig{" << config.baudRate << ", "
        << static_cast<int>(config.controllerMode) << ", "
        << static_cast<int>(config.simulationMode) << ", "
        ;
    out << "{";
    for (auto&& response: config.frameResponses)
    {
        out << "," << static_cast<uint16_t>(response.frame.id);
    }
    out << "}";

    return out;
}

std::string to_string(const LinTransmission& transmission)
{
    std::stringstream out;
    out << transmission;
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

std::string to_string(const LinWakeupPulse& pulse)
{
    std::stringstream out;
    out << pulse;
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

std::string to_string(const WireLinControllerConfig& config)
{
    std::stringstream out;
    out << config;
    return out.str();
}

} // namespace Lin
} // namespace Services
} // namespace SilKit
