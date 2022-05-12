// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <iostream>

#include "ib/sim/fr/FlexrayDatatypes.hpp"

namespace ib {
namespace sim {
namespace fr {

bool operator==(const FlexrayHeader& lhs, const FlexrayHeader& rhs);
bool operator==(const FlexrayFrame& lhs, const FlexrayFrame& rhs);
bool operator==(const FlexrayFrameEvent& lhs, const FlexrayFrameEvent& rhs);
bool operator==(const FlexrayFrameTransmitEvent& lhs, const FlexrayFrameTransmitEvent& rhs);
bool operator==(const FlexraySymbolEvent& lhs, const FlexraySymbolEvent& rhs);
bool operator==(const FlexrayWakeupEvent& lhs, const FlexrayWakeupEvent& rhs);
bool operator==(const FlexrayTxBufferConfigUpdate& lhs, const FlexrayTxBufferConfigUpdate& rhs);
bool operator==(const FlexrayTxBufferUpdate& lhs, const FlexrayTxBufferUpdate& rhs);
bool operator==(const FlexrayControllerConfig& lhs, const FlexrayControllerConfig& rhs);
bool operator==(const FlexrayHostCommand& lhs, const FlexrayHostCommand& rhs);
bool operator==(const FlexrayPocStatusEvent& lhs, const FlexrayPocStatusEvent& rhs);
bool operator==(const FlexrayCycleStartEvent& lhs, const FlexrayCycleStartEvent& rhs);


} // namespace fr
} // namespace sim
} // namespace ib
