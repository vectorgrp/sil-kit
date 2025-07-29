// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>

namespace SilKit {
namespace Services {
namespace Flexray {

struct FlexrayClusterParameters;
struct FlexrayNodeParameters;
struct FlexrayTxBufferConfig;

struct FlexrayControllerConfig;
struct FlexrayTxBufferUpdate;

struct FlexrayHeader;
struct FlexrayFrame;

struct FlexrayFrameEvent;
struct FlexrayFrameTransmitEvent;

struct FlexraySymbolEvent;
struct FlexraySymbolTransmitEvent;
struct FlexrayWakeupEvent;

struct FlexrayCycleStartEvent;

struct FlexrayPocStatusEvent;

class IFlexrayController;

} // namespace Flexray
} // namespace Services
} // namespace SilKit
