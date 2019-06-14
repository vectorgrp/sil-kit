// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IComAdapter.hpp"

#include <memory>
#include <vector>
#include <unordered_map>
#include <tuple>

#include "ib/cfg/Config.hpp"
#include "ib/mw/all.hpp"
#include "ib/sim/all.hpp"

#include "ILogmsgRouter.hpp"
#include "IIbToLogmsgRouter.hpp"


// Add connection types here and make sure they are instantiated in ComAdapter.cpp
#include "FastRtpsConnection.hpp"


namespace ib {
namespace mw {

template <class IbConnectionT>
class ComAdapter : public IComAdapter
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    ComAdapter() = default;
    ComAdapter(const ComAdapter&) = default;
    ComAdapter(ComAdapter&&) = default;
    ComAdapter(cfg::Config config, const std::string& participantName);

public:
    // ----------------------------------------
    // Operator Implementations
    ComAdapter& operator=(ComAdapter& other) = default;
    ComAdapter& operator=(ComAdapter&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IComAdapter
    auto CreateCanController(const std::string& canonicalName) -> sim::can::ICanController* override;
    auto CreateEthController(const std::string& canonicalName) -> sim::eth::IEthController* override;
    auto CreateFlexrayController(const std::string& canonicalName) -> sim::fr::IFrController* override;
    auto CreateLinController(const std::string& canonicalName) -> sim::lin::ILinController* override;
    auto CreateAnalogIn(const std::string& canonicalName) -> sim::io::IAnalogInPort* override;
    auto CreateDigitalIn(const std::string& canonicalName) -> sim::io::IDigitalInPort* override;
    auto CreatePwmIn(const std::string& canonicalName) -> sim::io::IPwmInPort* override;
    auto CreatePatternIn(const std::string& canonicalName) -> sim::io::IPatternInPort* override;
    auto CreateAnalogOut(const std::string& canonicalName) -> sim::io::IAnalogOutPort* override;
    auto CreateDigitalOut(const std::string& canonicalName) -> sim::io::IDigitalOutPort* override;
    auto CreatePwmOut(const std::string& canonicalName) -> sim::io::IPwmOutPort* override;
    auto CreatePatternOut(const std::string& canonicalName) -> sim::io::IPatternOutPort* override;
    auto CreateGenericPublisher(const std::string& canonicalName) -> sim::generic::IGenericPublisher* override;
    auto CreateGenericSubscriber(const std::string& canonicalName) -> sim::generic::IGenericSubscriber* override;
    auto GetSyncMaster() -> sync::ISyncMaster* override;
    auto GetParticipantController() -> sync::IParticipantController* override;
    auto GetSystemMonitor() -> sync::ISystemMonitor* override;
    auto GetSystemController() -> sync::ISystemController* override;
    auto GetLogger() -> std::shared_ptr<spdlog::logger>& override;

    void RegisterCanSimulator(sim::can::IIbToCanSimulator* busSim) override;
    void RegisterEthSimulator(sim::eth::IIbToEthSimulator* busSim) override;
    void RegisterFlexraySimulator(sim::fr::IIbToFrBusSimulator* busSim) override;
    void RegisterLinSimulator(sim::lin::IIbToLinSimulator* busSim) override;

    void SendIbMessage(EndpointAddress from, const sim::can::CanMessage& msg) override;
    void SendIbMessage(EndpointAddress from, sim::can::CanMessage&& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::can::CanTransmitAcknowledge& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::can::CanControllerStatus& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::can::CanConfigureBaudrate& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::can::CanSetControllerMode& msg) override;

    void SendIbMessage(EndpointAddress from, const sim::eth::EthMessage& msg) override;
    void SendIbMessage(EndpointAddress from, sim::eth::EthMessage&& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::eth::EthTransmitAcknowledge& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::eth::EthStatus& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::eth::EthSetMode& msg) override;

    void SendIbMessage(EndpointAddress from, const sim::fr::FrMessage& msg) override;
    void SendIbMessage(EndpointAddress from, sim::fr::FrMessage&& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::fr::FrMessageAck& msg) override;
    void SendIbMessage(EndpointAddress from, sim::fr::FrMessageAck&& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::fr::FrSymbol& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::fr::FrSymbolAck& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::fr::CycleStart& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::fr::HostCommand& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::fr::ControllerConfig& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::fr::TxBufferUpdate& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::fr::ControllerStatus& msg) override;

    void SendIbMessage(EndpointAddress from, const sim::lin::LinMessage& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::lin::RxRequest& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::lin::TxAcknowledge& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::lin::WakeupRequest& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::lin::ControllerConfig& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::lin::SlaveConfiguration& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::lin::SlaveResponse& msg) override;

    void SendIbMessage(EndpointAddress from, const sim::io::AnalogIoMessage& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::io::DigitalIoMessage& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::io::PatternIoMessage& msg) override;
    void SendIbMessage(EndpointAddress from, sim::io::PatternIoMessage&& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::io::PwmIoMessage& msg) override;

    void SendIbMessage(EndpointAddress from, const sync::Tick& msg) override;
    void SendIbMessage(EndpointAddress from, const sync::TickDone& msg) override;
    void SendIbMessage(EndpointAddress from, const sync::QuantumRequest& msg) override;
    void SendIbMessage(EndpointAddress from, const sync::QuantumGrant& msg) override;
    void SendIbMessage(EndpointAddress from, const sync::ParticipantStatus& msg) override;
    void SendIbMessage(EndpointAddress from, const sync::ParticipantCommand& msg) override;
    void SendIbMessage(EndpointAddress from, const sync::SystemCommand& msg) override;

    void SendIbMessage(EndpointAddress from, const logging::LogMsg& msg) override;
    void SendIbMessage(EndpointAddress from, logging::LogMsg&& msg) override;

    void SendIbMessage(EndpointAddress from, const sim::generic::GenericMessage& msg) override;
    void SendIbMessage(EndpointAddress from, sim::generic::GenericMessage&& msg) override;

    void WaitForMessageDelivery() override;
    void FlushSendBuffers() override;

public:
    // ----------------------------------------
    // Public methods

    /*! \brief Join the middleware domain as a participant.
    *
    * Join the middleware domain and become a participant.
    * \param domainId ID of the domain
    *
    * \throw std::exception A participant was created previously, or a
    * participant could not be created.
    */
    void joinIbDomain(uint32_t domainId);

    void Run() override { _ibConnection.Run(); }
    void Stop() override { _ibConnection.Stop(); }

private:
    // ----------------------------------------
    // private datatypes
    template<typename ControllerT>
    using ControllerMap = std::unordered_map<EndpointId, std::unique_ptr<ControllerT>>;

private:
    // ----------------------------------------
    // private methods
    void onIbDomainJoined();

    template<typename IbMessageT>
    void SendIbMessageImpl(EndpointAddress from, IbMessageT&& msg);

    template<class MsgT, class ConfigT>
    auto CreateInPort(const ConfigT& config) -> sim::io::IInPort<MsgT>*;
    template<class MsgT, class ConfigT>
    auto CreateOutPort(const ConfigT& config) -> sim::io::IOutPort<MsgT>*;

    template<class ControllerT>
    auto GetController(EndpointId endpointId) -> ControllerT*;
    template<class ControllerT, typename... Arg>
    auto CreateController(EndpointId endpointId, const std::string& topicname, Arg&&... arg) -> ControllerT*;

    auto GetLinkById(int16_t linkId) -> cfg::Link&;

    template<class ControllerT, class ConfigT, typename... Arg>
    auto CreateControllerForLink(const ConfigT& config, Arg&&... arg) -> ControllerT*;

    template<class IIbToSimulatorT>
    void RegisterSimulator(IIbToSimulatorT* busSim, cfg::Link::Type linkType);

    bool ControllerUsesNetworkSimulator(const std::string& controllerName) const;
    bool isSyncMaster() const;

private:
    // ----------------------------------------
    // private members
    cfg::Config _config;
    const cfg::Participant* _participant{nullptr};
    std::string _participantName;
    ParticipantId _participantId{0};

    std::shared_ptr<spdlog::logger> _logger;

    std::tuple<
        ControllerMap<sim::can::IIbToCanController>,
        ControllerMap<sim::can::IIbToCanControllerProxy>,
        ControllerMap<sim::eth::IIbToEthController>,
        ControllerMap<sim::eth::IIbToEthControllerProxy>,
        ControllerMap<sim::fr::IIbToFrController>,
        ControllerMap<sim::fr::IIbToFrControllerProxy>,
        ControllerMap<sim::lin::IIbToLinController>,
        ControllerMap<sim::lin::IIbToLinControllerProxy>,
        ControllerMap<sim::generic::IIbToGenericPublisher>,
        ControllerMap<sim::generic::IIbToGenericSubscriber>,
        ControllerMap<sim::io::IIbToInPort<sim::io::DigitalIoMessage>>,
        ControllerMap<sim::io::IIbToInPort<sim::io::AnalogIoMessage>>,
        ControllerMap<sim::io::IIbToInPort<sim::io::PwmIoMessage>>,
        ControllerMap<sim::io::IIbToInPort<sim::io::PatternIoMessage>>,
        ControllerMap<sim::io::IIbToOutPort<sim::io::DigitalIoMessage>>,
        ControllerMap<sim::io::IIbToOutPort<sim::io::AnalogIoMessage>>,
        ControllerMap<sim::io::IIbToOutPort<sim::io::PwmIoMessage>>,
        ControllerMap<sim::io::IIbToOutPort<sim::io::PatternIoMessage>>,
        ControllerMap<logging::IIbToLogmsgRouter>,
        ControllerMap<sync::IIbToParticipantController>,
        ControllerMap<sync::IIbToSystemMonitor>,
        ControllerMap<sync::IIbToSystemController>,
        ControllerMap<sync::IIbToSyncMaster>
    > _controllers;

    std::tuple<
        sim::can::IIbToCanSimulator*,
        sim::eth::IIbToEthSimulator*,
        sim::fr::IIbToFrBusSimulator*,
        sim::lin::IIbToLinSimulator*
    > _simulators {nullptr, nullptr, nullptr, nullptr};

    IbConnectionT _ibConnection;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // mw
} // namespace ib

