// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "FastRtpsGuard.hpp"

#include <mutex>

#include "fastrtps/Domain.h"

namespace ib {
namespace mw {
namespace FastRtps {

FastRtpsGuard::FastRtpsGuard()
{
    static std::weak_ptr<GuardImpl> sharedGuard;
    _guard = sharedGuard.lock();

    // Check if we have to create a new GuardImpl
    if (!_guard)
    {
        static std::mutex sharedGuardMutex;
        std::unique_lock<std::mutex> mutexGuard{sharedGuardMutex};

        // re-check if sharedFastRtpsGuard is still not valid.
        _guard = sharedGuard.lock();
        if (!_guard)
        {
            _guard = std::make_shared<GuardImpl>();
            sharedGuard = _guard;
        }
    }
}

FastRtpsGuard::GuardImpl::~GuardImpl()
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

    std::cout << "Domain::stopAll();" << std::endl;
    eprosima::fastrtps::Domain::stopAll();
}
    
} // namespace FastRtps
} // namespace mw
} // namespace ib
