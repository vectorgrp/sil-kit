/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

//#define CIntegrationBusAPI_EXPORT
#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/can/all.hpp"
#include "ib/sim/can/string_utils.hpp"
#include "ib/sim/eth/all.hpp"
#include "ib/sim/eth/string_utils.hpp"
#include "ib/sim/generic/all.hpp"
#include "ib/sim/generic/string_utils.hpp"
#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <mutex>

#define CAPI_DEFINE_FUNC(BODY) \
    try { \
        BODY \
    } \
    catch (const std::runtime_error& e) { \
        ib_error_string = e.what(); \
        return ib_ReturnCode_UNSPECIFIEDERROR; \
    } \
    catch (const std::exception&) { \
        return ib_ReturnCode_UNSPECIFIEDERROR; \
    }

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
    const char* error_string = ib_error_string.c_str();
    return error_string;
}

ib_ReturnCode ib_SimulationParticipant_destroy(ib_SimulationParticipant* self)
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr)
        {
            ib_error_string = "A nullpointer argument was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(self);
        delete comAdapter;
        return ib_ReturnCode_SUCCESS;
        )
}

ib_ReturnCode ib_SimulationParticipant_GetComAdapter(void** outWrappedObject, ib_SimulationParticipant* self)
{
    CAPI_DEFINE_FUNC(
        if (outWrappedObject == nullptr || self == nullptr)
        {
            ib_error_string = "A nullpointer argument was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto* comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(self);
        *outWrappedObject = comAdapter;

        return ib_ReturnCode_SUCCESS;
        )
}

IntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_create(ib_SimulationParticipant** outParticipant, const char* cJsonConfig, const char* cParticipantName, const char* cDomainId)
{
    CAPI_DEFINE_FUNC(
        if (outParticipant == nullptr || cJsonConfig == nullptr || cParticipantName == nullptr || cDomainId == nullptr)
        {
            ib_error_string = "A nullpointer argument was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }

        std::string jsonConfigStr(cJsonConfig);
        std::string participantName(cParticipantName);
        std::string domainIdStr(cDomainId);
        uint32_t domainId = atoi(domainIdStr.c_str());

        auto ibConfig = ib::cfg::Config::FromJsonString(jsonConfigStr);

        std::cout << "Creating ComAdapter for Participant=" << participantName << " in Domain " << domainId << std::endl;
        auto comAdapter = ib::CreateComAdapter(ibConfig, participantName, domainId).release();
        
        if (comAdapter == nullptr)
        {
            ib_error_string = "Creating Simulation Participant failed due to unknown error and returned null pointer.";
            return ib_ReturnCode_UNSPECIFIEDERROR;
        }

        *outParticipant = reinterpret_cast<ib_SimulationParticipant*>(comAdapter);
        return ib_ReturnCode_SUCCESS;
        )
}
#pragma endregion GENERAL



#pragma region CAN

struct PendingTransmits {
    std::map<uint32_t, void*> userContextById;
    std::map<uint32_t, std::function<void()>> callbacksById;
    std::mutex mutex;
};
PendingTransmits pendingTransmits;

ib_ReturnCode ib_CanController_RegisterReceiveMessageHandler(ib_CanController* self, void* context, void (*Callback)(void* context, ib_CanController* controller, ib_CanMessage* canFrameMeta))
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr || Callback == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->RegisterReceiveMessageHandler(
            [context, self, Callback](ib::sim::can::ICanController* /*ctrl*/, const ib::sim::can::CanMessage& msg)
            {
                ib_CanMessage cmm;
                ib_CanFrame cm{ 0,0,0, {(uint8_t*)msg.dataField.data(), (uint32_t)msg.dataField.size()} };
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

                Callback(context, self, &cmm);
            });
        return ib_ReturnCode_SUCCESS;
        )
}

ib_ReturnCode ib_CanController_RegisterTransmitStatusHandler(ib_CanController* self, void* context, void (*Callback)(void* context, ib_CanController* controller, ib_CanTransmitAcknowledge* cAck))
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr || Callback == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->RegisterTransmitStatusHandler(
            [Callback, context, self](ib::sim::can::ICanController* /*ctrl*/, const ib::sim::can::CanTransmitAcknowledge& ack)
            {
                
                
                auto transmitContext = pendingTransmits.userContextById[ack.transmitId];
                if (transmitContext == nullptr)
                {
                    std::unique_lock<std::mutex> lock(pendingTransmits.mutex);
                    pendingTransmits.callbacksById.insert(std::make_pair(ack.transmitId, [Callback, context, self, ack]() {
                        ib_CanTransmitAcknowledge tcack;
                        auto transmitContext = pendingTransmits.userContextById[ack.transmitId];
                        tcack.userContext = transmitContext;
                        pendingTransmits.userContextById.erase(ack.transmitId);
                        tcack.timestamp = ack.timestamp.count();
                        tcack.status = (ib_CanTransmitStatus)ack.status;
                        Callback(context, self, &tcack);
                        }));
                }
                else 
                {
                    ib_CanTransmitAcknowledge tcack;
                    tcack.userContext = transmitContext;
                    std::unique_lock<std::mutex> lock(pendingTransmits.mutex);
                    pendingTransmits.userContextById.erase(ack.transmitId);
                    tcack.timestamp = ack.timestamp.count();
                    tcack.status = (ib_CanTransmitStatus)ack.status;
                    Callback(context, self, &tcack);
                }
            });
        return ib_ReturnCode_SUCCESS;
        )
}

