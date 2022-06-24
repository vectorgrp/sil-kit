// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include "ITimeProvider.hpp"

namespace ib {
namespace mw {
namespace sync {

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

} // namespace sync
} // namespace mw
} // namespace ib
