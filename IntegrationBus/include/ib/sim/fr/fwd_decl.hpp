// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>

namespace ib {
namespace sim {
namespace fr {

struct ClusterParameters;
struct NodeParameters;
struct TxBufferConfig;

struct ControllerConfig;
struct TxBufferConfigUpdate;
struct TxBufferUpdate;

struct Header;
struct Frame;

struct FrMessage;
struct FrMessageAck;

struct FrSymbol;
struct FrSymbolAck;

struct CycleStart;

struct HostCommand;
struct ControllerStatus;
struct PocStatus;

class IFrController;
class IIbToFrController;
class IIbToFrControllerProxy;
class IIbToFrBusSimulator;

} // namespace fr
} // namespace sim
} // namespace ib