ib_ReturnCode ib_CanController_RegisterStateChangedHandler(ib_CanController* self, void* context, void (*Callback)(void* context, ib_CanController* controller, ib_CanControllerState state))
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr || Callback == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->RegisterStateChangedHandler(
            [Callback, context, self](ib::sim::can::ICanController* /*ctrl*/, const ib::sim::can::CanControllerState state)
            {
                Callback(context, self, (ib_CanControllerState)state);
            });
        return ib_ReturnCode_SUCCESS;
        )
}

ib_ReturnCode ib_CanController_RegisterErrorStateChangedHandler(ib_CanController* self, void* context, void (*Callback)(void* context, ib_CanController* controller, ib_CanErrorState state))
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr || Callback == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->RegisterErrorStateChangedHandler(
            [Callback, context, self](ib::sim::can::ICanController* /*ctrl*/, const ib::sim::can::CanErrorState state)
            {
                Callback(context, self, (ib_CanErrorState)state);
            });
        return ib_ReturnCode_SUCCESS;
        )
}

ib_ReturnCode ib_CanController_SetBaudRate(ib_CanController* self, uint32_t rate, uint32_t fdRate)
{
    CAPI_DEFINE_FUNC(
        if(self == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->SetBaudRate(rate, fdRate);
        return ib_ReturnCode_SUCCESS;
        )
}

ib_ReturnCode ib_CanController_SendFrame(ib_CanController* self, ib_CanFrame* message, void* transmitContext)
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr || message == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
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
        cm.dataField = std::vector<uint8_t>(&(message->data.pointer[0]), &(message->data.pointer[0]) + message->data.size);

        // ack queue is empty 
        auto transmitId = canController->SendMessage(std::move(cm)); // AckCallback -> fügt in queue hinzu weil er keine userContext findet
        std::unique_lock<std::mutex> lock(pendingTransmits.mutex);
        pendingTransmits.userContextById[transmitId] = transmitContext;
        for (auto pendingTransmitId : pendingTransmits.callbacksById)
        {
            pendingTransmitId.second();
        }
        pendingTransmits.callbacksById.clear();
        return ib_ReturnCode_SUCCESS;
        )
}

ib_ReturnCode ib_CanController_Start(ib_CanController* self)
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->Start();
        return ib_ReturnCode_SUCCESS;
        )
}

ib_ReturnCode ib_CanController_Stop(ib_CanController* self)
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->Stop();
        return ib_ReturnCode_SUCCESS;
        )
}

ib_ReturnCode ib_CanController_Reset(ib_CanController* self)
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->Reset();
        return ib_ReturnCode_SUCCESS;
        )
}

ib_ReturnCode ib_CanController_Sleep(ib_CanController* self)
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto canController = reinterpret_cast<ib::sim::can::ICanController*>(self);
        canController->Sleep();
        return ib_ReturnCode_SUCCESS;
        )
}


CIntegrationBusAPI ib_ReturnCode ib_CanController_create(ib_CanController** outCanController, ib_SimulationParticipant* self,  const char* cName)
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr || cName == nullptr || outCanController == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }

        std::string name(cName);
        auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(self);
        auto canController = comAdapter->CreateCanController(name);
        *outCanController = reinterpret_cast<ib_CanController*>(canController);
        return ib_ReturnCode_SUCCESS;
        )
}
#pragma endregion CAN

#pragma region ETHERNET
std::map<uint32_t, void*> ethernetTransmitContextMap;
std::map<uint32_t, int> ethernetTransmitContextMapCounter;
int transmitAckListeners = 0;

CIntegrationBusAPI ib_ReturnCode ib_EthernetController_create(ib_EthernetController** outController, ib_SimulationParticipant* participant, const char* name)
{
    CAPI_DEFINE_FUNC(
        if (outController == nullptr || participant == nullptr || name == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }

        std::string strName(name);
        auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
        auto ethernetController = comAdapter->CreateEthController(strName);
        *outController = reinterpret_cast<ib_EthernetController*>(ethernetController);
        return ib_ReturnCode_SUCCESS;
    )
}

