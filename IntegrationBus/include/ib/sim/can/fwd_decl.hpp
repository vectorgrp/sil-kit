// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

namespace ib {
namespace sim {
namespace can {

struct CanMessage;
struct CanControllerStatus;
struct CanConfigureBaudrate;
struct CanSetControllerMode;
struct CanTransmitAcknowledge;

class ICanController;
class IIbToCanController;
class IIbToCanControllerProxy;
class IIbToCanSimulator;

} // namespace can
} // namespace sim
} // namespace ib
