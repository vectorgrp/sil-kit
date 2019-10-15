// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
namespace ib {
namespace cfg {

struct Version;
struct Link;
struct Logger;
struct CanController;
struct LinController;
struct EthernetController;
struct FlexrayController;
struct Participant;
struct Switch;
struct NetworkSimulator;
struct TimeSync;
struct Config;
class Misconfiguration;

class LoggerBuilder;
class ParticipantBuilder;
template<class ControllerCfg>
class ControllerBuilder;
class ConfigBuilder;
class SimulationSetupBuilder;
class SwitchBuilder;
class NetworkSimulatorBuilder;
class SwitchPortBuilder;
class SwitchBuilder;
class TimeSyncBuilder;
class LinkBuilder;

// IB Internal: resolve cyclic dependencies by not de-referencing Parent() (ie., an incomplete type at declaration point) in the children directly
template<template<class> class Builder, typename BuilderCfg>
auto ParticipantMakeQualifiedName(Builder<BuilderCfg> &builder, const std::string controllerName) -> std::string
{
    return builder.Parent()->MakeQualifiedName(controllerName);
}
//helper template for double deref
template<template<class> class Builder, typename BuilderCfg, typename LinkType>
auto __SimBuilderAddOrGetLink(Builder<BuilderCfg> *builder, LinkType linkType, const std::string& linkName) -> LinkBuilder&
{
    return builder->Parent()->AddOrGetLink(linkType, linkName);
}
template<template<class> class Builder, typename BuilderCfg, typename LinkType>
auto ParticipantBuilderAddOrGetLink(Builder<BuilderCfg> &builder, LinkType linkType, const std::string& linkName) -> LinkBuilder&
{
    return __SimBuilderAddOrGetLink(builder.Parent(), linkType, linkName);
}

} // namespace cfg
} // namespace ib
