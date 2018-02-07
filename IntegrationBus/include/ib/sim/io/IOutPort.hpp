// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>

#include "IoDatatypes.hpp"
#include "ib/cfg/Config.hpp"

namespace ib {
namespace sim {
namespace io {

template<typename MsgT>
class IOutPort
{
public:
    using MessageType = MsgT;
    using ValueType = decltype(MsgT::value);
    using ConfigType = cfg::IoPort<ValueType>;

public:
    virtual ~IOutPort() {}

    //! \brief Get the config struct used to setup this IOutPort
    virtual auto Config() const -> const ConfigType& = 0;

    /*! \ Write a new IO value to the port
     *
     * Value and timestamp are combined to an IO message that is sent
     * via the connected \ref ib::mw::IComAdapter
     */
    virtual void Write(ValueType value, std::chrono::nanoseconds timestamp) = 0;

    /*! \brief Get the most recently written value
     *
     *  Returns the initial value according to the IB config if,
     *  otherwise, no value was written yet.
     */
    virtual auto Read() const -> const ValueType& = 0;
};

using IDigitalOutPort = IOutPort<DigitalIoMessage>;
using IAnalogOutPort  = IOutPort<AnalogIoMessage>;
using IPwmOutPort     = IOutPort<PwmIoMessage>;
using IPatternOutPort = IOutPort<PatternIoMessage>;

} // namespace io
} // namespace sim
} // namespace ib
