// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/eth/all.hpp"
#include "ib/sim/eth/string_utils.hpp"

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

ib_ReturnCode ib_Ethernet_Controller_Create(ib_Ethernet_Controller** outController, ib_Participant* participant,
                                            const char* name, const char* network)
{
    ASSERT_VALID_OUT_PARAMETER(outController);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(name);
    ASSERT_VALID_POINTER_PARAMETER(network);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
        auto ethernetController = cppParticipant->CreateEthernetController(name, network);
        *outController = reinterpret_cast<ib_Ethernet_Controller*>(ethernetController);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_Activate(ib_Ethernet_Controller* controller)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);
        cppController->Activate();
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_Deactivate(ib_Ethernet_Controller* controller)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);
        cppController->Deactivate();
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_AddFrameHandler(ib_Ethernet_Controller* controller, void* context,
                                                     ib_Ethernet_FrameHandler_t handler, ib_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);
        auto cppHandlerId = cppController->AddFrameHandler(
            [handler, context, controller](auto* /*ctrl*/,
                                           const auto& cppFrameEvent) {
                auto& cppFrame = cppFrameEvent.frame;
                auto* dataPointer = !cppFrame.raw.empty() ? cppFrame.raw.data() : nullptr;

                ib_Ethernet_FrameEvent frameEvent{};
                ib_Ethernet_Frame frame{ib_InterfaceIdentifier_EthernetFrame, {dataPointer, cppFrame.raw.size()}};

                frameEvent.interfaceId = ib_InterfaceIdentifier_EthernetFrameEvent;
                frameEvent.ethernetFrame = &frame;
                frameEvent.timestamp = cppFrameEvent.timestamp.count();

                handler(context, controller, &frameEvent);
            });
        *outHandlerId = static_cast<ib_HandlerId>(cppHandlerId);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_RemoveFrameHandler(ib_Ethernet_Controller* controller, ib_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);
        cppController->RemoveFrameHandler(static_cast<ib::util::HandlerId>(handlerId));
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_AddFrameTransmitHandler(ib_Ethernet_Controller* controller, void* context,
                                                             ib_Ethernet_FrameTransmitHandler_t handler,
                                                             ib_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);
        auto cppHandlerId = cppController->AddFrameTransmitHandler(
            [handler, context, controller](auto*,
                                           const auto& ack) {
                std::unique_lock<std::mutex> lock(pendingEthernetTransmits.mutex);

                auto transmitContext = pendingEthernetTransmits.userContextById[ack.transmitId];
                if (transmitContext == nullptr)
                {
                    pendingEthernetTransmits.callbacksById[ack.transmitId] = [handler, context, controller, ack]() {
                        ib_Ethernet_FrameTransmitEvent eta;
                        eta.interfaceId = ib_InterfaceIdentifier_EthernetFrameTransmitEvent;
                        eta.status = (ib_Ethernet_TransmitStatus)ack.status;
                        eta.timestamp = ack.timestamp.count();

                        auto tmpContext = pendingEthernetTransmits.userContextById[ack.transmitId];
                        pendingEthernetTransmits.userContextById.erase(ack.transmitId);
                        eta.userContext = tmpContext;

                        handler(context, controller, &eta);
                    };
                }
                else
                {
                    ib_Ethernet_FrameTransmitEvent eta;
                    eta.interfaceId = ib_InterfaceIdentifier_EthernetFrameTransmitEvent;
                    eta.status = (ib_Ethernet_TransmitStatus)ack.status;
                    eta.timestamp = ack.timestamp.count();

                    auto tmpContext = pendingEthernetTransmits.userContextById[ack.transmitId];
                    pendingEthernetTransmits.userContextById.erase(ack.transmitId);
                    eta.userContext = tmpContext;

                    handler(context, controller, &eta);
                }
            });
        *outHandlerId = static_cast<ib_HandlerId>(cppHandlerId);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}
ib_ReturnCode ib_Ethernet_Controller_RemoveFrameTransmitHandler(ib_Ethernet_Controller* controller,
                                                                ib_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);
        cppController->RemoveFrameTransmitHandler(static_cast<ib::util::HandlerId>(handlerId));
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_AddStateChangeHandler(ib_Ethernet_Controller* controller, void* context,
                                                           ib_Ethernet_StateChangeHandler_t handler,
                                                           ib_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);
        auto cppHandlerId = cppController->AddStateChangeHandler(
            [handler, context, controller](ib::sim::eth::IEthernetController*,
                                           const ib::sim::eth::EthernetStateChangeEvent& stateChangeEvent) {
                ib_Ethernet_StateChangeEvent cStateChangeEvent;
                cStateChangeEvent.interfaceId = ib_InterfaceIdentifier_EthernetStateChangeEvent;
                cStateChangeEvent.timestamp = stateChangeEvent.timestamp.count();
                cStateChangeEvent.state = (ib_Ethernet_State)stateChangeEvent.state;
                handler(context, controller, &cStateChangeEvent);
            });
        *outHandlerId = static_cast<ib_HandlerId>(cppHandlerId);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}
ib_ReturnCode ib_Ethernet_Controller_RemoveStateChangeHandler(ib_Ethernet_Controller* controller,
                                                              ib_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);
        cppController->RemoveStateChangeHandler(static_cast<ib::util::HandlerId>(handlerId));
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_AddBitrateChangeHandler(ib_Ethernet_Controller* controller, void* context,
                                                             ib_Ethernet_BitrateChangeHandler_t handler,
                                                             ib_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);
        auto cppHandlerId = cppController->AddBitrateChangeHandler(
            [handler, context, controller](ib::sim::eth::IEthernetController*,
                                           const ib::sim::eth::EthernetBitrateChangeEvent& bitrateChangeEvent) {
                ib_Ethernet_BitrateChangeEvent cBitrateChangeEvent;
                cBitrateChangeEvent.interfaceId = ib_InterfaceIdentifier_EthernetBitrateChangeEvent;
                cBitrateChangeEvent.timestamp = bitrateChangeEvent.timestamp.count();
                cBitrateChangeEvent.bitrate = (ib_Ethernet_Bitrate)bitrateChangeEvent.bitrate;

                handler(context, controller, &cBitrateChangeEvent);
            });
        *outHandlerId = static_cast<ib_HandlerId>(cppHandlerId);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}
ib_ReturnCode ib_Ethernet_Controller_RemoveBitrateChangeHandler(ib_Ethernet_Controller* controller,
                                                                ib_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);
        cppController->RemoveBitrateChangeHandler(static_cast<ib::util::HandlerId>(handlerId));
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_SendFrame(ib_Ethernet_Controller* controller, ib_Ethernet_Frame* frame,
                                               void* userContext)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(frame);
    CAPI_ENTER
    {
        if (frame->raw.size < ETHERNET_MIN_FRAME_SIZE)
        {
            ib_error_string = "An ethernet frame must be at least 60 bytes in size.";
            return ib_ReturnCode_BADPARAMETER;
        }
        using std::chrono::duration;
        auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);

        ib::sim::eth::EthernetFrame ef;
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
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}
