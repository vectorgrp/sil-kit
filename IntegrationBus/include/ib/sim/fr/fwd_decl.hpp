// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>

namespace ib {
namespace sim {
namespace fr {

struct FlexrayClusterParameters;
struct FlexrayNodeParameters;
struct FlexrayTxBufferConfig;

struct FlexrayControllerConfig;
struct FlexrayTxBufferConfigUpdate;
struct FlexrayTxBufferUpdate;

struct FlexrayHeader;
struct FlexrayFrame;

struct FlexrayFrameEvent;
struct FlexrayFrameTransmitEvent;

struct FlexraySymbolEvent;
struct FlexraySymbolTransmitEvent;

struct FlexrayCycleStartEvent;

struct FlexrayHostCommand;
struct FlexrayPocStatusEvent;

class IFlexrayController;

} // namespace fr
} // namespace sim
} // namespace ib
