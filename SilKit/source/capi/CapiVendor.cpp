// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ParticipantConfiguration.hpp"
#include "CreateSilKitRegistryImpl.hpp"

#include "silkit/capi/SilKit.h"

#include "silkit/vendor/ISilKitRegistry.hpp"

#include "CapiImpl.hpp"
#include "TypeConversion.hpp"

#include <memory>
#include <map>
#include <mutex>


SilKit_ReturnCode SilKitCALL SilKit_Vendor_Vector_SilKitRegistry_Create(
    SilKit_Vendor_Vector_SilKitRegistry** outSilKitRegistry, SilKit_ParticipantConfiguration* participantConfiguration)
try
{
    ASSERT_VALID_OUT_PARAMETER(outSilKitRegistry);
    ASSERT_VALID_POINTER_PARAMETER(participantConfiguration);

    auto* cppParticipantConfiguration =
        reinterpret_cast<SilKit::Config::ParticipantConfiguration*>(participantConfiguration);

    // since we do _not_ want to consume the pointer itself, we must make a copy and store it in a shared_ptr
    auto cppSharedParticipantConfiguration =
        std::make_shared<SilKit::Config::ParticipantConfiguration>(*cppParticipantConfiguration);

    auto* silKitRegistry =
        SilKit::Vendor::Vector::CreateSilKitRegistryImpl(std::move(cppSharedParticipantConfiguration)).release();

    if (silKitRegistry == nullptr)
    {
        SilKit_error_string = "Creating simulation registry failed due to an unknown error and returned null pointer.";
        return SilKit_ReturnCode_UNSPECIFIEDERROR;
    }

    *outSilKitRegistry = reinterpret_cast<SilKit_Vendor_Vector_SilKitRegistry*>(silKitRegistry);

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL
SilKit_Vendor_Vector_SilKitRegistry_Destroy(SilKit_Vendor_Vector_SilKitRegistry* cSilKitRegistry)
try
{
    ASSERT_VALID_POINTER_PARAMETER(cSilKitRegistry);

    auto* cppSilKitRegistry = reinterpret_cast<SilKit::Vendor::Vector::ISilKitRegistry*>(cSilKitRegistry);
    delete cppSilKitRegistry;

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Vendor_Vector_SilKitRegistry_SetAllDisconnectedHandler(
    SilKit_Vendor_Vector_SilKitRegistry* cSilKitRegistry, void* context,
    SilKit_Vendor_Vector_SilKitRegistry_AllDisconnectedHandler_t handler)
try
{
    ASSERT_VALID_POINTER_PARAMETER(cSilKitRegistry);
    ASSERT_VALID_HANDLER_PARAMETER(handler);

    auto* cppRegistry = reinterpret_cast<SilKit::Vendor::Vector::ISilKitRegistry*>(cSilKitRegistry);

    cppRegistry->SetAllDisconnectedHandler(
        [handler, context, cSilKitRegistry]() { handler(context, cSilKitRegistry); });

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Vendor_Vector_SilKitRegistry_GetLogger(
    SilKit_Logger** outLogger, SilKit_Vendor_Vector_SilKitRegistry* cSilKitRegistry)
try
{
    ASSERT_VALID_OUT_PARAMETER(outLogger);
    ASSERT_VALID_POINTER_PARAMETER(cSilKitRegistry);

    auto* silKitRegistry = reinterpret_cast<SilKit::Vendor::Vector::ISilKitRegistry*>(cSilKitRegistry);

    *outLogger = reinterpret_cast<SilKit_Logger*>(silKitRegistry->GetLogger());

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Vendor_Vector_SilKitRegistry_StartListening(
    SilKit_Vendor_Vector_SilKitRegistry* cSilKitRegistry, const char* listenUri, const char** outRegistryUri)
try
{
    ASSERT_VALID_POINTER_PARAMETER(cSilKitRegistry);
    ASSERT_VALID_POINTER_PARAMETER(listenUri);
    ASSERT_VALID_OUT_PARAMETER(outRegistryUri);

    thread_local std::string theRegistryUri;

    auto* silKitRegistry = reinterpret_cast<SilKit::Vendor::Vector::ISilKitRegistry*>(cSilKitRegistry);

    theRegistryUri = silKitRegistry->StartListening(listenUri);
    *outRegistryUri = theRegistryUri.c_str();

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS
