// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <mutex>

#include "silkit/capi/SilKit.h"

#include "silkit/experimental/serviceDiscovery/IServiceDiscovery.hpp"

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
    };

    SilKit_Experimental_ServiceDiscovery* _serviceDiscovery{nullptr};
    // C callback context; the trampoline invokes a copy of the current handler
    std::unique_ptr<HandlerSlot> _slot{std::make_unique<HandlerSlot>()};
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
}

void ServiceDiscovery::SetServiceDiscoveryHandler(
    SilKit::Experimental::ServiceDiscovery::ServiceDiscoveryHandler handler)
{
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

        const auto orEmpty = [](const char* s) -> const char* { return s != nullptr ? s : ""; };

        SD::ServiceDescriptor serviceDescriptor{};
        serviceDescriptor.participantName = orEmpty(cServiceDescriptor->participantName);
        serviceDescriptor.serviceName = orEmpty(cServiceDescriptor->serviceName);
        serviceDescriptor.serviceKind = static_cast<SD::ServiceKind>(cServiceDescriptor->serviceKind);
        serviceDescriptor.primaryIdentifier = orEmpty(cServiceDescriptor->primaryIdentifier);
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
        serviceDescriptor.simulationName = orEmpty(cServiceDescriptor->simulationName);
        serviceDescriptor.connectedParticipantName = orEmpty(cServiceDescriptor->connectedParticipantName);
        serviceDescriptor.connectedServiceName = orEmpty(cServiceDescriptor->connectedServiceName);

        auto* slot = static_cast<HandlerSlot*>(context);
        std::shared_ptr<const SD::ServiceDiscoveryHandler> userHandler;
        {
            std::lock_guard<std::mutex> lock{slot->mutex};
            userHandler = slot->handler;
        }
        if (userHandler && *userHandler)
        {
            (*userHandler)(static_cast<SD::ServiceDiscoveryEventType>(eventType), serviceDescriptor);
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

} // namespace ServiceDiscovery
} // namespace Experimental
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
