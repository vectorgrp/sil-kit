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
    std::unique_ptr<SilKit::Experimental::ServiceDiscovery::ServiceDiscoveryHandler> _handler;
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
    auto ownedHandler =
        std::make_unique<SilKit::Experimental::ServiceDiscovery::ServiceDiscoveryHandler>(std::move(handler));

    const auto cHandler = [](void* context, SilKit_Experimental_ServiceDiscoveryEvent_Type eventType,
                             const SilKit_Experimental_ServiceDescriptor* cServiceDescriptor) {
        namespace SD = SilKit::Experimental::ServiceDiscovery;

        SD::ServiceDescriptor serviceDescriptor{};
        serviceDescriptor.participantName = cServiceDescriptor->participantName;
        serviceDescriptor.serviceName = cServiceDescriptor->serviceName;
        serviceDescriptor.serviceKind = static_cast<SD::ServiceKind>(cServiceDescriptor->serviceKind);
        serviceDescriptor.primaryIdentifier = cServiceDescriptor->primaryIdentifier;
        serviceDescriptor.mediaType = cServiceDescriptor->mediaType;
        serviceDescriptor.labels.reserve(cServiceDescriptor->labelList.numLabels);
        for (size_t i = 0; i < cServiceDescriptor->labelList.numLabels; ++i)
        {
            const auto& cLabel = cServiceDescriptor->labelList.labels[i];
            SilKit::Services::MatchingLabel label;
            label.key = cLabel.key;
            label.value = cLabel.value;
            label.kind = static_cast<SilKit::Services::MatchingLabel::Kind>(cLabel.kind);
            serviceDescriptor.labels.emplace_back(std::move(label));
        }
        serviceDescriptor.simulationName = cServiceDescriptor->simulationName ? cServiceDescriptor->simulationName : "";
        serviceDescriptor.connectedParticipantName = cServiceDescriptor->connectedParticipantName ? cServiceDescriptor->connectedParticipantName : "";
        serviceDescriptor.connectedServiceName = cServiceDescriptor->connectedServiceName ? cServiceDescriptor->connectedServiceName : "";
        serviceDescriptor.simulatingParticipantName = cServiceDescriptor->simulatingParticipantName ? cServiceDescriptor->simulatingParticipantName : "";
        serviceDescriptor.numberOfConnections = cServiceDescriptor->numberOfConnections;
        serviceDescriptor.isSimulated = cServiceDescriptor->isSimulated != SilKit_False;

        (*static_cast<SD::ServiceDiscoveryHandler*>(context))(
            static_cast<SD::ServiceDiscoveryEventType>(eventType), serviceDescriptor);
    };

    const auto returnCode =
        SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(_serviceDiscovery, ownedHandler.get(), cHandler);
    ThrowOnError(returnCode);

    _handler = std::move(ownedHandler);
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
