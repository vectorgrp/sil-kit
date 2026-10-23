// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/participant/exception.hpp"

#include "capi/CapiImpl.hpp"
#include "capi/ServiceObserver.hpp"

#include "core/internal/IParticipantInternal.hpp"
#include "core/service/IServiceDiscovery.hpp"
#include "core/internal/ServiceDescriptor.hpp"

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

namespace {

namespace Discovery = SilKit::Core::Discovery;

auto GetServiceDiscovery(SilKit_Participant* participant) -> Discovery::IServiceDiscovery*
{
    auto* cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
    auto* participantInternal = dynamic_cast<SilKit::Core::IParticipantInternal*>(cppParticipant);
    if (participantInternal == nullptr)
    {
        throw SilKit::SilKitError{"participant is not a valid SilKit::IParticipant*"};
    }
    return participantInternal->GetServiceDiscovery();
}

// The single observer of each participant's service discovery. The internal handler owns the observer, so the
// entry expires when the participant, and with it the internal handler, is destroyed.
struct ObserverRegistry
{
    std::mutex mutex;
    std::map<Discovery::IServiceDiscovery*, std::weak_ptr<VSilKit::ServiceObserver>> observers;
};

auto GetObserverRegistry() -> ObserverRegistry&
{
    static ObserverRegistry registry;
    return registry;
}

// Called with registry.mutex held.
void EraseExpiredObservers(ObserverRegistry& registry)
{
    for (auto it = registry.observers.begin(); it != registry.observers.end();)
    {
        if (it->second.expired())
        {
            it = registry.observers.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

} // namespace


SilKit_ReturnCode SilKitCALL SilKit_Experimental_ServiceDiscovery_Create(
    SilKit_Experimental_ServiceDiscovery** outServiceDiscovery, SilKit_Participant* participant)
try
{
    ASSERT_VALID_OUT_PARAMETER(outServiceDiscovery);
    ASSERT_VALID_POINTER_PARAMETER(participant);

    auto* serviceDiscovery = GetServiceDiscovery(participant);
    *outServiceDiscovery = reinterpret_cast<SilKit_Experimental_ServiceDiscovery*>(serviceDiscovery);

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(
    SilKit_Experimental_ServiceDiscovery* serviceDiscovery, void* context,
    SilKit_Experimental_ServiceDiscoveryHandler_t handler)
try
{
    ASSERT_VALID_POINTER_PARAMETER(serviceDiscovery);
    ASSERT_VALID_HANDLER_PARAMETER(handler);

    auto* cppServiceDiscovery = reinterpret_cast<Discovery::IServiceDiscovery*>(serviceDiscovery);
    auto& registry = GetObserverRegistry();

    std::shared_ptr<VSilKit::ServiceObserver> existing;
    std::shared_ptr<VSilKit::ServiceObserver> observer;
    {
        std::lock_guard<std::mutex> lock{registry.mutex};
        EraseExpiredObservers(registry);
        const auto it = registry.observers.find(cppServiceDiscovery);
        if (it != registry.observers.end())
        {
            existing = it->second.lock();
        }
        if (existing == nullptr)
        {
            // Registered before the internal handler, so that a concurrent call replaces the handler of this observer
            // instead of registering a second one, and a call from within the replay is rejected below.
            observer = std::make_shared<VSilKit::ServiceObserver>(handler, context);
            registry.observers[cppServiceDiscovery] = observer;
        }
    }

    if (existing != nullptr)
    {
        if (existing->IsDispatchingOnThisThread())
        {
            throw SilKit::StateError{"The service discovery handler must not be set from within a handler"};
        }
        existing->SetHandler(handler, context);
        return SilKit_ReturnCode_SUCCESS;
    }

    // The internal service discovery replays the already known services synchronously on this thread, inside
    // RegisterServiceDiscoveryHandler and under its lock; later events come from other threads or after the call
    // returned. So an invocation on this thread while registering is exactly part of that replay.
    auto registeringThread = std::make_shared<std::atomic<std::thread::id>>(std::this_thread::get_id());

    try
    {
        cppServiceDiscovery->RegisterServiceDiscoveryHandler(
            [observer, registeringThread](Discovery::ServiceDiscoveryEvent::Type type,
                                          const SilKit::Core::ServiceDescriptor& serviceDescriptor) {
            // Invoked by the internal service discovery with its lock held, serialized, on an unspecified
            // thread (an IO worker for remote events, or the caller's thread for locally created/removed
            // services). Exceptions must never propagate into SIL Kit internals, except during the replay: that
            // runs inside this function, so the failure reaches the caller as an error code.
            const bool isReplay = std::this_thread::get_id() == registeringThread->load();
            try
            {
                observer->HandleEvent(type, serviceDescriptor);
            }
            catch (...)
            {
                if (isReplay)
                {
                    throw;
                }
            }
        });
    }
    catch (...)
    {
        std::lock_guard<std::mutex> lock{registry.mutex};
        const auto it = registry.observers.find(cppServiceDiscovery);
        if (it != registry.observers.end() && it->second.lock() == observer)
        {
            registry.observers.erase(it);
        }
        throw;
    }
    registeringThread->store(std::thread::id{});

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS
