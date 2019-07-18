// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/lin/LinDatatypes.hpp"

#include "idl/LinTopics.h"
#include "idl/LinTopicsPubSubTypes.h"

#include "IdlTraitMacro.hpp"

namespace ib {

    template <typename IdlT>
    struct TopicTrait;

    DefineTopicTrait(sim::lin::idl, SendFrameRequest);
    DefineTopicTrait(sim::lin::idl, SendFrameHeaderRequest);
    DefineTopicTrait(sim::lin::idl, Transmission);
    DefineTopicTrait(sim::lin::idl, WakeupPulse);
    DefineTopicTrait(sim::lin::idl, ControllerConfig);
    DefineTopicTrait(sim::lin::idl, ControllerStatusUpdate);
    DefineTopicTrait(sim::lin::idl, FrameResponseUpdate);


} // namespace ib
