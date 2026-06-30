// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/participant/exception.hpp"

#include "capi/CapiImpl.hpp"

#include "core/internal/IParticipantInternal.hpp"
#include "core/service/IServiceDiscovery.hpp"
#include "core/internal/ServiceDescriptor.hpp"

namespace {

auto GetServiceDiscovery(SilKit_Participant* participant) -> SilKit::Core::Discovery::IServiceDiscovery*
{
    auto* cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
    auto* participantInternal = dynamic_cast<SilKit::Core::IParticipantInternal*>(cppParticipant);
    if (participantInternal == nullptr)
    {
        throw SilKit::SilKitError{"participant is not a valid SilKit::IParticipant*"};
    }
    return participantInternal->GetServiceDiscovery();
}

auto ToC(SilKit::Core::Discovery::ServiceDiscoveryEvent::Type type)
    -> SilKit_Experimental_ServiceDiscoveryEvent_Type
{
    using Type = SilKit::Core::Discovery::ServiceDiscoveryEvent::Type;
    switch (type)
    {
    case Type::ServiceCreated:
        return SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated;
    case Type::ServiceRemoved:
        return SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved;
    default:
        return SilKit_Experimental_ServiceDiscoveryEvent_Type_Invalid;
    }
}

auto FromC(const SilKit_Experimental_ServiceDescriptor* serviceDescriptor) -> const SilKit::Core::ServiceDescriptor*
{
    return reinterpret_cast<const SilKit::Core::ServiceDescriptor*>(serviceDescriptor);
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

    auto* cppServiceDiscovery = reinterpret_cast<SilKit::Core::Discovery::IServiceDiscovery*>(serviceDiscovery);

    cppServiceDiscovery->RegisterServiceDiscoveryHandler(
        [handler, context](SilKit::Core::Discovery::ServiceDiscoveryEvent::Type type,
                           const SilKit::Core::ServiceDescriptor& serviceDescriptor) {
        handler(context, ToC(type),
                reinterpret_cast<const SilKit_Experimental_ServiceDescriptor*>(&serviceDescriptor));
    });

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_ServiceDescriptor_GetParticipantName(
    const SilKit_Experimental_ServiceDescriptor* serviceDescriptor, const char** outParticipantName)
try
{
    ASSERT_VALID_POINTER_PARAMETER(serviceDescriptor);
    ASSERT_VALID_OUT_PARAMETER(outParticipantName);

    *outParticipantName = FromC(serviceDescriptor)->GetParticipantName().c_str();

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_ServiceDescriptor_GetServiceName(
    const SilKit_Experimental_ServiceDescriptor* serviceDescriptor, const char** outServiceName)
try
{
    ASSERT_VALID_POINTER_PARAMETER(serviceDescriptor);
    ASSERT_VALID_OUT_PARAMETER(outServiceName);

    *outServiceName = FromC(serviceDescriptor)->GetServiceName().c_str();

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_ServiceDescriptor_GetNetworkName(
    const SilKit_Experimental_ServiceDescriptor* serviceDescriptor, const char** outNetworkName)
try
{
    ASSERT_VALID_POINTER_PARAMETER(serviceDescriptor);
    ASSERT_VALID_OUT_PARAMETER(outNetworkName);

    *outNetworkName = FromC(serviceDescriptor)->GetNetworkName().c_str();

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_ServiceDescriptor_GetServiceType(
    const SilKit_Experimental_ServiceDescriptor* serviceDescriptor, SilKit_Experimental_ServiceType* outServiceType)
try
{
    ASSERT_VALID_POINTER_PARAMETER(serviceDescriptor);
    ASSERT_VALID_OUT_PARAMETER(outServiceType);

    *outServiceType = static_cast<SilKit_Experimental_ServiceType>(FromC(serviceDescriptor)->GetServiceType());

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_ServiceDescriptor_GetSupplementalDataItem(
    const SilKit_Experimental_ServiceDescriptor* serviceDescriptor, const char* key, const char** outValue,
    SilKit_Bool* outHasValue)
try
{
    ASSERT_VALID_POINTER_PARAMETER(serviceDescriptor);
    ASSERT_VALID_POINTER_PARAMETER(key);
    ASSERT_VALID_OUT_PARAMETER(outValue);
    ASSERT_VALID_OUT_PARAMETER(outHasValue);

    const auto& supplementalData = FromC(serviceDescriptor)->GetSupplementalDataRef();
    const auto it = supplementalData.find(key);
    if (it == supplementalData.end())
    {
        *outValue = nullptr;
        *outHasValue = SilKit_False;
    }
    else
    {
        *outValue = it->second.c_str();
        *outHasValue = SilKit_True;
    }

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS
