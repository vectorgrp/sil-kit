// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IComAdapterInternal.hpp"

#include <memory>
#include <vector>
#include <unordered_map>
#include <tuple>

#include "ib/cfg/Config.hpp"
#include "ib/mw/all.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/logging/ILogger.hpp"

#include "ib/extensions/ITraceMessageSink.hpp"
#include "ib/extensions/ITraceMessageSource.hpp"

#include "Tracing.hpp"
#include "ReplayScheduler.hpp"

// Interfaces relying on IbInternal
#include "IIbToLogMsgSender.hpp"
#include "IIbToLogMsgReceiver.hpp"

#include "IIbToCanSimulator.hpp"
#include "IIbToCanController.hpp"
#include "IIbToCanControllerProxy.hpp"
#include "IIbToCanControllerFacade.hpp"

#include "IIbToEthSimulator.hpp"
#include "IIbToEthController.hpp"
#include "IIbToEthControllerProxy.hpp"
#include "IIbToEthControllerFacade.hpp"

#include "IIbToLinSimulator.hpp"
#include "IIbToLinController.hpp"
#include "IIbToLinControllerProxy.hpp"
#include "IIbToLinControllerFacade.hpp"

#include "IIbToFrBusSimulator.hpp"
#include "IIbToFrController.hpp"
#include "IIbToFrControllerProxy.hpp"
#include "IIbToFrControllerFacade.hpp"

#include "IIbToInPort.hpp"
#include "IIbToOutPort.hpp"

#include "IIbToGenericSubscriber.hpp"
#include "IIbToGenericPublisher.hpp"

#include "IIbToDataPublisher.hpp"
#include "IIbToDataSubscriber.hpp"
#include "IIbToDataSubscriberInternal.hpp"

#include "IIbToSystemMonitor.hpp"
#include "IIbToSystemController.hpp"
#include "IIbToParticipantController.hpp"

// IbMwService
#include "ServiceDiscovery.hpp"

// Add connection types here and make sure they are instantiated in ComAdapter.cpp
#if defined(IB_MW_HAVE_VASIO)
#   include "VAsioConnection.hpp"
#endif


namespace ib {
namespace mw {

template <class IbConnectionT>
class ComAdapter : public IComAdapterInternal
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
    auto CreateDataPublisher(const std::string& canonicalName, const sim::data::DataExchangeFormat& dataExchangeFormat, 
        size_t history = 0)->sim::data::IDataPublisher* override;
    auto CreateDataSubscriber(const std::string& canonicalName, const sim::data::DataExchangeFormat& dataExchangeFormat, 
        sim::data::CallbackExchangeFormatT callback)->sim::data::IDataSubscriber* override;
    auto CreateDataSubscriberInternal(const std::string& canonicalName, const std::string& linkName, 
        const sim::data::DataExchangeFormat& dataExchangeFormat, sim::data::CallbackExchangeFormatT callback)
        ->sim::data::IDataSubscriber* override;

    auto GetParticipantController() -> sync::IParticipantController* override;
    auto GetSystemMonitor() -> sync::ISystemMonitor* override;
    auto GetSystemController() -> sync::ISystemController* override;
    auto GetServiceDiscovery() -> service::IServiceDiscovery* override;
    auto GetLogger() -> logging::ILogger* override;
    auto GetParticipantName() const -> const std::string& override { return _participantName; }
    auto GetConfig() const -> const ib::cfg::Config& override { return _config; }

    void RegisterCanSimulator(sim::can::IIbToCanSimulator* busSim) override;
    void RegisterEthSimulator(sim::eth::IIbToEthSimulator* busSim) override;
    void RegisterFlexraySimulator(sim::fr::IIbToFrBusSimulator* busSim) override;
    void RegisterLinSimulator(sim::lin::IIbToLinSimulator* busSim) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const sim::can::CanMessage& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, sim::can::CanMessage&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::can::CanTransmitAcknowledge& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::can::CanControllerStatus& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::can::CanConfigureBaudrate& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::can::CanSetControllerMode& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const sim::eth::EthMessage& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, sim::eth::EthMessage&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::eth::EthTransmitAcknowledge& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::eth::EthStatus& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::eth::EthSetMode& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FrMessage& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, sim::fr::FrMessage&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FrMessageAck& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, sim::fr::FrMessageAck&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FrSymbol& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FrSymbolAck& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::CycleStart& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::HostCommand& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::ControllerConfig& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::TxBufferConfigUpdate& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::TxBufferUpdate& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::ControllerStatus& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::PocStatus& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::SendFrameRequest& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::SendFrameHeaderRequest& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::Transmission& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::WakeupPulse& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::ControllerConfig& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::ControllerStatusUpdate& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::FrameResponseUpdate& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const sim::io::AnalogIoMessage& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::io::DigitalIoMessage& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::io::PatternIoMessage& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, sim::io::PatternIoMessage&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::io::PwmIoMessage& msg) override;

