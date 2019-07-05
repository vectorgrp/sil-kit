// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <cassert>
#include <sstream>

#include "CanController.hpp"
#include "CanControllerProxy.hpp"
#include "EthController.hpp"
#include "EthControllerProxy.hpp"
#include "FrController.hpp"
#include "FrControllerProxy.hpp"
#include "LinController.hpp"
#include "LinControllerProxy.hpp"
#include "InPort.hpp"
#include "OutPort.hpp"
#include "GenericPublisher.hpp"
#include "GenericSubscriber.hpp"
#include "ParticipantController.hpp"
#include "SystemController.hpp"
#include "SystemMonitor.hpp"
#include "SyncMaster.hpp"
#include "LogmsgRouter.hpp"

#include "tuple_tools/bind.hpp"
#include "tuple_tools/for_each.hpp"
#include "tuple_tools/predicative_get.hpp"

#include "ib/cfg/string_utils.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/null_sink.h"


namespace ib {
namespace mw {

using namespace ib::sim;

namespace tt = util::tuple_tools;

// Anonymous namespace for Helper Traits and Functions
namespace {

    template<class T, class U>
    struct IsControllerMap : std::false_type {};
    template<class T, class U>
    struct IsControllerMap<std::unordered_map<EndpointId, std::unique_ptr<T>>, U> : std::is_base_of<T, U> {};

} // namespace anonymous

template <class IbConnectionT>
ComAdapter<IbConnectionT>::ComAdapter(cfg::Config config, const std::string& participantName)
    : _config{std::move(config)}
    , _participantName(participantName)
    , _logger{spdlog::create<spdlog::sinks::null_sink_st>(_participantName)}
    , _ibConnection(_config, participantName)
{
    // FIXME: move to initialize list
    _participant = &get_by_name(_config.simulationSetup.participants, participantName);
    _participantId = _participant->id;

    // we immediately drop the logger from the spdlog registry, because this cannot be controlled
    // by a user of the IntegrationBus.dll(!), which can have strange side effects, e.g., a stale
    // but logger even after the ComAdapter was destroyed. A user of the IntegrationBus.dll can
    // easily add the Logger to the global registry again so that it is under his control.
    spdlog::drop(_participantName);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::joinIbDomain(uint32_t domainId)
{
    _ibConnection.JoinDomain(domainId);
    onIbDomainJoined();
    _logger->info("Participant {} has joined the IB-Domain {}", _participantName, domainId);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::onIbDomainJoined()
{
    if (_participant->isSyncMaster)
    {
        /*[[maybe_unused]]*/ auto* controller = GetSyncMaster();
        (void)controller;
    }

    auto&& logMsgRouter = CreateController<logging::LogmsgRouter>(1028, "default");
    logMsgRouter->SetLogger(_logger);
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateCanController(const std::string& canonicalName) -> can::ICanController*
{
    assert(_participant);

    auto&& config = get_by_name(_participant->canControllers, canonicalName);

    if (ControllerUsesNetworkSimulator(config.name))
    {
        return CreateControllerForLink<can::CanControllerProxy>(config);
    }
    else
    {
        return CreateControllerForLink<can::CanController>(config);
    }
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateEthController(const std::string& canonicalName) -> eth::IEthController*
{
    assert(_participant);

    auto&& config = get_by_name(_participant->ethernetControllers, canonicalName);
    if (ControllerUsesNetworkSimulator(config.name))
    {
        return CreateControllerForLink<eth::EthControllerProxy>(config);
    }
    else
    {
        return CreateControllerForLink<eth::EthController>(config);
    }
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateFlexrayController(const std::string& canonicalName) -> sim::fr::IFrController*
{
    assert(_participant);

    auto&& config = get_by_name(_participant->flexrayControllers, canonicalName);
    if (ControllerUsesNetworkSimulator(config.name))
    {
        return CreateControllerForLink<fr::FrControllerProxy>(config);
    }
    else
    {
        return CreateControllerForLink<fr::FrController>(config);
    }
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateLinController(const std::string& canonicalName) -> lin::ILinController*
{
    assert(_participant);

    auto&& config = get_by_name(_participant->linControllers, canonicalName);
    if (ControllerUsesNetworkSimulator(config.name))
    {
        return CreateControllerForLink<lin::LinControllerProxy>(config);
    }
    else
    {
        return CreateControllerForLink<lin::LinController>(config);
    }
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateAnalogIn(const std::string& canonicalName) -> sim::io::IAnalogInPort*
{
    auto&& config = get_by_name(_participant->analogIoPorts, canonicalName);
    return CreateInPort<io::AnalogIoMessage>(config);
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateDigitalIn(const std::string& canonicalName) -> sim::io::IDigitalInPort*
{
    auto&& config = get_by_name(_participant->digitalIoPorts, canonicalName);
    return CreateInPort<io::DigitalIoMessage>(config);
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreatePwmIn(const std::string& canonicalName) -> sim::io::IPwmInPort*
{
    auto&& config = get_by_name(_participant->pwmPorts, canonicalName);
    return CreateInPort<io::PwmIoMessage>(config);
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreatePatternIn(const std::string& canonicalName) -> sim::io::IPatternInPort*
{
    auto&& config = get_by_name(_participant->patternPorts, canonicalName);
    return CreateInPort<io::PatternIoMessage>(config);
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateAnalogOut(const std::string& canonicalName) -> sim::io::IAnalogOutPort*
{
    auto&& config = get_by_name(_participant->analogIoPorts, canonicalName);
    return CreateOutPort<io::AnalogIoMessage>(config);
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateDigitalOut(const std::string& canonicalName) -> sim::io::IDigitalOutPort*
{
    auto&& config = get_by_name(_participant->digitalIoPorts, canonicalName);
    return CreateOutPort<io::DigitalIoMessage>(config);
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreatePwmOut(const std::string& canonicalName) -> sim::io::IPwmOutPort*
{
    auto&& config = get_by_name(_participant->pwmPorts, canonicalName);
    return CreateOutPort<io::PwmIoMessage>(config);
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreatePatternOut(const std::string& canonicalName) -> sim::io::IPatternOutPort*
{
    auto&& config = get_by_name(_participant->patternPorts, canonicalName);
    return CreateOutPort<io::PatternIoMessage>(config);
}

template <class IbConnectionT>
template <class MsgT, class ConfigT>
auto ComAdapter<IbConnectionT>::CreateInPort(const ConfigT& config) -> io::IInPort<MsgT>*
{
    if (config.direction != cfg::PortDirection::In)
        throw std::runtime_error("Invalid port direction!");

    return CreateControllerForLink<io::InPort<MsgT>>(config, config);
}

template <class IbConnectionT>
template <class MsgT, class ConfigT>
auto ComAdapter<IbConnectionT>::CreateOutPort(const ConfigT& config) -> io::IOutPort<MsgT>*
{
    if (config.direction != cfg::PortDirection::Out)
        throw std::runtime_error("Invalid port direction!");

    auto port = CreateControllerForLink<io::OutPort<MsgT>>(config, config);
    port->Write(config.initvalue, std::chrono::nanoseconds{0});

    return port;
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateGenericPublisher(const std::string& canonicalName) -> sim::generic::IGenericPublisher*
{
    auto&& config = get_by_name(_participant->genericPublishers, canonicalName);
    return CreateControllerForLink<sim::generic::GenericPublisher>(config, config);
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateGenericSubscriber(const std::string& canonicalName) -> sim::generic::IGenericSubscriber*
{
    auto&& config = get_by_name(_participant->genericSubscribers, canonicalName);
    return CreateControllerForLink<sim::generic::GenericSubscriber>(config, config);
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::GetSyncMaster() -> sync::ISyncMaster*
{
    if (!isSyncMaster())
    {
        _logger->error("ComAdapter::GetSyncMaster(): Participant is not configured as SyncMaster!");
        throw std::runtime_error("Participant not configured as SyncMaster");
    }

    auto* controller = GetController<sync::SyncMaster>(1027);
    if (!controller)
    {
        auto* systemMonitor = GetSystemMonitor();
        controller = CreateController<sync::SyncMaster>(1027, "default", _config, systemMonitor);
    }
    return controller;
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::GetParticipantController() -> sync::IParticipantController*
{
    auto* controller = GetController<sync::ParticipantController>(1024);
    if (!controller)
    {
        controller = CreateController<sync::ParticipantController>(1024, "default", *_participant, _config.simulationSetup.timeSync);
        RegisterNewPeerCallback([controller]() { controller->RefreshStatus(); });
    }
    return controller;
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::GetSystemMonitor() -> sync::ISystemMonitor*
{
    auto* controller = GetController<sync::SystemMonitor>(1025);
    if (!controller)
    {
        controller = CreateController<sync::SystemMonitor>(1025, "default", _config.simulationSetup);
    }
    return controller;
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::GetSystemController() -> sync::ISystemController*
{
    auto* controller = GetController<sync::SystemController>(1026);
    if (!controller)
    {
        return CreateController<sync::SystemController>(1026, "default");
    }
    return controller;
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::GetLogger() -> std::shared_ptr<spdlog::logger>&
{
    return _logger;
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::RegisterCanSimulator(can::IIbToCanSimulator* busSim)
{
    RegisterSimulator(busSim, cfg::Link::Type::CAN);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::RegisterEthSimulator(sim::eth::IIbToEthSimulator* busSim)
{
    RegisterSimulator(busSim, cfg::Link::Type::Ethernet);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::RegisterFlexraySimulator(sim::fr::IIbToFrBusSimulator* busSim)
{
    RegisterSimulator(busSim, cfg::Link::Type::FlexRay);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::RegisterLinSimulator(sim::lin::IIbToLinSimulator* busSim)
{
    RegisterSimulator(busSim, cfg::Link::Type::LIN);
}


template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const can::CanMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, can::CanMessage&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const can::CanTransmitAcknowledge& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const can::CanControllerStatus& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const can::CanConfigureBaudrate& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const can::CanSetControllerMode& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const eth::EthMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, eth::EthMessage&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const eth::EthTransmitAcknowledge& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const eth::EthStatus& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const eth::EthSetMode& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::fr::FrMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, sim::fr::FrMessage&& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::fr::FrMessageAck& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, sim::fr::FrMessageAck&& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::fr::FrSymbol& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::fr::FrSymbolAck& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::fr::CycleStart& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::fr::HostCommand& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::fr::ControllerConfig& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::fr::TxBufferUpdate& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::fr::ControllerStatus& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::lin::LinMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::lin::RxRequest& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::lin::TxAcknowledge& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::lin::WakeupRequest& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::lin::ControllerConfig& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::lin::SlaveConfiguration& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::lin::SlaveResponse& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::io::AnalogIoMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::io::DigitalIoMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::io::PatternIoMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, sim::io::PatternIoMessage&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::io::PwmIoMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sim::generic::GenericMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, sim::generic::GenericMessage&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sync::Tick& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sync::TickDone& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sync::QuantumRequest& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sync::QuantumGrant& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sync::ParticipantStatus& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sync::ParticipantCommand& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const sync::SystemCommand& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, const logging::LogMsg& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(EndpointAddress from, logging::LogMsg&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
template <typename IbMessageT>
void ComAdapter<IbConnectionT>::SendIbMessageImpl(EndpointAddress from, IbMessageT&& msg)
{
    _ibConnection.SendIbMessageImpl(from, std::forward<IbMessageT>(msg));
}

template <class IbConnectionT>
template <class ControllerT>
auto ComAdapter<IbConnectionT>::GetController(EndpointId endpointId) -> ControllerT*
{
    auto&& controllerMap = tt::predicative_get<tt::rbind<IsControllerMap, ControllerT>::template type>(_controllers);
    if (controllerMap.count(endpointId))
    {
        return static_cast<ControllerT*>(controllerMap.at(endpointId).get());
    }
    else
    {
        return nullptr;
    }
}

template <class IbConnectionT>
template <class ControllerT, typename... Arg>
auto ComAdapter<IbConnectionT>::CreateController(EndpointId endpointId, const std::string& topicname, Arg&&... arg) -> ControllerT*
{
    auto&& controllerMap = tt::predicative_get<tt::rbind<IsControllerMap, ControllerT>::template type>(_controllers);
    if (controllerMap.count(endpointId))
    {
        _logger->error("ComAdapter already has a controller with endpointId={}", endpointId);
        throw std::runtime_error("Duplicate EndpointId");
    }

    auto controller = std::make_unique<ControllerT>(this, std::forward<Arg>(arg)...);
    auto controllerPtr = controller.get();
    controller->SetEndpointAddress(EndpointAddress{_participantId, endpointId});

    _ibConnection.RegisterIbService(topicname, endpointId, controllerPtr);

    controllerMap[endpointId] = std::move(controller);
    return controllerPtr;
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::GetLinkById(int16_t linkId) -> cfg::Link&
{
    for (auto&& link : _config.simulationSetup.links)
    {
        if (link.id == linkId)
            return link;
    }

    throw cfg::Misconfiguration("Invalid linkId " + std::to_string(linkId));
}

template <class IbConnectionT>
template <class ControllerT, class ConfigT, typename... Arg>
auto ComAdapter<IbConnectionT>::CreateControllerForLink(const ConfigT& config, Arg&&... arg) -> ControllerT*
{
    auto&& linkCfg = GetLinkById(config.linkId);
    return CreateController<ControllerT>(config.endpointId, linkCfg.name, std::forward<Arg>(arg)...);
}


template <class IbConnectionT>
template <class IIbToSimulatorT>
void ComAdapter<IbConnectionT>::RegisterSimulator(IIbToSimulatorT* busSim, cfg::Link::Type linkType)
{
    auto&& simulator = std::get<IIbToSimulatorT*>(_simulators);
    if (simulator)
    {
        _logger->error("A {} is already registered", typeid(IIbToSimulatorT).name());
        return;
    }

    std::unordered_map<std::string, mw::EndpointId> endpointMap;
    auto addToEndpointMap = [&endpointMap](auto&& participantName, auto&& controllerConfigs)
    {
        for (auto&& cfg : controllerConfigs)
        {
            std::string qualifiedName = participantName + "/" + cfg.name;
            endpointMap[qualifiedName] = cfg.endpointId;
        }
    };

    for (auto&& participant : _config.simulationSetup.participants)
    {
        addToEndpointMap(participant.name, participant.canControllers);
        addToEndpointMap(participant.name, participant.linControllers);
        addToEndpointMap(participant.name, participant.ethernetControllers);
        addToEndpointMap(participant.name, participant.flexrayControllers);
    }
    for (auto&& ethSwitch : _config.simulationSetup.switches)
    {
        addToEndpointMap(ethSwitch.name, ethSwitch.ports);
    }

    // get_by_name throws if the current node is not configured as a network simulator.
    for (auto&& simulatorName : _participant->networkSimulators)
    {
        auto&& simulatorConfig = get_by_name(_config.simulationSetup.networkSimulators, simulatorName);

        for (auto&& linkName : simulatorConfig.simulatedLinks)
        {
            auto&& linkConfig = get_by_name(_config.simulationSetup.links, linkName);

            if (linkConfig.type != linkType)
                continue;

            for (auto&& endpointName : linkConfig.endpoints)
            {
                try
                {
                    auto proxyEndpoint = endpointMap.at(endpointName);
                    _ibConnection.RegisterIbService(linkName, proxyEndpoint, busSim);
                }
                catch (const std::exception& e)
                {
                    _logger->error("Cannot register simulator topics for link \"{}\": {}", linkName, e.what());
                    continue;
                }
            }
        }
    }

    simulator = busSim;
}

template <class IbConnectionT>
bool ComAdapter<IbConnectionT>::ControllerUsesNetworkSimulator(const std::string& controllerName) const
{
    auto endpointName = _participantName + "/" + controllerName;

    for (auto&& link : _config.simulationSetup.links)
    {
        auto endpointIter = std::find(link.endpoints.begin(), link.endpoints.end(), endpointName);

        if (endpointIter == link.endpoints.end())
            continue;

        for (auto&& simulator : _config.simulationSetup.networkSimulators)
        {
            auto linkIter = std::find(simulator.simulatedLinks.begin(), simulator.simulatedLinks.end(), link.name);
            if (linkIter != simulator.simulatedLinks.end())
                return true;
        }
    }

    return false;
}

template <class IbConnectionT>
bool ComAdapter<IbConnectionT>::isSyncMaster() const
{
    assert(_participant);

    return _participant->isSyncMaster;
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::OnAllMessagesDelivered(std::function<void()> callback)
{
    _ibConnection.OnAllMessagesDelivered(std::move(callback));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::FlushSendBuffers()
{
    _ibConnection.FlushSendBuffers();
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::RegisterNewPeerCallback(std::function<void()> callback)
{
    _ibConnection.RegisterNewPeerCallback(callback);
}

} // namespace mw
} // namespace ib
