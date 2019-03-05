// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "idl/LoggingTopicsPubSubTypes.h"
#include "idl/LoggingTopics.h"

#include "ib/mw/logging/LoggingDatatypes.hpp"

#include "IdlTraitMacro.hpp"

namespace ib {

template<typename IdlT>
struct TopicTrait;

DefineTopicTrait(mw::logging::idl, LogMsg);

}