    void SendIbMessage(const IIbServiceEndpoint*, const sync::NextSimTask& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const sync::ParticipantStatus& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const sync::ParticipantCommand& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const sync::SystemCommand& msg) override;

    void SendIbMessage(const IIbServiceEndpoint*, const logging::LogMsg& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, logging::LogMsg&& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const sim::generic::GenericMessage& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, sim::generic::GenericMessage&& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const sim::data::DataMessage& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, sim::data::DataMessage&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::data::PublisherAnnouncement& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, sim::data::PublisherAnnouncement&& msg) override;


    void SendIbMessage(const IIbServiceEndpoint*, const service::ServiceAnnouncement& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const service::ServiceDiscoveryEvent& msg) override;

    // targeted messaging
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanMessage& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::can::CanMessage&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanTransmitAcknowledge& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanControllerStatus& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanConfigureBaudrate& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanSetControllerMode& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::eth::EthMessage& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::eth::EthMessage&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::eth::EthTransmitAcknowledge& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::eth::EthStatus& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::eth::EthSetMode& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FrMessage& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::fr::FrMessage&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FrMessageAck& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::fr::FrMessageAck&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FrSymbol& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FrSymbolAck& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::CycleStart& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::HostCommand& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::ControllerConfig& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::TxBufferConfigUpdate& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::TxBufferUpdate& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::ControllerStatus& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::PocStatus& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::SendFrameRequest& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::SendFrameHeaderRequest& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::Transmission& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::WakeupPulse& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::ControllerConfig& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::ControllerStatusUpdate& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::FrameResponseUpdate& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::io::AnalogIoMessage& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::io::DigitalIoMessage& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::io::PatternIoMessage& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::io::PatternIoMessage&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::io::PwmIoMessage& msg) override;

    void SendIbMessage(const IIbServiceEndpoint*, const std::string& targetParticipantName, const sync::NextSimTask& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const std::string& targetParticipantName, const sync::ParticipantStatus& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const std::string& targetParticipantName, const sync::ParticipantCommand& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const std::string& targetParticipantName, const sync::SystemCommand& msg) override;

    void SendIbMessage(const IIbServiceEndpoint*, const std::string& targetParticipantName, const logging::LogMsg& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const std::string& targetParticipantName, logging::LogMsg&& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::generic::GenericMessage& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::generic::GenericMessage&& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::data::DataMessage& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::data::DataMessage&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::data::PublisherAnnouncement& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::data::PublisherAnnouncement&& msg) override;

    void SendIbMessage(const IIbServiceEndpoint*, const std::string& targetParticipantName, const service::ServiceAnnouncement& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const std::string& targetParticipantName, const service::ServiceDiscoveryEvent& msg) override;
	
    void OnAllMessagesDelivered(std::function<void()> callback) override;
    void FlushSendBuffers() override;
    void ExecuteDeferred(std::function<void()> callback) override;

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
    void joinIbDomain(uint32_t domainId) override;

    // For Testing Purposes:
    inline auto GetIbConnection() -> IbConnectionT& { return _ibConnection; }

private:
    // ----------------------------------------
    // private datatypes
    template<typename ControllerT>
    using ControllerMap = std::unordered_map<std::string, std::unique_ptr<ControllerT>>;

private:
    // ----------------------------------------
    // private methods
    void onIbDomainJoined();

    void SetupRemoteLogging();

    void SetTimeProvider(sync::ITimeProvider*);

    template<class IbMessageT>
    void SendIbMessageImpl(const IIbServiceEndpoint* from, IbMessageT&& msg);
    template<class IbMessageT>
    void SendIbMessageImpl(const IIbServiceEndpoint* from, const std::string& targetParticipantName, IbMessageT&& msg);

    template<class MsgT, class ConfigT>
    auto CreateInPort(const ConfigT& config) -> sim::io::IInPort<MsgT>*;
    template<class MsgT, class ConfigT>
    auto CreateOutPort(const ConfigT& config) -> sim::io::IOutPort<MsgT>*;

    template<class ControllerT>
    auto GetController(const std::string& linkName, const std::string& serviceName) -> ControllerT*;
    template<class ControllerT, typename... Arg>
    auto CreateController(const cfg::Link& link, const std::string& serviceName, Arg&&... arg) -> ControllerT*;
    //!< internal services don't have a link config
    template<class ControllerT, typename... Arg>
    auto CreateController(const std::string& serviceName,  Arg&&... arg) -> ControllerT*;

    auto GetLinkById(int16_t linkId) -> cfg::Link&;

    template<class ControllerT, class ConfigT, typename... Arg>
    auto CreateControllerForLink(const ConfigT& config, Arg&&... arg) -> ControllerT*;

    template<class IIbToSimulatorT>
    void RegisterSimulator(IIbToSimulatorT* busSim, cfg::Link::Type linkType);

    bool ControllerUsesNetworkSimulator(const std::string& controllerName) const;
   
    template<class ConfigT>
    void AddTraceSinksToSource(extensions::ITraceMessageSource* controller, ConfigT config);

