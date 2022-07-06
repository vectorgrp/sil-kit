// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/core/logging/ILogger.hpp"
#include "silkit/core/sync/all.hpp"
#include "silkit/core/sync/string_utils.hpp"
#include "silkit/services/eth/all.hpp"
#include "silkit/services/eth/string_utils.hpp"

#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstring>
#include "CapiImpl.hpp"

std::map<uint32_t, void*> ethernetTransmitContextMap;
std::map<uint32_t, int> ethernetTransmitContextMapCounter;
int transmitAckListeners = 0;

struct PendingEthernetTransmits
{
    std::map<uint32_t, void*> userContextById;
    std::map<uint32_t, std::function<void()>> callbacksById;
    std::mutex mutex;
};
PendingEthernetTransmits pendingEthernetTransmits;

#define ETHERNET_MIN_FRAME_SIZE 60

SilKit_ReturnCode SilKit_EthernetController_Create(SilKit_EthernetController** outController, SilKit_Participant* participant,
                                            const char* name, const char* network)
{
    ASSERT_VALID_OUT_PARAMETER(outController);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(name);
    ASSERT_VALID_POINTER_PARAMETER(network);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto ethernetController = cppParticipant->CreateEthernetController(name, network);
        *outController = reinterpret_cast<SilKit_EthernetController*>(ethernetController);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_EthernetController_Activate(SilKit_EthernetController* controller)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<SilKit::Services::Ethernet::IEthernetController*>(controller);
        cppController->Activate();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_EthernetController_Deactivate(SilKit_EthernetController* controller)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<SilKit::Services::Ethernet::IEthernetController*>(controller);
        cppController->Deactivate();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_EthernetController_AddFrameHandler(SilKit_EthernetController* controller, void* context,
                                                     SilKit_EthernetFrameHandler_t handler, SilKit_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<SilKit::Services::Ethernet::IEthernetController*>(controller);
        auto cppHandlerId = cppController->AddFrameHandler(
            [handler, context, controller](auto* /*ctrl*/,
                                           const auto& cppFrameEvent) {
                auto& cppFrame = cppFrameEvent.frame;
                auto* dataPointer = !cppFrame.raw.empty() ? cppFrame.raw.data() : nullptr;

                SilKit_EthernetFrameEvent frameEvent{};
                SilKit_EthernetFrame frame{SilKit_InterfaceIdentifier_EthernetFrame, {dataPointer, cppFrame.raw.size()}};

                frameEvent.interfaceId = SilKit_InterfaceIdentifier_EthernetFrameEvent;
                frameEvent.ethernetFrame = &frame;
                frameEvent.timestamp = cppFrameEvent.timestamp.count();

                handler(context, controller, &frameEvent);
            });
        *outHandlerId = static_cast<SilKit_HandlerId>(cppHandlerId);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_EthernetController_RemoveFrameHandler(SilKit_EthernetController* controller, SilKit_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<SilKit::Services::Ethernet::IEthernetController*>(controller);
        cppController->RemoveFrameHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_EthernetController_AddFrameTransmitHandler(SilKit_EthernetController* controller, void* context,
                                                             SilKit_EthernetFrameTransmitHandler_t handler,
                                                             SilKit_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<SilKit::Services::Ethernet::IEthernetController*>(controller);
        auto cppHandlerId = cppController->AddFrameTransmitHandler(
            [handler, context, controller](auto*,
                                           const auto& ack) {
                std::unique_lock<std::mutex> lock(pendingEthernetTransmits.mutex);

                auto transmitContext = pendingEthernetTransmits.userContextById[ack.transmitId];
                if (transmitContext == nullptr)
                {
                    pendingEthernetTransmits.callbacksById[ack.transmitId] = [handler, context, controller, ack]() {
                        SilKit_EthernetFrameTransmitEvent eta;
                        eta.interfaceId = SilKit_InterfaceIdentifier_EthernetFrameTransmitEvent;
                        eta.status = (SilKit_EthernetTransmitStatus)ack.status;
                        eta.timestamp = ack.timestamp.count();

                        auto tmpContext = pendingEthernetTransmits.userContextById[ack.transmitId];
                        pendingEthernetTransmits.userContextById.erase(ack.transmitId);
                        eta.userContext = tmpContext;

                        handler(context, controller, &eta);
                    };
                }
                else
                {
                    SilKit_EthernetFrameTransmitEvent eta;
                    eta.interfaceId = SilKit_InterfaceIdentifier_EthernetFrameTransmitEvent;
                    eta.status = (SilKit_EthernetTransmitStatus)ack.status;
                    eta.timestamp = ack.timestamp.count();

                    auto tmpContext = pendingEthernetTransmits.userContextById[ack.transmitId];
                    pendingEthernetTransmits.userContextById.erase(ack.transmitId);
                    eta.userContext = tmpContext;

                    handler(context, controller, &eta);
                }
            });
        *outHandlerId = static_cast<SilKit_HandlerId>(cppHandlerId);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}
SilKit_ReturnCode SilKit_EthernetController_RemoveFrameTransmitHandler(SilKit_EthernetController* controller,
                                                                SilKit_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<SilKit::Services::Ethernet::IEthernetController*>(controller);
        cppController->RemoveFrameTransmitHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_EthernetController_AddStateChangeHandler(SilKit_EthernetController* controller, void* context,
                                                           SilKit_EthernetStateChangeHandler_t handler,
                                                           SilKit_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<SilKit::Services::Ethernet::IEthernetController*>(controller);
        auto cppHandlerId = cppController->AddStateChangeHandler(
            [handler, context, controller](SilKit::Services::Ethernet::IEthernetController*,
                                           const SilKit::Services::Ethernet::EthernetStateChangeEvent& stateChangeEvent) {
                SilKit_EthernetStateChangeEvent cStateChangeEvent;
                cStateChangeEvent.interfaceId = SilKit_InterfaceIdentifier_EthernetStateChangeEvent;
                cStateChangeEvent.timestamp = stateChangeEvent.timestamp.count();
                cStateChangeEvent.state = (SilKit_EthernetState)stateChangeEvent.state;
                handler(context, controller, &cStateChangeEvent);
            });
        *outHandlerId = static_cast<SilKit_HandlerId>(cppHandlerId);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}
SilKit_ReturnCode SilKit_EthernetController_RemoveStateChangeHandler(SilKit_EthernetController* controller,
                                                              SilKit_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<SilKit::Services::Ethernet::IEthernetController*>(controller);
        cppController->RemoveStateChangeHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_EthernetController_AddBitrateChangeHandler(SilKit_EthernetController* controller, void* context,
                                                             SilKit_EthernetBitrateChangeHandler_t handler,
                                                             SilKit_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<SilKit::Services::Ethernet::IEthernetController*>(controller);
        auto cppHandlerId = cppController->AddBitrateChangeHandler(
            [handler, context, controller](SilKit::Services::Ethernet::IEthernetController*,
                                           const SilKit::Services::Ethernet::EthernetBitrateChangeEvent& bitrateChangeEvent) {
                SilKit_EthernetBitrateChangeEvent cBitrateChangeEvent;
                cBitrateChangeEvent.interfaceId = SilKit_InterfaceIdentifier_EthernetBitrateChangeEvent;
                cBitrateChangeEvent.timestamp = bitrateChangeEvent.timestamp.count();
                cBitrateChangeEvent.bitrate = (SilKit_EthernetBitrate)bitrateChangeEvent.bitrate;

                handler(context, controller, &cBitrateChangeEvent);
            });
        *outHandlerId = static_cast<SilKit_HandlerId>(cppHandlerId);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}
SilKit_ReturnCode SilKit_EthernetController_RemoveBitrateChangeHandler(SilKit_EthernetController* controller,
                                                                SilKit_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<SilKit::Services::Ethernet::IEthernetController*>(controller);
        cppController->RemoveBitrateChangeHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_EthernetController_SendFrame(SilKit_EthernetController* controller, SilKit_EthernetFrame* frame,
                                               void* userContext)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(frame);
    CAPI_ENTER
    {
        if (frame->raw.size < ETHERNET_MIN_FRAME_SIZE)
        {
            SilKit_error_string = "An ethernet frame must be at least 60 bytes in size.";
            return SilKit_ReturnCode_BADPARAMETER;
        }
        using std::chrono::duration;
        auto cppController = reinterpret_cast<SilKit::Services::Ethernet::IEthernetController*>(controller);

        SilKit::Services::Ethernet::EthernetFrame ef;
        std::vector<uint8_t> rawFrame(frame->raw.data, frame->raw.data + frame->raw.size);
        ef.raw = rawFrame;
        auto transmitId = cppController->SendFrame(ef);

        std::unique_lock<std::mutex> lock(pendingEthernetTransmits.mutex);
        pendingEthernetTransmits.userContextById[transmitId] = userContext;
        for (auto pendingTransmitId : pendingEthernetTransmits.callbacksById)
        {
            pendingTransmitId.second();
        }
        pendingEthernetTransmits.callbacksById.clear();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}
