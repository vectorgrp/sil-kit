// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/fr/FrDatatypes.hpp"

#include "idl/FlexRayTopics.h"
#include "idl/FlexRayTopicsPubSubTypes.h"

#include "IdlTraitMacro.hpp"

namespace ib {

    template <typename IdlT>
    struct TopicTrait;

    DefineTopicTrait(sim::fr::idl, FrMessage);
    DefineTopicTrait(sim::fr::idl, FrMessageAck);
    DefineTopicTrait(sim::fr::idl, FrSymbol);
    DefineTopicTrait(sim::fr::idl, FrSymbolAck);
    DefineTopicTrait(sim::fr::idl, HostCommand);
    DefineTopicTrait(sim::fr::idl, ControllerConfig);
    DefineTopicTrait(sim::fr::idl, TxBufferUpdate);
    DefineTopicTrait(sim::fr::idl, ControllerStatus);

} // namespace ib
