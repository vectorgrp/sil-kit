// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <cstdint>

// ================================================================================
//  I/O specific data types
// ================================================================================
namespace ib {
namespace sim {
namespace io {

struct PwmValue;

template <typename ValueT>
struct IoMessage;

using AnalogIoMessage = IoMessage<double>;
using DigitalIoMessage = IoMessage<bool>;
using PatternIoMessage = IoMessage<std::vector<uint8_t>>;
using PwmIoMessage = IoMessage<PwmValue>;

template<typename MsgT>
class IInPort;

using IDigitalInPort = IInPort<DigitalIoMessage>;
using IAnalogInPort  = IInPort<AnalogIoMessage>;
using IPwmInPort     = IInPort<PwmIoMessage>;
using IPatternInPort = IInPort<PatternIoMessage>;

template<typename MsgT>
class IOutPort;

using IDigitalOutPort = IOutPort<DigitalIoMessage>;
using IAnalogOutPort  = IOutPort<AnalogIoMessage>;
using IPwmOutPort     = IOutPort<PwmIoMessage>;
using IPatternOutPort = IOutPort<PatternIoMessage>;

} // namespace io
} // namespace sim
} // namespace ib
