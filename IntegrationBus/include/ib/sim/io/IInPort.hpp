// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>

#include "IoDatatypes.hpp"
#include "ib/cfg/Config.hpp"

namespace ib {
namespace sim {
namespace io {

template<typename MsgT>
class IInPort
{
public:
    using MessageType = MsgT;
    using ValueType = decltype(MsgT::value);
    using ConfigType = cfg::IoPort<ValueType>;

    template<typename T>
    using CallbackT = std::function<void(IInPort* port, const T& t)>;

    //! \brief Callback type for whole IO messages
    using MessageHandler = CallbackT<MsgT>;

    //! \brief Callback type for received IO values
    using ValueHandler   = CallbackT<ValueType>;

public:
    virtual ~IInPort() {}

    //! \brief Get the config struct used to setup this IInPort
    virtual auto Config() const -> const ConfigType& = 0;

    //! \brief Read the most recently received IO value
    virtual auto Read() const -> const ValueType& = 0;

    /*! \brief Register a handler for new IO messages
     *
     * The handler is called upon reception of a new IO messages. It
     * has access to the whole message including its timestamp.
     */
    virtual void RegisterHandler(MessageHandler handler) = 0;
    /*! \brief Register a handler for new IO value
     *
     * The handler is called upon reception of a new IO messages. It
     * is intended for simpler handlers, for which the new IO value is
     * sufficient.
     */
    virtual void RegisterHandler(ValueHandler handler) = 0;
};

using IDigitalInPort = IInPort<DigitalIoMessage>;
using IAnalogInPort  = IInPort<AnalogIoMessage>;
using IPwmInPort     = IInPort<PwmIoMessage>;
using IPatternInPort = IInPort<PatternIoMessage>;

} // namespace io
} // namespace sim
} // namespace ib
