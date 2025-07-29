// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include "ITimeProvider.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

/*!
* \brief Virtual time consumer. Used to register with a time provider.
* 
*/
class ITimeConsumer
{
public:
    virtual ~ITimeConsumer() {}
    virtual void SetTimeProvider(ITimeProvider* provider) = 0;
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
