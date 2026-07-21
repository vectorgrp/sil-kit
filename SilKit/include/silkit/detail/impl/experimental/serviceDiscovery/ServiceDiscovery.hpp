// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>

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
    SilKit_Experimental_ServiceDiscovery* _serviceDiscovery{nullptr};
    // The handler is stored in a slot whose address is passed to the C API as the callback context and
    // must therefore remain stable for the lifetime of this object. Repeated calls to
    // SetServiceDiscoveryHandler replace the stored std::function in place (never freeing the slot) and
    // register the C trampoline exactly once, so the context can never dangle and events are not
    // delivered twice. See SetServiceDiscoveryHandler below.
    std::shared_ptr<SilKit::Experimental::ServiceDiscovery::ServiceDiscoveryHandler> _handler;
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
    // Store the handler in a slot with a stable address (used as the C callback context). On repeated
    // calls we swap the std::function in place instead of freeing and re-registering, which would leave
    // the previously registered C trampoline pointing at a destroyed std::function (use-after-free).
    if (_handler == nullptr)
    {
        _handler = std::make_shared<SilKit::Experimental::ServiceDiscovery::ServiceDiscoveryHandler>();
    }
    *_handler = std::move(handler);

    if (_registered)
    {
        // The trampoline reads the current value of *_handler on every invocation, so the replacement
        // above already takes effect; registering again would duplicate event delivery.
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
        serviceDescriptor.simulatingParticipantName = orEmpty(cServiceDescriptor->simulatingParticipantName);
        serviceDescriptor.numberOfConnections = cServiceDescriptor->numberOfConnections;
        serviceDescriptor.isSimulated = cServiceDescriptor->isSimulated != SilKit_False;

        auto& userHandler = *static_cast<SD::ServiceDiscoveryHandler*>(context);
        if (userHandler)
        {
            userHandler(static_cast<SD::ServiceDiscoveryEventType>(eventType), serviceDescriptor);
        }
    };

    const auto returnCode =
        SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(_serviceDiscovery, _handler.get(), cHandler);
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