private:
    // ----------------------------------------
    // private members
    cfg::Config _config;
    const cfg::Participant& _participant;
    std::string _participantName;
    ParticipantId _participantId{0};
    std::shared_ptr<sync::ITimeProvider> _timeProvider{nullptr};

    std::unique_ptr<logging::ILogger> _logger;
    std::vector<std::unique_ptr<extensions::ITraceMessageSink>> _traceSinks;
    std::unique_ptr<tracing::ReplayScheduler> _replayScheduler;

    std::tuple<
        ControllerMap<sim::can::IIbToCanController>,
        ControllerMap<sim::can::IIbToCanControllerProxy>,
        ControllerMap<sim::can::IIbToCanControllerFacade>,
        ControllerMap<sim::eth::IIbToEthController>,
        ControllerMap<sim::eth::IIbToEthControllerProxy>,
        ControllerMap<sim::eth::IIbToEthControllerFacade>,
        ControllerMap<sim::fr::IIbToFrController>,
        ControllerMap<sim::fr::IIbToFrControllerProxy>,
        ControllerMap<sim::fr::IIbToFrControllerFacade>,
        ControllerMap<sim::lin::IIbToLinController>,
        ControllerMap<sim::lin::IIbToLinControllerProxy>,
        ControllerMap<sim::lin::IIbToLinControllerFacade>,
        ControllerMap<sim::generic::IIbToGenericPublisher>,
        ControllerMap<sim::generic::IIbToGenericSubscriber>,
        ControllerMap<sim::data::IIbToDataPublisher>,
        ControllerMap<sim::data::IIbToDataSubscriber>,
        ControllerMap<sim::data::IIbToDataSubscriberInternal>,
        ControllerMap<sim::io::IIbToInPort<sim::io::DigitalIoMessage>>,
        ControllerMap<sim::io::IIbToInPort<sim::io::AnalogIoMessage>>,
        ControllerMap<sim::io::IIbToInPort<sim::io::PwmIoMessage>>,
        ControllerMap<sim::io::IIbToInPort<sim::io::PatternIoMessage>>,
        ControllerMap<sim::io::IIbToOutPort<sim::io::DigitalIoMessage>>,
        ControllerMap<sim::io::IIbToOutPort<sim::io::AnalogIoMessage>>,
        ControllerMap<sim::io::IIbToOutPort<sim::io::PwmIoMessage>>,
        ControllerMap<sim::io::IIbToOutPort<sim::io::PatternIoMessage>>,
        ControllerMap<logging::IIbToLogMsgSender>,
        ControllerMap<logging::IIbToLogMsgReceiver>,
        ControllerMap<sync::IIbToParticipantController>,
        ControllerMap<sync::IIbToSystemMonitor>,
        ControllerMap<sync::IIbToSystemController>,
        ControllerMap<service::ServiceDiscovery>
    > _controllers;

    std::tuple<
        sim::can::IIbToCanSimulator*,
        sim::eth::IIbToEthSimulator*,
        sim::fr::IIbToFrBusSimulator*,
        sim::lin::IIbToLinSimulator*
    > _simulators {nullptr, nullptr, nullptr, nullptr};

    IbConnectionT _ibConnection;
};

inline auto GetParticipantByName(const cfg::Config& config, const std::string& participantName) -> const cfg::Participant&;

// ================================================================================
//  Inline Implementations
// ================================================================================
auto GetParticipantByName(const cfg::Config& config, const std::string& participantName) -> const cfg::Participant&
{
    if (participantName.size() == 0)
    {
        throw ib::cfg::Misconfiguration{"Cannot create a ComAdapter with empty name."};
    }
    try
    {
        return get_by_name(config.simulationSetup.participants, participantName);
    }
    catch (const ib::cfg::Misconfiguration&)
    {
        throw ib::cfg::Misconfiguration{"ParticipantName '" + participantName + "' does not exist in IbConfig{name='" + config.name + "'}"};
    }
}

} // mw
} // namespace ib

