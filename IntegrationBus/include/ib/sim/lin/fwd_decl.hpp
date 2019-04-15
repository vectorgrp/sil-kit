// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>

namespace ib {
namespace sim {
namespace lin { 

struct LinMessage;
struct RxRequest;
struct TxAcknowledge;
struct WakeupRequest;

struct ControllerConfig;
struct SlaveResponseConfig;
struct SlaveConfiguration;
struct SlaveResponse;

class ILinController;
class IIbToLinController;
class IIbToLinControllerProxy;
class IIbToLinSimulator;

} // namespace lin
} // namespace sim
} // namespace ib
