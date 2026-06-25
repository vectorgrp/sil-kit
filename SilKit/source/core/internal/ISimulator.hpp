#pragma once

// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// internal connection endpoint types:
#include "services/can/IMsgForCanSimulator.hpp"
#include "services/ethernet/IMsgForEthSimulator.hpp"
#include "services/flexray/IMsgForFlexrayBusSimulator.hpp"
#include "services/lin/IMsgForLinSimulator.hpp"

namespace SilKit {
namespace Core {

class ISimulator
    : public SilKit::Services::Can::IMsgForCanSimulator
    , public SilKit::Services::Ethernet::IMsgForEthSimulator
    , public SilKit::Services::Flexray::IMsgForFlexraySimulator
    , public SilKit::Services::Lin::IMsgForLinSimulator
    , public Core::IServiceEndpoint
{
public:
    virtual ~ISimulator() = default;
};

} // namespace Core
} // namespace SilKit
