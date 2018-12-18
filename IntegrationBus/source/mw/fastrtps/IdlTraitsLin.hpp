// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/lin/LinDatatypes.hpp"

#include "idl/LinTopics.h"
#include "idl/LinTopicsPubSubTypes.h"

#include "IdlTraitMacro.hpp"

namespace ib {

    template <typename IdlT>
    struct TopicTrait;

    DefineTopicTrait(sim::lin::idl, LinMessage);
    DefineTopicTrait(sim::lin::idl, RxRequest);
    DefineTopicTrait(sim::lin::idl, TxAcknowledge);
    DefineTopicTrait(sim::lin::idl, WakeupRequest);
    DefineTopicTrait(sim::lin::idl, ControllerConfig);
    DefineTopicTrait(sim::lin::idl, SlaveConfiguration);
    DefineTopicTrait(sim::lin::idl, SlaveResponse);

} // namespace ib
