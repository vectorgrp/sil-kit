// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>

namespace ib {
namespace sim {
namespace lin { 

struct Transmission;
struct WakeupPulse;
struct SendFrameRequest;
struct SendFrameHeaderRequest;
struct ControllerConfig;
struct ControllerStatusUpdate;
struct FrameResponseUpdate;

class ILinController;
class IIbToLinController;
class IIbToLinControllerProxy;
class IIbToLinSimulator;

} // namespace lin
} // namespace sim
} // namespace ib
