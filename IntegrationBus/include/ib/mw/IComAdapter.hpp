// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <functional>

#include "ib/mw/fwd_decl.hpp"
#include "ib/cfg/fwd_decl.hpp"
#include "ib/sim/fwd_decl.hpp"

namespace ib {
namespace mw {

/*! \brief Communication interface to be used by IB participants
 *
 */
class IComAdapter
{
public:
    virtual ~IComAdapter() = default;

    /* Methods Create*Controller() create controllers at this IB participant.
     *
     * Controllers provide an easy interface to interact with a simulated bus. They
     * act as a proxy to the controller implementation in a Network Simulator connected
     * to the Integration Bus.
     *
     * Each Create*Controller() method creates a proxy instance, sets up all
     * necessary data structures and establishes the connection according to the
     * underlying middleware.
     */

     //! \brief Create a CAN controller at this IB participant.
    virtual auto CreateCanController(const std::string& canonicalName) -> sim::can::ICanController* = 0;
    //! \brief Create an Ethernet controller at this IB participant.
    virtual auto CreateEthController(const std::string& canonicalName) -> sim::eth::IEthController* = 0;
    //! \brief Create an Ethernet controller at this IB participant.
    virtual auto CreateFlexrayController(const std::string& canonicalName) -> sim::fr::IFrController* = 0;
    //! \brief Create a LIN controller at this IB participant.
    virtual auto CreateLinController(const std::string& canonicalName) -> sim::lin::ILinController* = 0;

    //! \brief Create an analog input port at this IB participant.
    virtual auto CreateAnalogIn(const std::string& canonicalName) -> sim::io::IAnalogInPort* = 0;
    //! \brief Create an digital input port at this IB participant.
    virtual auto CreateDigitalIn(const std::string& canonicalName) -> sim::io::IDigitalInPort* = 0;
    //! \brief Create a PWM input port at this IB participant.
    virtual auto CreatePwmIn(const std::string& canonicalName) -> sim::io::IPwmInPort* = 0;
    //! \brief Create a pattern input port at this IB participant.
    virtual auto CreatePatternIn(const std::string& canonicalName) -> sim::io::IPatternInPort* = 0;

    //! \brief Create an analog output port at this IB participant.
    virtual auto CreateAnalogOut(const std::string& canonicalName) -> sim::io::IAnalogOutPort* = 0;
    //! \brief Create a digital output port at this IB participant.
    virtual auto CreateDigitalOut(const std::string& canonicalName) -> sim::io::IDigitalOutPort* = 0;
    //! \brief Create a PWM output port at this IB participant.
    virtual auto CreatePwmOut(const std::string& canonicalName) -> sim::io::IPwmOutPort* = 0;
    //! \brief Create a pattern output port at this IB participant.
    virtual auto CreatePatternOut(const std::string& canonicalName) -> sim::io::IPatternOutPort* = 0;
    //! \brief Create a generic message publisher at this IB participant.
    virtual auto CreateGenericPublisher(const std::string& canonicalName) -> sim::generic::IGenericPublisher* = 0;
    //! \brief Create a generic message subscriber at this IB participant.
    virtual auto CreateGenericSubscriber(const std::string& canonicalName) -> sim::generic::IGenericSubscriber* = 0;

    virtual auto GetSyncMaster() -> sync::ISyncMaster* = 0;
    //! \brief Return the  IParticipantController for the current participant.
    virtual auto GetParticipantController() -> sync::IParticipantController* = 0;
    virtual auto GetSystemMonitor() -> sync::ISystemMonitor* = 0;
    virtual auto GetSystemController() -> sync::ISystemController* = 0;
    virtual auto GetLogger() -> logging::ILogger* = 0;
};

} // mw
} // namespace ib
