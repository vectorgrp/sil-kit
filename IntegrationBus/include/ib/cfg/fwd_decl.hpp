// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

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
class ParticipantControllerBuilder;
template<class ControllerCfg>
class ControllerBuilder;
class ConfigBuilder;
class SimulationSetupBuilder;
class SwitchBuilder;
class NetworkSimulatorBuilder;
class SwitchPortBuilder;
class SwitchBuilder;
class TimeSyncBuilder;

} // namespace cfg
} // namespace ib
