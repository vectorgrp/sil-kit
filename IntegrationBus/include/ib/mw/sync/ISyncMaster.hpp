// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>

namespace ib {
namespace mw {
namespace sync {

/*!
 * \brief Generic synchronization master.
 * 
 * NB: This interface is still subject to change.
 */
class ISyncMaster
{
public:
    virtual ~ISyncMaster() = default;

    virtual void WaitForShutdown() = 0;
};

} // namespace sync
} // namespace mw
} // namespace ib
