// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <tuple>
#include <vector>

#include "ib/IbMacros.hpp"

#include "fwd_decl.hpp"

#include "Config.hpp"
#include "ParentBuilder.hpp"
#include "ControllerBuilder.hpp"
#include "IoPortBuilder.hpp"
#include "GenericPortBuilder.hpp"

namespace ib {
namespace cfg {

class ParticipantBuilder : public ParentBuilder<SimulationSetupBuilder>
{
public:
    IntegrationBusAPI ParticipantBuilder(SimulationSetupBuilder* ibConfig, std::string name, mw::ParticipantId id);

    IntegrationBusAPI auto AddCan(std::string name) -> ControllerBuilder<CanController>&;
    IntegrationBusAPI auto AddLin(std::string name) -> ControllerBuilder<LinController>&;
    IntegrationBusAPI auto AddEthernet(std::string name) -> ControllerBuilder<EthernetController>&;
    IntegrationBusAPI auto AddFlexray(std::string name) -> ControllerBuilder<FlexrayController>&;
    IntegrationBusAPI auto AddDigitalIn(std::string name) -> IoPortBuilder<DigitalIoPort>&;
    IntegrationBusAPI auto AddAnalogIn(std::string name) -> IoPortBuilder<AnalogIoPort>&;
    IntegrationBusAPI auto AddPwmIn(std::string name) -> IoPortBuilder<PwmPort>&;
    IntegrationBusAPI auto AddPatternIn(std::string name) -> IoPortBuilder<PatternPort>&;
    IntegrationBusAPI auto AddDigitalOut(std::string name) -> IoPortBuilder<DigitalIoPort>&;
    IntegrationBusAPI auto AddAnalogOut(std::string name) -> IoPortBuilder<AnalogIoPort>&;
    IntegrationBusAPI auto AddPwmOut(std::string name) -> IoPortBuilder<PwmPort>&;
    IntegrationBusAPI auto AddPatternOut(std::string name) -> IoPortBuilder<PatternPort>&;

    IntegrationBusAPI auto AddGenericPublisher(std::string name) -> GenericPortBuilder&;
    IntegrationBusAPI auto AddGenericSubscriber(std::string name) -> GenericPortBuilder&;

    IntegrationBusAPI auto AddNetworkSimulator(std::string name) -> NetworkSimulatorBuilder&;

    IntegrationBusAPI auto WithParticipantId(mw::ParticipantId id) -> ParticipantBuilder&;
    IntegrationBusAPI auto WithSyncType(SyncType syncType) -> ParticipantBuilder&;
    IntegrationBusAPI auto AsSyncMaster() -> ParticipantBuilder&;

    IntegrationBusAPI auto operator->() -> ParticipantBuilder*;

    IntegrationBusAPI auto Build() -> Participant;


public:
    // IB Internal Only
    IntegrationBusAPI auto MakeQualifiedName(std::string controllerName) const -> std::string;

private:
    template<class ControllerT>
    inline auto GetControllers() -> std::vector<ControllerBuilder<ControllerT>>&;
    template<class PortT>
    inline auto GetPorts() -> std::vector<IoPortBuilder<PortT>>&;

    template<class ControllerT, class... Arg>
    auto AddController(Arg&&... arg) -> ControllerBuilder<ControllerT>&;
    template<class PortT, class... Arg>
    auto AddIoPort(Arg&&... arg) -> IoPortBuilder<PortT>&;

    template<class ControllerT>
    void BuildControllers(std::vector<ControllerT>& configs);
    template<class PortT>
    void BuildPorts(std::vector<PortT>& configs);

private:
    Participant config;

    std::tuple<
        std::vector<ControllerBuilder<CanController>>,
        std::vector<ControllerBuilder<LinController>>,
        std::vector<ControllerBuilder<EthernetController>>,
        std::vector<ControllerBuilder<FlexrayController>>,
        std::vector<IoPortBuilder<AnalogIoPort>>,
        std::vector<IoPortBuilder<DigitalIoPort>>,
        std::vector<IoPortBuilder<PwmPort>>,
        std::vector<IoPortBuilder<PatternPort>>
    > _endpoints;

    std::vector<GenericPortBuilder> _genericPublishers;
    std::vector<GenericPortBuilder> _genericSubscribers;
};

} // namespace cfg
} // namespace ib
