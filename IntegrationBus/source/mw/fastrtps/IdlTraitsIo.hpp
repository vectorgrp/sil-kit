// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "idl/IoTopicsPubSubTypes.h"
#include "idl/IoTopics.h"

#include "IdlTraitMacro.hpp"

namespace ib {

    template<typename IdlT>
    struct TopicTrait;

    DefineTopicTrait(sim::io::idl, AnalogIoMessage);
    DefineTopicTrait(sim::io::idl, DigitalIoMessage);
    DefineTopicTrait(sim::io::idl, PatternIoMessage);
    DefineTopicTrait(sim::io::idl, PwmIoMessage);

}
