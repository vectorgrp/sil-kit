// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "idl/EthernetTopicsPubSubTypes.h"
#include "idl/EthernetTopics.h"

#include "ib/sim/eth/EthDatatypes.hpp"

#include "IdlTraitMacro.hpp"

namespace ib {

    template<typename IdlT>
    struct TopicTrait;

    DefineTopicTrait(sim::eth::idl, EthMessage);
    DefineTopicTrait(sim::eth::idl, EthTransmitAcknowledge);
    DefineTopicTrait(sim::eth::idl, EthStatus);
    DefineTopicTrait(sim::eth::idl, EthSetMode);

}
