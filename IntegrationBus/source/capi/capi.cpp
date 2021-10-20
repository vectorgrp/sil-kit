/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

//#define CIntegrationBusAPI_EXPORT
#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/can/all.hpp"
#include "ib/sim/can/string_utils.hpp"
#include <string>
#include <iostream>
#pragma pack(8)

extern "C" {

#pragma region GENERAL
thread_local std::string ib_error_string = "";

#define ib_ReturnCode_SUCCESS_str "Operation succeeded."
#define ib_ReturnCode_UNSPECIFIEDERROR_str "An unspecified error occured."
#define ib_ReturnCode_NOTSUPPORTED_str "Operation is not supported."
#define ib_ReturnCode_NOTIMPLEMENTED_str "Operation is not implemented."
#define ib_ReturnCode_BADPARAMETER_str "Operation failed due to a bad parameter."
#define ib_ReturnCode_BUFFERTOOSMALL_str "Operation failed because a buffer is too small."
#define ib_ReturnCode_TIMEOUT_str "Operation timed out."
#define ib_ReturnCode_UNSUPPORTEDSERVICE_str "The requested service is not supported."


IntegrationBusAPI ib_ReturnCode ib_ReturnCodeToString(const char** outString, ib_ReturnCode returnCode)
{
    if (outString == nullptr)
    {
        return ib_ReturnCode_BADPARAMETER;
    }

    switch (returnCode)
    {
    case ib_ReturnCode_SUCCESS: *outString = ib_ReturnCode_SUCCESS_str; break;
    case ib_ReturnCode_UNSPECIFIEDERROR: *outString = ib_ReturnCode_UNSPECIFIEDERROR_str; break;
    case ib_ReturnCode_NOTSUPPORTED: *outString = ib_ReturnCode_NOTSUPPORTED_str; break;
    case ib_ReturnCode_NOTIMPLEMENTED: *outString = ib_ReturnCode_NOTIMPLEMENTED_str; break;
    case ib_ReturnCode_BADPARAMETER: *outString = ib_ReturnCode_BADPARAMETER_str; break;
    case ib_ReturnCode_BUFFERTOOSMALL: *outString = ib_ReturnCode_BUFFERTOOSMALL_str; break;
    case ib_ReturnCode_TIMEOUT: *outString = ib_ReturnCode_TIMEOUT_str; break;
    case ib_ReturnCode_UNSUPPORTEDSERVICE: *outString = ib_ReturnCode_UNSUPPORTEDSERVICE_str; break;
    default: return ib_ReturnCode_BADPARAMETER; break;
    }

    return ib_ReturnCode_SUCCESS;
}


CIntegrationBusAPI const char* ib_GetLastErrorString() {
    const char* error_string = ib_error_string.c_str();;
    return error_string;
}

ib_ReturnCode ib_SimulationParticipant_destroy(ib_SimulationParticipant* self)
{
    try {
        auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(self);
        delete comAdapter;
        return ib_ReturnCode_SUCCESS;
    }
    catch (const std::runtime_error& e) {
        ib_error_string = e.what();
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    catch (const std::exception& ) {
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
}

ib_ReturnCode ib_SimulationParticipant_GetComAdapter(void** outWrappedObject, ib_SimulationParticipant* self)
{
    try {
        if (outWrappedObject == nullptr)
        {
            return ib_ReturnCode_BADPARAMETER;
        }
        auto* comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(self);
        *outWrappedObject = comAdapter;

        return ib_ReturnCode_SUCCESS;
    }
    catch (const std::runtime_error& e) {
        ib_error_string = e.what();
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    catch (const std::exception& ) {
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
}

IntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_create(ib_SimulationParticipant** outParticipant, const char* cJsonConfig, const char* cParticipantName, const char* cDomainId)
{
    try {
        if (outParticipant == nullptr)
        {
            return ib_ReturnCode_BADPARAMETER;
        }

        std::string jsonConfigStr(cJsonConfig);
        std::string participantName(cParticipantName);
        std::string domainIdStr(cDomainId);
        uint32_t domainId = atoi(domainIdStr.c_str());

        auto ibConfig = ib::cfg::Config::FromJsonString(jsonConfigStr);

        std::cout << "Creating ComAdapter for Participant=" << participantName << " in Domain " << domainId << std::endl;
        auto comAdapter = ib::CreateComAdapter(ibConfig, participantName, domainId).release();
            
        *outParticipant = reinterpret_cast<ib_SimulationParticipant*>(comAdapter);
        return ib_ReturnCode_SUCCESS;
    }
    catch (const std::runtime_error& e) 
    {
        ib_error_string = e.what();
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    catch (const std::exception& ) 
    {
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
}
#pragma endregion GENERAL



#pragma region CAN

std::map<uint32_t, void*> transmitContexMap;

ib_ReturnCode ib_CanController_RegisterReceiveMessageHandler(ib_CanController* self, void* context, void (*Callback)(void* context, ib_CanController* controller, ib_CanFrame_Meta* canFrameMeta))
{
    try {
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->RegisterReceiveMessageHandler(
            [context, self, Callback](ib::sim::can::ICanController* /*ctrl*/, const ib::sim::can::CanMessage& msg)
            {
                ib_CanFrame_Meta cmm;
                ib_CanFrame cm;
                cmm.timestamp = msg.timestamp.count();
                cmm.interfaceId = ib_InterfaceIdentifier_CanFrame_Meta;
                cmm.canFrame = &cm;

                cm.id = msg.canId;
                uint32_t flags = 0;
                flags |= msg.flags.ide ? ib_CanFrameFlag_ide : 0;
                flags |= msg.flags.rtr ? ib_CanFrameFlag_rtr : 0;
                flags |= msg.flags.fdf ? ib_CanFrameFlag_fdf : 0;
                flags |= msg.flags.brs ? ib_CanFrameFlag_brs : 0;
                flags |= msg.flags.esi ? ib_CanFrameFlag_esi : 0;
                cm.flags = flags;
                cm.dlc = msg.dlc;
                std::copy(msg.dataField.begin(), msg.dataField.end(), cm.data);
                cm.dataLength = (uint32_t)msg.dataField.size();

                Callback(context, self, &cmm);
            });
        return ib_ReturnCode_SUCCESS;
    }
    catch (const std::runtime_error& e) {
        ib_error_string = e.what();
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    catch (const std::exception& ) {
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
}

ib_ReturnCode ib_CanController_RegisterTransmitStatusHandler(ib_CanController* self, void* context, void (*Callback)(void* context, ib_CanController* controller, ib_CanTransmitAcknowledge* cAck))
{
    try {
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->RegisterTransmitStatusHandler(
            [Callback, context, self](ib::sim::can::ICanController* /*ctrl*/, const ib::sim::can::CanTransmitAcknowledge& ack)
            {
                ib_CanTransmitAcknowledge tcack;

                auto transmitContext = transmitContexMap[ack.transmitId];
                tcack.userContext = transmitContext;
                transmitContexMap.erase(ack.transmitId);
                tcack.timestamp = ack.timestamp.count();
                tcack.status = (ib_CanTransmitStatus)ack.status;
                Callback(context, self, &tcack);
            });
        return ib_ReturnCode_SUCCESS;
    }
    catch (const std::runtime_error& e) {
        ib_error_string = e.what();
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    catch (const std::exception& ) {
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
}

ib_ReturnCode ib_CanController_RegisterStateChangedHandler(ib_CanController* self, void* context, void (*Callback)(void* context, ib_CanController* controller, ib_CanControllerState state))
{
    try {
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->RegisterStateChangedHandler(
            [Callback, context, self](ib::sim::can::ICanController* /*ctrl*/, const ib::sim::can::CanControllerState state)
            {
                Callback(context, self, (ib_CanControllerState)state);
            });
        return ib_ReturnCode_SUCCESS;
    }
    catch (const std::runtime_error& e) {
        ib_error_string = e.what();
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    catch (const std::exception& ) {
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
}

ib_ReturnCode ib_CanController_RegisterErrorStateChangedHandler(ib_CanController* self, void* context, void (*Callback)(void* context, ib_CanController* controller, ib_CanErrorState state))
{
    try {
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->RegisterErrorStateChangedHandler(
            [Callback, context, self](ib::sim::can::ICanController* /*ctrl*/, const ib::sim::can::CanErrorState state)
            {
                Callback(context, self, (ib_CanErrorState)state);
            });
        return ib_ReturnCode_SUCCESS;
    }
    catch (const std::runtime_error& e) {
        ib_error_string = e.what();
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    catch (const std::exception& ) {
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
}

ib_ReturnCode ib_CanController_SetBaudRate(ib_CanController* self, uint32_t rate, uint32_t fdRate)
{
    try {
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->SetBaudRate(rate, fdRate);
        return ib_ReturnCode_SUCCESS;
    }
    catch (const std::runtime_error& e) {
        ib_error_string = e.what();
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    catch (const std::exception& ) {
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
}

ib_ReturnCode ib_CanController_SendFrame(ib_CanController* self, ib_CanFrame* message, void* transmitContext)
{
    try {
        using std::chrono::duration;
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);

        ib::sim::can::CanMessage cm;
        cm.timestamp = {};
        cm.canId = message->id;
        cm.flags.ide = message->flags & ib_CanFrameFlag_ide;
        cm.flags.rtr = message->flags & ib_CanFrameFlag_rtr;
        cm.flags.fdf = message->flags & ib_CanFrameFlag_fdf;
        cm.flags.brs = message->flags & ib_CanFrameFlag_brs;
        cm.flags.esi = message->flags & ib_CanFrameFlag_esi;

        static int msgId = 0;
        std::stringstream payloadBuilder;
        payloadBuilder << "CAN " << msgId++;
        auto payloadStr = payloadBuilder.str();

        cm.dlc = message->dlc;
        cm.dataField = std::vector<uint8_t>(&(message->data[0]), &(message->data[0]) + message->dataLength);

        auto transmitId = canController->SendMessage(std::move(cm));
        transmitContexMap[transmitId] = transmitContext;
        return ib_ReturnCode_SUCCESS;
    }
    catch (const std::runtime_error& e) {
        ib_error_string = e.what();
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    catch (const std::exception& ) {
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
}

ib_ReturnCode ib_CanController_Start(ib_CanController* self)
{
    try {
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->Start();
        return ib_ReturnCode_SUCCESS;
    }
    catch (const std::runtime_error& e) {
        ib_error_string = e.what();
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    catch (const std::exception& ) {
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
}

ib_ReturnCode ib_CanController_Stop(ib_CanController* self)
{
    try {
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->Stop();
        return ib_ReturnCode_SUCCESS;
    }
    catch (const std::runtime_error& e) {
        ib_error_string = e.what();
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    catch (const std::exception& ) {
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
}

ib_ReturnCode ib_CanController_Reset(ib_CanController* self)
{
    try {
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->Reset();
        return ib_ReturnCode_SUCCESS;
    }
    catch (const std::runtime_error& e) {
        ib_error_string = e.what();
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    catch (const std::exception& ) {
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
}

ib_ReturnCode ib_CanController_Sleep(ib_CanController* self)
{
    try {
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->Sleep();
        return ib_ReturnCode_SUCCESS;
    }
    catch (const std::runtime_error& e) {
        ib_error_string = e.what();
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    catch (const std::exception& ) {
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
}


CIntegrationBusAPI ib_ReturnCode ib_CanController_create(ib_CanController** outCanController, ib_SimulationParticipant* self,  const char* cName)
{
    try {
        if (outCanController == nullptr)
        {
            return ib_ReturnCode_BADPARAMETER;
        }

        std::string name(cName);
        auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(self);
        auto canController = comAdapter->CreateCanController(name);
        *outCanController = reinterpret_cast<ib_CanController*>(canController);
        return ib_ReturnCode_SUCCESS;
    }
    catch (const std::runtime_error& e) {
        ib_error_string = e.what();
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    catch (const std::exception& ) {
        return ib_ReturnCode_UNSPECIFIEDERROR;
    }
}
#pragma endregion CAN


}
