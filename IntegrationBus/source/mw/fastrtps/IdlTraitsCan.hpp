// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "idl/CanTopicsPubSubTypes.h"
#include "idl/CanTopics.h"

#include "ib/sim/can/CanDatatypes.hpp"

#include "IdlTraitMacro.hpp"

namespace ib {

    template<typename IdlT>
    struct TopicTrait;

    DefineTopicTrait(sim::can::idl, CanMessage);
    DefineTopicTrait(sim::can::idl, CanTransmitAcknowledge);
    DefineTopicTrait(sim::can::idl, CanControllerStatus);
    DefineTopicTrait(sim::can::idl, CanConfigureBaudrate);
    DefineTopicTrait(sim::can::idl, CanSetControllerMode);

}
