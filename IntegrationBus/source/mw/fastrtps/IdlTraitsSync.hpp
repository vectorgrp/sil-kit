// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "idl/MiddlewareTopicsPubSubTypes.h"
#include "idl/MiddlewareTopics.h"

#include "ib/mw/sync/SyncDatatypes.hpp"

#include "IdlTraitMacro.hpp"

namespace ib {

    template<typename IdlT>
    struct TopicTrait;

    DefineTopicTrait(mw::sync::idl, QuantumRequest);
    DefineTopicTrait(mw::sync::idl, QuantumGrant);
    DefineTopicTrait(mw::sync::idl, Tick);
    DefineTopicTrait(mw::sync::idl, TickDone);
    DefineTopicTrait(mw::sync::idl, ParticipantCommand);
    DefineTopicTrait(mw::sync::idl, SystemCommand);
    DefineTopicTrait(mw::sync::idl, ParticipantStatus);

}
