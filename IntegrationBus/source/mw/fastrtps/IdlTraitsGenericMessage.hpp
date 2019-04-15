// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "idl/GenericTopicsPubSubTypes.h"
#include "idl/GenericTopics.h"

#include "ib/sim/generic/GenericMessageDatatypes.hpp"

#include "IdlTraitMacro.hpp"

namespace ib {

    template<typename IdlT>
    struct TopicTrait;

    DefineTopicTrait(sim::generic::idl, GenericMessage);

}
