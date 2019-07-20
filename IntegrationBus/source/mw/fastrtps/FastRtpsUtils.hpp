// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <mutex>

#include "fastrtps_fwd.h"

namespace ib {
namespace mw {
namespace FastRtps {

auto GetFastRtpsDomainLock() -> std::unique_lock<std::mutex>;
    
class RemoveParticipant
{
public:
    RemoveParticipant();
    void operator()(eprosima::fastrtps::Participant* participant);

private:
    struct ShutdownGuard
    {
        static auto Create() -> std::shared_ptr<ShutdownGuard>;
        ~ShutdownGuard();
    };
    
    std::shared_ptr<ShutdownGuard> _shutdownGuard;
};

struct RemovePublisher
{
    void operator()(eprosima::fastrtps::Publisher* publisher);
};

struct RemoveSubscriber
{
    void operator()(eprosima::fastrtps::Subscriber* subscriber);
};
    
} // namespace FastRtps
} // namespace mw
} // namespace ib
