// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "silkit/capi/SilKit.h"

#include "silkit/experimental/serviceDiscovery/IServiceDiscovery.hpp"
#include "silkit/participant/exception.hpp"

#include "silkit/detail/impl/ThrowOnError.hpp"
#include "silkit/detail/macros.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Experimental {
namespace ServiceDiscovery {

class ServiceDiscovery : public SilKit::Experimental::ServiceDiscovery::IServiceDiscovery
{
public:
    inline explicit ServiceDiscovery(SilKit_Participant* participant);

    inline ~ServiceDiscovery() override = default;

    inline void SetServiceDiscoveryHandler(
        SilKit::Experimental::ServiceDiscovery::ServiceDiscoveryHandler handler) override;

public:
    inline auto Get() const -> SilKit_Experimental_ServiceDiscovery*;

private:
    struct HandlerSlot
    {
        std::mutex mutex;
        std::shared_ptr<const SilKit::Experimental::ServiceDiscovery::ServiceDiscoveryHandler> handler;
        // Set once in the constructor, so the trampoline reads it without the lock
        SilKit_Logger* logger{nullptr};
        // The thread currently running the user handler, to reject setting the handler from within it
        std::atomic<std::thread::id> dispatchingThread{};
    };

    static inline void LogHandlerException(SilKit_Logger* logger, const char* what);

    SilKit_Experimental_ServiceDiscovery* _serviceDiscovery{nullptr};
    // C callback context; the trampoline invokes a copy of the current handler
    std::unique_ptr<HandlerSlot> _slot{std::make_unique<HandlerSlot>()};
    // Only accessed by SetServiceDiscoveryHandler, never by the trampoline, so unlike the handler it needs no lock
    bool _registered{false};
};

} // namespace ServiceDiscovery
} // namespace Experimental
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


// ================================================================================
//  Inline Implementations
// ================================================================================

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Experimental {
namespace ServiceDiscovery {

ServiceDiscovery::ServiceDiscovery(SilKit_Participant* participant)
{
    const auto returnCode = SilKit_Experimental_ServiceDiscovery_Create(&_serviceDiscovery, participant);
    ThrowOnError(returnCode);

    const auto loggerReturnCode = SilKit_Participant_GetLogger(&_slot->logger, participant);
    ThrowOnError(loggerReturnCode);
}

void ServiceDiscovery::SetServiceDiscoveryHandler(
    SilKit::Experimental::ServiceDiscovery::ServiceDiscoveryHandler handler)
{
    if (_slot->dispatchingThread.load() == std::this_thread::get_id())
    {
        throw SilKit::StateError{"The service discovery handler must not be set from within a handler"};
    }

    auto replaced =
        std::make_shared<const SilKit::Experimental::ServiceDiscovery::ServiceDiscoveryHandler>(std::move(handler));
    {
        std::lock_guard<std::mutex> lock{_slot->mutex};
        _slot->handler.swap(replaced);
    }
    // A running invocation keeps its own reference to the replaced handler

    if (_registered)
    {
        return;
    }

    const auto cHandler = [](void* context, SilKit_Experimental_ServiceDiscoveryEvent_Type eventType,
                             const SilKit_Experimental_ServiceDescriptor* cServiceDescriptor) {
        namespace SD = SilKit::Experimental::ServiceDiscovery;

        auto* slot = static_cast<HandlerSlot*>(context);

        // This function is called through the C ABI from inside the SIL Kit library; an exception must not
        // unwind across that boundary.
        try
        {
            const auto orEmpty = [](const char* s) -> const char* { return s != nullptr ? s : ""; };

            SD::ServiceDescriptor serviceDescriptor{};
            serviceDescriptor.participantName = orEmpty(cServiceDescriptor->participantName);
            serviceDescriptor.serviceName = orEmpty(cServiceDescriptor->serviceName);
            serviceDescriptor.serviceId = cServiceDescriptor->serviceId;
            serviceDescriptor.serviceKind = static_cast<SD::ServiceKind>(cServiceDescriptor->serviceKind);
            serviceDescriptor.primaryIdentifier = orEmpty(cServiceDescriptor->primaryIdentifier);
            serviceDescriptor.networkType = static_cast<SilKit::Experimental::NetworkSimulation::SimulatedNetworkType>(
                cServiceDescriptor->networkType);
            serviceDescriptor.mediaType = orEmpty(cServiceDescriptor->mediaType);
            serviceDescriptor.labels.reserve(cServiceDescriptor->labelList.numLabels);
            for (size_t i = 0; i < cServiceDescriptor->labelList.numLabels; ++i)
            {
                const auto& cLabel = cServiceDescriptor->labelList.labels[i];
                SilKit::Services::MatchingLabel label;
                label.key = orEmpty(cLabel.key);
                label.value = orEmpty(cLabel.value);
                label.kind = static_cast<SilKit::Services::MatchingLabel::Kind>(cLabel.kind);
                serviceDescriptor.labels.emplace_back(std::move(label));
            }
            serviceDescriptor.operationMode =
                static_cast<SilKit::Services::Orchestration::OperationMode>(cServiceDescriptor->operationMode);
            serviceDescriptor.timeSyncActive = cServiceDescriptor->timeSyncActive == SilKit_True;
            serviceDescriptor.connectedParticipantName = orEmpty(cServiceDescriptor->connectedParticipantName);
            serviceDescriptor.connectedServiceName = orEmpty(cServiceDescriptor->connectedServiceName);
            serviceDescriptor.connectedServiceId = cServiceDescriptor->connectedServiceId;

            std::shared_ptr<const SD::ServiceDiscoveryHandler> userHandler;
            {
                std::lock_guard<std::mutex> lock{slot->mutex};
                userHandler = slot->handler;
            }
            if (userHandler && *userHandler)
            {
                struct DispatchScope
                {
                    std::atomic<std::thread::id>& dispatchingThread;
                    std::thread::id previous;
                    ~DispatchScope()
                    {
                        dispatchingThread.store(previous);
                    }
                } dispatchScope{slot->dispatchingThread, slot->dispatchingThread.exchange(std::this_thread::get_id())};

                (*userHandler)(static_cast<SD::ServiceDiscoveryEventType>(eventType), serviceDescriptor);
            }
        }
        catch (const std::exception& exception)
        {
            LogHandlerException(slot->logger, exception.what());
        }
        catch (...)
        {
        }
    };

    const auto returnCode =
        SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(_serviceDiscovery, _slot.get(), cHandler);
    ThrowOnError(returnCode);

    _registered = true;
}

auto ServiceDiscovery::Get() const -> SilKit_Experimental_ServiceDiscovery*
{
    return _serviceDiscovery;
}

void ServiceDiscovery::LogHandlerException(SilKit_Logger* logger, const char* what)
{
    if (logger == nullptr)
    {
        return;
    }
    try
    {
        const std::string message = std::string{"Detected exception in service discovery handler:\n"} + what;
        SilKit_Logger_Log(logger, SilKit_LoggingLevel_Warn, message.c_str());
    }
    catch (...)
    {
    }
}

} // namespace ServiceDiscovery
} // namespace Experimental
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
