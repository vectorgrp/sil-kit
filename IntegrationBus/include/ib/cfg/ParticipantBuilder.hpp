// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <tuple>
#include <vector>

#include "ib/IbMacros.hpp"

#include "fwd_decl.hpp"

#include "Config.hpp"
#include "ParentBuilder.hpp"
#include "ControllerBuilder.hpp"
#include "GenericPortBuilder.hpp"
#include "DataPortBuilder.hpp"
#include "RpcPortBuilder.hpp"
#include "LoggerBuilder.hpp"
#include "ParticipantControllerBuilder.hpp"
#include "NetworkSimulatorBuilder.hpp"
#include "TraceSinkBuilder.hpp"
#include "TraceSourceBuilder.hpp"

namespace ib {
namespace cfg {
inline namespace deprecated {

class ParticipantBuilder : public ParentBuilder<SimulationSetupBuilder>
{
public:
    IntegrationBusAPI ParticipantBuilder(SimulationSetupBuilder* ibConfig, std::string name, mw::ParticipantId id);

    IntegrationBusAPI auto ConfigureLogger() -> LoggerBuilder&;
    IntegrationBusAPI auto AddParticipantController() -> ParticipantControllerBuilder&;
    IntegrationBusAPI auto AddCan(std::string name) -> ControllerBuilder<CanController>&;
    IntegrationBusAPI auto AddLin(std::string name) -> ControllerBuilder<LinController>&;
    IntegrationBusAPI auto AddEthernet(std::string name) -> ControllerBuilder<EthernetController>&;
    IntegrationBusAPI auto AddFlexray(std::string name) -> ControllerBuilder<FlexrayController>&;

    IntegrationBusAPI auto AddGenericPublisher(std::string name) -> GenericPortBuilder&;
    IntegrationBusAPI auto AddGenericSubscriber(std::string name) -> GenericPortBuilder&;

    IntegrationBusAPI auto AddDataPublisher(std::string name) -> DataPortBuilder&;
    IntegrationBusAPI auto AddDataSubscriber(std::string name) -> DataPortBuilder&;
	
    IntegrationBusAPI auto AddRpcClient(std::string name) -> RpcPortBuilder&;
    IntegrationBusAPI auto AddRpcServer(std::string name) -> RpcPortBuilder&;

    IntegrationBusAPI auto AddNetworkSimulator(std::string name) -> NetworkSimulatorBuilder&;

    IntegrationBusAPI auto WithParticipantId(mw::ParticipantId id) -> ParticipantBuilder&;
    IntegrationBusAPI auto WithSyncType(SyncType syncType) -> ParticipantBuilder&;
    IntegrationBusAPI auto AsSyncMaster() -> ParticipantBuilder&;


    IntegrationBusAPI auto AddTraceSink(std::string name) -> TraceSinkBuilder&;
    IntegrationBusAPI auto AddTraceSource(std::string name) -> TraceSourceBuilder&;

    IntegrationBusAPI auto operator->() -> ParticipantBuilder*;

    IntegrationBusAPI auto Build() -> Participant;


public:
    // IB Internal Only
    IntegrationBusAPI auto MakeQualifiedName(std::string controllerName) const -> std::string;

private:
    template<class ControllerT>
    inline auto GetControllers() -> std::vector<ControllerBuilder<ControllerT>>&;

    template<class ControllerT, class... Arg>
    auto AddController(Arg&&... arg) -> ControllerBuilder<ControllerT>&;

    template<class ControllerT>
    void BuildControllers(std::vector<ControllerT>& configs);

private:
    Participant config;

    std::unique_ptr<LoggerBuilder> _logger;
    std::unique_ptr<ParticipantControllerBuilder> _participantController;

    std::tuple<
        std::vector<ControllerBuilder<CanController>>,
        std::vector<ControllerBuilder<LinController>>,
        std::vector<ControllerBuilder<EthernetController>>,
        std::vector<ControllerBuilder<FlexrayController>>
    > _endpoints;

    std::vector<GenericPortBuilder> _genericPublishers;
    std::vector<GenericPortBuilder> _genericSubscribers;

    std::vector<DataPortBuilder> _dataPublishers;
    std::vector<DataPortBuilder> _dataSubscribers;
	
    std::vector<RpcPortBuilder> _rpcClients;
    std::vector<RpcPortBuilder> _rpcServers;

    std::vector<TraceSinkBuilder> _traceSinks;
    std::vector<TraceSourceBuilder> _traceSources;
    std::vector<NetworkSimulatorBuilder> _networkSimulators;
};

} // namespace deprecated
} // namespace cfg
} // namespace ib