ib_ReturnCode ib_EthernetController_Activate(ib_EthernetController* self)
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto controller = reinterpret_cast<ib::sim::eth::IEthController*>(self);
        controller->Activate();
        return ib_ReturnCode_SUCCESS;
    )
}

ib_ReturnCode ib_EthernetController_Deactivate(ib_EthernetController* self)
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto controller = reinterpret_cast<ib::sim::eth::IEthController*>(self);
        controller->Deactivate();
        return ib_ReturnCode_SUCCESS;
    )
}

ib_ReturnCode ib_EthernetController_RegisterReceiveMessageHandler(ib_EthernetController* self, void* context, ib_EthernetReceiveMessageHandler_t* handler)
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr || handler == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto controller = reinterpret_cast<ib::sim::eth::IEthController*>(self);
        controller->RegisterReceiveMessageHandler(
            [handler, context, self](ib::sim::eth::IEthController* /*ctrl*/, const ib::sim::eth::EthMessage& msg)
            {
                auto rawFrame = msg.ethFrame.RawFrame();
                
                uint8_t* dataPointer = 0;
                if (rawFrame.size() > 0)
                {
                    dataPointer = &(rawFrame[0]);
                }

                ib_EthernetMessage em;
                ib_EthernetFrame ef{ dataPointer, rawFrame.size() };

                em.ethernetFrame = &ef;
                em.interfaceId = ib_InterfaceIdentifier_EthernetFrame;
                em.timestamp = msg.timestamp.count();

                handler(context, self, &em);
            });
        return ib_ReturnCode_SUCCESS;
    )
}

ib_ReturnCode ib_EthernetController_RegisterFrameAckHandler(ib_EthernetController* self, void* context, ib_EthernetFrameAckHandler_t* handler)
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr || handler == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto controller = reinterpret_cast<ib::sim::eth::IEthController*>(self);
        transmitAckListeners += 1;
        controller->RegisterMessageAckHandler(
            [handler, context, self](ib::sim::eth::IEthController* controller, const ib::sim::eth::EthTransmitAcknowledge& ack)
            {
                ib_EthernetTransmitAcknowledge eta;

                eta.interfaceId = ib_InterfaceIdentifier_EthernetTransmitAcknowledge;
                eta.status = (ib_EthernetTransmitStatus)ack.status;
                eta.timestamp = ack.timestamp.count();

                auto transmitContext = ethernetTransmitContextMap[ack.transmitId];
                eta.userContext = transmitContext;
                ethernetTransmitContextMapCounter[ack.transmitId] += 1;
                if (ethernetTransmitContextMapCounter[ack.transmitId] >= transmitAckListeners)
                {
                    ethernetTransmitContextMap.erase(ack.transmitId);
                    ethernetTransmitContextMapCounter.erase(ack.transmitId);
                }
                

                handler(context, self, &eta);
            });
        return ib_ReturnCode_SUCCESS;
    )
}

ib_ReturnCode ib_EthernetController_RegisterStateChangedHandler(ib_EthernetController* self, void* context, ib_EthernetStateChangedHandler_t* handler)
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr || handler == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto controller = reinterpret_cast<ib::sim::eth::IEthController*>(self);
        controller->RegisterStateChangedHandler(
            [handler, context, self](ib::sim::eth::IEthController* controller, const ib::sim::eth::EthState& state)
            {
                ib_EthernetState cstate = (int32_t)state;

                handler(context, self, cstate);
            });
        return ib_ReturnCode_SUCCESS;
    )
}

ib_ReturnCode ib_EthernetController_RegisterBitRateChangedHandler(ib_EthernetController* self, void* context, ib_EthernetBitRateChangedHandler_t* handler)
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr || handler == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto controller = reinterpret_cast<ib::sim::eth::IEthController*>(self);
        controller->RegisterBitRateChangedHandler(
            [handler, context, self](ib::sim::eth::IEthController* controller, const uint32_t& bitrate)
            {
                handler(context, self, bitrate);
            });
        return ib_ReturnCode_SUCCESS;
    )
}

ib_ReturnCode ib_EthernetController_SendFrame(ib_EthernetController* self, ib_EthernetFrame* frame, void* userContext)
{
    CAPI_DEFINE_FUNC(
        if (self == nullptr || frame == nullptr)
        {
            ib_error_string = "A nullpointer parameter was passed to the function.";
            return ib_ReturnCode_BADPARAMETER;
        }
        using std::chrono::duration;
        auto controller = reinterpret_cast<ib::sim::eth::IEthController*>(self);

        ib::sim::eth::EthFrame ef;
        std::vector<uint8_t> rawFrame(frame->pointer, frame->pointer + frame->size);
        ef.SetRawFrame(rawFrame);
        auto transmitId = controller->SendFrame(ef);

        ethernetTransmitContextMap[transmitId] = userContext;
        ethernetTransmitContextMapCounter[transmitId] = 0;
        return ib_ReturnCode_SUCCESS;
    )
}

