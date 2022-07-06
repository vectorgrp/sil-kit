// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "silkit/services/can/CanDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Can {

bool operator==(const CanFrame::CanFrameFlags& lhs, const CanFrame::CanFrameFlags& rhs);
bool operator==(const CanFrame& lhs, const CanFrame& rhs);
bool operator==(const CanFrameEvent& lhs, const CanFrameEvent& rhs);
bool operator==(const CanFrameTransmitEvent& lhs, const CanFrameTransmitEvent& rhs);
bool operator==(const CanSetControllerMode& lhs, const CanSetControllerMode& rhs);
bool operator==(const CanConfigureBaudrate& lhs, const CanConfigureBaudrate& rhs);
bool operator==(const CanStateChangeEvent& lhs, const CanStateChangeEvent& rhs);
bool operator==(const CanErrorStateChangeEvent& lhs, const CanErrorStateChangeEvent& rhs);

}
}
}
