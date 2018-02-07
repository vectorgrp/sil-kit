// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

namespace ib {
namespace mw {
namespace FastRtps {

//! \Brief Helper to ensure that FastRTPS is only shut down when there are no active users anymore.
class FastRtpsGuard
{
public:
    FastRtpsGuard();
    FastRtpsGuard(const FastRtpsGuard& other) = default;
    FastRtpsGuard(FastRtpsGuard&& other) = default;

    FastRtpsGuard& operator=(const FastRtpsGuard& other) = default;
    FastRtpsGuard& operator=(FastRtpsGuard&& other) = default;

private:
    struct GuardImpl
    {
        ~GuardImpl();
    };

    std::shared_ptr<GuardImpl> _guard;
};

    
} // namespace FastRtps
} // namespace mw
} // namespace ib