#pragma endregion ETHERNET


#pragma region DATA

CIntegrationBusAPI ib_ReturnCode ib_DataPublisher_create(ib_DataPublisher** out,
    ib_SimulationParticipant* participant, char* topic, ib_DataExchangeFormat* dataTypeInfo, uint8_t history)
{
    CAPI_DEFINE_FUNC(
        if (out == NULL)
        {
            ib_error_string = "The provided out pointer was null.";
            return ib_ReturnCode_BADPARAMETER;
        }
        if (participant == NULL || topic == NULL)
        {
            ib_error_string = "A nullpointer parameter was provided";
            return ib_ReturnCode_BADPARAMETER;
        }
        if (dataTypeInfo != NULL && (std::strcmp(dataTypeInfo->mediaType, "")))
        {
            ib_error_string = "This integration bus implementation currently does not support dataTypeInfos that are more specific then wildcards.";
            return ib_ReturnCode_NOTSUPPORTED;
        }
        if (history != 0)
        {
            ib_error_string = "This integration bus implementation currently only supports a history length of 0.";
            return ib_ReturnCode_NOTSUPPORTED;
        }

        std::string strTopic(topic);
        auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
        auto genericPublisher = comAdapter->CreateGenericPublisher(strTopic);
        *out = reinterpret_cast<ib_DataPublisher*>(genericPublisher);
        return ib_ReturnCode_SUCCESS;
    )
}


CIntegrationBusAPI ib_ReturnCode ib_DataSubscriber_create(ib_DataSubscriber** out,
    ib_SimulationParticipant* participant, char* topic, ib_DataExchangeFormat* dataTypeInfo, void* context,
    ib_DataHandler_t dataHandler)
{
    CAPI_DEFINE_FUNC(
        if (out == NULL)
        {
            ib_error_string = "The provided out pointer was null.";
            return ib_ReturnCode_BADPARAMETER;
        }
        if (participant == NULL || topic == NULL)
        {
            ib_error_string = "A nullpointer parameter was provided";
            return ib_ReturnCode_BADPARAMETER;
        }
        if (dataTypeInfo != NULL && (std::strcmp(dataTypeInfo->mediaType,"") != 0))
        {
            ib_error_string = "This integration bus implementation currently does not support data exchange formats that are more specific then wildcards.";
            return ib_ReturnCode_NOTSUPPORTED;
        }

        std::string strTopic(topic);
        auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
        auto genericSubscriber = comAdapter->CreateGenericSubscriber(strTopic);
        *out = reinterpret_cast<ib_DataSubscriber*>(genericSubscriber);

        // Register Data Handler if provided
        if (dataHandler != NULL)
        {
            ib_DataSubscriber_SetReceiveDataHandler(*out, context, dataHandler);
        }

        return ib_ReturnCode_SUCCESS;
    )
}

ib_ReturnCode ib_DataSubscriber_SetReceiveDataHandler(ib_DataSubscriber* self, void* context,
    ib_DataHandler_t dataHandler)
{
    CAPI_DEFINE_FUNC(
        if (self == NULL || dataHandler == NULL)
        {
            ib_error_string = "A nullpointer parameter was provided";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto subscriber = reinterpret_cast<ib::sim::generic::IGenericSubscriber*>(self);
        subscriber->SetReceiveMessageHandler(
            [dataHandler, context, self](ib::sim::generic::IGenericSubscriber* subscriber, const std::vector<uint8_t>& data)
            {
                uint8_t* payloadPointer = NULL;
                if (data.size() > 0)
                {
                    payloadPointer = (uint8_t* const)&(data[0]);
                }
                const ib_ByteVector ccdata{ payloadPointer, data.size() };

                dataHandler(context, self,  &ccdata);
            });
        return ib_ReturnCode_SUCCESS;
    )
}

ib_ReturnCode ib_DataPublisher_Publish(ib_DataPublisher* self, const ib_ByteVector* data)
{
    CAPI_DEFINE_FUNC(
        if (self == NULL || data == NULL)
        {
            ib_error_string = "A nullpointer parameter was provided";
            return ib_ReturnCode_BADPARAMETER;
        }
        auto publisher = reinterpret_cast<ib::sim::generic::IGenericPublisher*>(self);
        
        publisher->Publish(std::vector<uint8_t>(&(data->pointer[0]), &(data->pointer[0]) + data->size));
        return ib_ReturnCode_SUCCESS;
    )
}



#pragma endregion DATA

}
