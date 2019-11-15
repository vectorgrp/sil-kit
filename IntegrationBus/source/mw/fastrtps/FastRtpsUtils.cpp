// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "FastRtpsUtils.hpp"

#include "fastrtps/Domain.h"

namespace ib {
namespace mw {
namespace FastRtps {

auto GetFastRtpsDomainLock() -> std::unique_lock<std::mutex>
{
    static std::mutex fastRtpsDomainMutex;
    return std::unique_lock<std::mutex>{fastRtpsDomainMutex};
}

RemoveParticipant::RemoveParticipant()
    : _shutdownGuard{ShutdownGuard::Create()}
{
}

void RemoveParticipant::operator()(eprosima::fastrtps::Participant* participant)
{
    auto lock{GetFastRtpsDomainLock()};
    eprosima::fastrtps::Domain::removeParticipant(participant);
}

auto RemoveParticipant::ShutdownGuard::Create() -> std::shared_ptr<RemoveParticipant::ShutdownGuard>
{
    static std::weak_ptr<ShutdownGuard> sharedGuard;

    // Check if there is already an instance of a shared ShutdownGuard
    auto guard = sharedGuard.lock();
    if (guard)
    {
        // yes, return the new "reference" to the shared ShutdownGuard
        return std::move(guard);
    }
    else
    {
        // Nope. Seems like we have to create the ShutdownGuard
        static std::mutex sharedGuardMutex;
        std::unique_lock<std::mutex> mutexGuard{sharedGuardMutex};

        // re-check if the shared ShutdownGuard is now available
        guard = sharedGuard.lock();
        if (guard)
        {
            // Yes. Seems like someone else has created the ShutdownGuard in the meantime
            return std::move(guard);
        }
        else
        {
            // Still nope. But we have the lock. So we are responsible for creating the shared ShutdownGuard
            guard = std::make_shared<ShutdownGuard>();
            sharedGuard = guard;
            return std::move(guard);
        }
    }
}

    
RemoveParticipant::ShutdownGuard::~ShutdownGuard()
{
    // Ensure that FastRTPS is properly shut down when it is no longer needed. I.e, by no ComAdapter in the entire Process.

    // FastRTPS does two things in Domain::stopAll(), of which the first thing is *not* needed according to how we use the API, and even harmful: 
    // (1) Delete previously undeleted participants: 
    //     The IntegrationBus already calls removeParticipant, the list should be already empty.
    // (2) Killing the logging thread 
    //     On the first glance, this should not be needed, since it will be called from Log::Resources::~Resources().
    //     However, with VisualStudio 2015/2017 the STL is deadlocking when condition variables are used on CRT shutdown and DLL unload in particular.
    //     https://stackoverflow.com/questions/35828004/waitforsingleobject-for-thread-object-does-not-work-in-dll-unload
    //     https://social.msdn.microsoft.com/Forums/en-US/7f91e360-2108-40ca-8728-e295f17cb26b/visual-c-2015-stdconditionvariablenotifyall-may-get-stuck
    //     https://stackoverflow.com/questions/39241400/cleaning-up-threads-in-a-dll-endthreadex-vs-terminatethread
    //     https://stackoverflow.com/questions/10915233/stdthreadjoin-hangs-if-called-after-main-exits-when-using-vs2012-rc

    eprosima::fastrtps::Domain::stopAll();
}

void RemovePublisher::operator()(eprosima::fastrtps::Publisher* publisher)
{
    eprosima::fastrtps::Domain::removePublisher(publisher);
}

void RemoveSubscriber::operator()(eprosima::fastrtps::Subscriber* subscriber)
{
    eprosima::fastrtps::Domain::removeSubscriber(subscriber);
}
    
} // namespace FastRtps
} // namespace mw
} // namespace ib
