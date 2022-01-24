// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ParticipantBuilder.hpp"

#include <sstream>
#include <thread>
#include <algorithm>

#include "ib/cfg/SimulationSetupBuilder.hpp"

using namespace std::chrono_literals;

namespace ib {
namespace cfg {
inline namespace deprecated {

ParticipantBuilder::ParticipantBuilder(SimulationSetupBuilder* ibConfig, std::string name, mw::ParticipantId id)
    : ParentBuilder<SimulationSetupBuilder>{ibConfig}
{
    config.name = std::move(name);
    config.id = id;
}

auto ParticipantBuilder::Build() -> Participant
{
    if (_logger)
    {
        config.logger = _logger->Build();
        _logger.reset();
    }

    if (_participantController)
    {
        config.participantController = _participantController->Build();
        _participantController.reset();
    }

    BuildControllers(config.canControllers);
    BuildControllers(config.linControllers);
    BuildControllers(config.ethernetControllers);
    BuildControllers(config.flexrayControllers);
    BuildPorts(config.analogIoPorts);
    BuildPorts(config.digitalIoPorts);
    BuildPorts(config.pwmPorts);
    BuildPorts(config.patternPorts);

    for (auto&& builder : _genericPublishers)
    {
        config.genericPublishers.emplace_back(builder.Build());
    }

    for (auto&& builder : _genericSubscribers)
    {
        config.genericSubscribers.emplace_back(builder.Build());
    }

    for (auto&& builder : _dataPublishers)
    {
        config.dataPublishers.emplace_back(builder.Build());
    }

    for (auto&& builder : _dataSubscribers)
    {
        config.dataSubscribers.emplace_back(builder.Build());
    }
	
    for (auto&& builder : _rpcClients)
    {
        config.rpcClients.emplace_back(builder.Build());
    }

    for (auto&& builder : _rpcServers)
    {
        config.rpcServers.emplace_back(builder.Build());
    }
	
    for (auto& sink : _traceSinks)
    {
        config.traceSinks.emplace_back(sink.Build());
    }

    for (auto& source : _traceSources)
    {
        config.traceSources.emplace_back(source.Build());
    }

    for (auto& netSim : _networkSimulators)
    {
        config.networkSimulators.emplace_back(netSim.Build());
    }

    Participant newConfig;
    std::swap(config, newConfig);
    return newConfig;
}

auto ParticipantBuilder::operator->() -> ParticipantBuilder*
{
    return this;
}

auto ParticipantBuilder::ConfigureLogger() -> LoggerBuilder&
{
    _logger = std::make_unique<LoggerBuilder>();
    return *_logger;
}
auto ParticipantBuilder::AddParticipantController() -> ParticipantControllerBuilder&
{
    _participantController = std::make_unique<ParticipantControllerBuilder>(this);
    return *_participantController;
}
auto ParticipantBuilder::AddCan(std::string name) -> ControllerBuilder<CanController>&
{
    return AddController<CanController>(std::move(name), Parent()->GetFreeEndpointId());
}
auto ParticipantBuilder::AddLin(std::string name) -> ControllerBuilder<LinController>&
{
    return AddController<LinController>(std::move(name), Parent()->GetFreeEndpointId());
}
auto ParticipantBuilder::AddEthernet(std::string name) -> ControllerBuilder<EthernetController>&
{
    return AddController<EthernetController>(std::move(name), Parent()->GetFreeEndpointId());
}
auto ParticipantBuilder::AddFlexray(std::string name) -> ControllerBuilder<FlexrayController>&
{
    return AddController<FlexrayController>(std::move(name), Parent()->GetFreeEndpointId());
}

auto ParticipantBuilder::AddDigitalIn(std::string name) -> IoPortBuilder<DigitalIoPort>&
{
    return AddIoPort<DigitalIoPort>(std::move(name), PortDirection::In, Parent()->GetFreeEndpointId());
}

auto ParticipantBuilder::AddAnalogIn(std::string name) -> IoPortBuilder<AnalogIoPort>&
{
    return AddIoPort<AnalogIoPort>(std::move(name), PortDirection::In, Parent()->GetFreeEndpointId());
}

auto ParticipantBuilder::AddPwmIn(std::string name) -> IoPortBuilder<PwmPort>&
{
    return AddIoPort<PwmPort>(std::move(name), PortDirection::In, Parent()->GetFreeEndpointId());
}

auto ParticipantBuilder::AddPatternIn(std::string name) -> IoPortBuilder<PatternPort>&
{
    return AddIoPort<PatternPort>(std::move(name), PortDirection::In, Parent()->GetFreeEndpointId());
}

auto ParticipantBuilder::AddDigitalOut(std::string name) -> IoPortBuilder<DigitalIoPort>&
{
    return AddIoPort<DigitalIoPort>(std::move(name), PortDirection::Out, Parent()->GetFreeEndpointId());
}

auto ParticipantBuilder::AddAnalogOut(std::string name) -> IoPortBuilder<AnalogIoPort>&
{
    return AddIoPort<AnalogIoPort>(std::move(name), PortDirection::Out, Parent()->GetFreeEndpointId());
}

auto ParticipantBuilder::AddPwmOut(std::string name) -> IoPortBuilder<PwmPort>&
{
    return AddIoPort<PwmPort>(std::move(name), PortDirection::Out, Parent()->GetFreeEndpointId());
}

auto ParticipantBuilder::AddPatternOut(std::string name) -> IoPortBuilder<PatternPort>&
{
    return AddIoPort<PatternPort>(std::move(name), PortDirection::Out, Parent()->GetFreeEndpointId());
}

auto ParticipantBuilder::AddGenericPublisher(std::string name) -> GenericPortBuilder&
{
    _genericPublishers.emplace_back(this, std::move(name), Parent()->GetFreeEndpointId());
    return _genericPublishers[_genericPublishers.size() - 1];
}

auto ParticipantBuilder::AddGenericSubscriber(std::string name) -> GenericPortBuilder&
{
    _genericSubscribers.emplace_back(this, std::move(name), Parent()->GetFreeEndpointId());
    return _genericSubscribers[_genericSubscribers.size() - 1];
}

auto ParticipantBuilder::AddDataPublisher(std::string name) -> DataPortBuilder&
{
    _dataPublishers.emplace_back(this, std::move(name), Parent()->GetFreeEndpointId());
    return _dataPublishers[_dataPublishers.size() - 1];
}

auto ParticipantBuilder::AddDataSubscriber(std::string name) -> DataPortBuilder&
{
    _dataSubscribers.emplace_back(this, std::move(name), Parent()->GetFreeEndpointId());
    return _dataSubscribers[_dataSubscribers.size() - 1];
}

auto ParticipantBuilder::AddRpcClient(std::string name) -> RpcPortBuilder&
{
    _rpcClients.emplace_back(this, std::move(name), Parent()->GetFreeEndpointId());
    return _rpcClients[_rpcClients.size() - 1];
}

auto ParticipantBuilder::AddRpcServer(std::string name) -> RpcPortBuilder&
{
    _rpcServers.emplace_back(this, std::move(name), Parent()->GetFreeEndpointId());
    return _rpcServers[_rpcServers.size() - 1];
}

auto ParticipantBuilder::AddNetworkSimulator(std::string name) -> NetworkSimulatorBuilder&
{
    _networkSimulators.emplace_back(name);
    return _networkSimulators[_networkSimulators.size() - 1];
}

auto ParticipantBuilder::WithParticipantId(mw::ParticipantId id) -> ParticipantBuilder&
{
    if (id == 0)
        throw Misconfiguration{ "ParticipantId must not be 0" };

    config.id = id;
    return *this;
}

auto ParticipantBuilder::WithSyncType(SyncType syncType) -> ParticipantBuilder&
{
    std::cerr << R"deprecation(WARNING: ParticipantBuilder::WithSyncType(SyncType) is deprecated
    INFO: SyncType configuration is now part of the ParticipantController configuration.
    INFO: Replace participantBuilder.WithSyncType(syncType);
    INFO: with    participantBuilder.AddParticipantController().WithSyncType(syncType);
)deprecation";
    std::this_thread::sleep_for(3s);

    ParticipantController controller;
    controller.syncType = syncType;
    config.participantController = controller;
    return *this;
}

auto ParticipantBuilder::AsSyncMaster() -> ParticipantBuilder&
{
    config.isSyncMaster = true;
    return *this;
}

auto ParticipantBuilder::AddTraceSink(std::string sinkName) -> TraceSinkBuilder&
{
    _traceSinks.emplace_back(std::move(sinkName));
    return _traceSinks.back();
}

auto ParticipantBuilder::AddTraceSource(std::string sourceName) -> TraceSourceBuilder&
{
    _traceSources.emplace_back(std::move(sourceName));
    return _traceSources.back();
}

auto ParticipantBuilder::MakeQualifiedName(std::string controllerName) const -> std::string
{
    std::stringstream qualifiedName;
    qualifiedName << config.name << '/' << std::move(controllerName);
    return qualifiedName.str();
}

template<class ControllerT>
inline auto ParticipantBuilder::GetControllers() -> std::vector<ControllerBuilder<ControllerT>>&
{
    return std::get<std::vector<ControllerBuilder<ControllerT>>>(_endpoints);
}

template<class PortT>
inline auto ParticipantBuilder::GetPorts() -> std::vector<IoPortBuilder<PortT>>&
{
    return std::get<std::vector<IoPortBuilder<PortT>>>(_endpoints);
}


template<class ControllerT, class... Arg>
auto ParticipantBuilder::AddController(Arg&&... arg) -> ControllerBuilder<ControllerT>&
{
    auto&& controllers = GetControllers<ControllerT>();
    controllers.emplace_back(this, std::forward<Arg>(arg)...);
    return controllers[controllers.size() - 1];
}

template<class PortT, class... Arg>
auto ParticipantBuilder::AddIoPort(Arg&&... arg)->IoPortBuilder<PortT>&
{
    auto&& ioPorts = GetPorts<PortT>();
    ioPorts.emplace_back(this, std::forward<Arg>(arg)...);
    return ioPorts[ioPorts.size() - 1];
}

template<class ControllerT>
void ParticipantBuilder::BuildControllers(std::vector<ControllerT>& configs)
{
    for (auto&& builder : GetControllers<ControllerT>())
    {
        configs.emplace_back(builder.Build());
    }
}
template<class PortT>
void ParticipantBuilder::BuildPorts(std::vector<PortT>& configs)
{
    for (auto&& builder : GetPorts<PortT>())
    {
        configs.emplace_back(builder.Build());
    }
}

} // inline namespace deprecated
} // namespace cfg
} // namespace ib
