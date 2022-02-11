/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

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
#include "CapiImpl.h"

#pragma region ETHERNET
std::map<uint32_t, void*> ethernetTransmitContextMap;
std::map<uint32_t, int> ethernetTransmitContextMapCounter;
int transmitAckListeners = 0;

struct PendingEthernetTransmits {
    std::map<uint32_t, void*> userContextById;
    std::map<uint32_t, std::function<void()>> callbacksById;
    std::mutex mutex;
};
PendingEthernetTransmits pendingEthernetTransmits;

#define ETHERNET_MIN_FRAME_SIZE 60

IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_Create(ib_Ethernet_Controller** outController, ib_SimulationParticipant* participant, const char* name)
{
  ASSERT_VALID_OUT_PARAMETER(outController);
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_POINTER_PARAMETER(name);
  CAPI_ENTER
  {
    std::string strName(name);
    auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
    auto ethernetController = comAdapter->CreateEthController(strName);
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
    auto cppController = reinterpret_cast<ib::sim::eth::IEthController*>(controller);
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
    auto cppController = reinterpret_cast<ib::sim::eth::IEthController*>(controller);
    cppController->Deactivate();
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_RegisterReceiveMessageHandler(ib_Ethernet_Controller* controller, void* context, ib_Ethernet_ReceiveMessageHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    auto cppController = reinterpret_cast<ib::sim::eth::IEthController*>(controller);
    cppController->RegisterReceiveMessageHandler(
      [handler, context, controller](ib::sim::eth::IEthController* /*ctrl*/, const ib::sim::eth::EthMessage& msg)
      {
        auto rawFrame = msg.ethFrame.RawFrame();
                
        uint8_t* dataPointer = 0;
        if (rawFrame.size() > 0)
        {
          dataPointer = &(rawFrame[0]);
        }

        ib_Ethernet_Message em;
        ib_Ethernet_Frame ef{ dataPointer, rawFrame.size() };

        em.ethernetFrame = &ef;
        em.interfaceId = ib_InterfaceIdentifier_EthernetFrame;
        em.timestamp = msg.timestamp.count();

        handler(context, controller, &em);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_RegisterFrameAckHandler(ib_Ethernet_Controller* controller, void* context, ib_Ethernet_FrameAckHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    auto cppController = reinterpret_cast<ib::sim::eth::IEthController*>(controller);
    cppController->RegisterMessageAckHandler(
      [handler, context, controller](ib::sim::eth::IEthController* cppController, const ib::sim::eth::EthTransmitAcknowledge& ack)
      {
        std::unique_lock<std::mutex> lock(pendingEthernetTransmits.mutex);

        auto transmitContext = pendingEthernetTransmits.userContextById[ack.transmitId];
        if (transmitContext == nullptr)
        {
          pendingEthernetTransmits.callbacksById[ack.transmitId] =
            [handler, context, controller, ack]()
            {
              ib_Ethernet_TransmitAcknowledge eta;
              eta.interfaceId = ib_InterfaceIdentifier_EthernetTransmitAcknowledge;
              eta.status = (ib_Ethernet_TransmitStatus)ack.status;
              eta.timestamp = ack.timestamp.count();

              auto transmitContext = pendingEthernetTransmits.userContextById[ack.transmitId];
              pendingEthernetTransmits.userContextById.erase(ack.transmitId);
              eta.userContext = transmitContext;

              handler(context, controller, &eta);
            };
        }
        else
        {
          ib_Ethernet_TransmitAcknowledge eta;
          eta.interfaceId = ib_InterfaceIdentifier_EthernetTransmitAcknowledge;
          eta.status = (ib_Ethernet_TransmitStatus)ack.status;
          eta.timestamp = ack.timestamp.count();

          auto transmitContext = pendingEthernetTransmits.userContextById[ack.transmitId];
          pendingEthernetTransmits.userContextById.erase(ack.transmitId);
          eta.userContext = transmitContext;

          handler(context, controller, &eta);
        }
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_RegisterStateChangedHandler(ib_Ethernet_Controller* controller, void* context, ib_Ethernet_StateChangedHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    auto cppController = reinterpret_cast<ib::sim::eth::IEthController*>(controller);
    cppController->RegisterStateChangedHandler(
      [handler, context, controller](ib::sim::eth::IEthController* cppController, const ib::sim::eth::EthState& state)
      {
        ib_Ethernet_State cstate = (ib_Ethernet_State)state;
        handler(context, controller, cstate);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_RegisterBitRateChangedHandler(ib_Ethernet_Controller* controller, void* context, ib_Ethernet_BitRateChangedHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    auto cppController = reinterpret_cast<ib::sim::eth::IEthController*>(controller);
    cppController->RegisterBitRateChangedHandler(
      [handler, context, controller](ib::sim::eth::IEthController* cppController, const uint32_t& bitrate)
      {
          handler(context, controller, bitrate);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_SendFrame(ib_Ethernet_Controller* controller, ib_Ethernet_Frame* frame, void* userContext)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_POINTER_PARAMETER(frame);
  CAPI_ENTER
  {
    if (frame->size < ETHERNET_MIN_FRAME_SIZE) {
      ib_error_string = "An ethernet frame must be at least 60 bytes in size.";
      return ib_ReturnCode_BADPARAMETER;
    }
    using std::chrono::duration;
    auto cppController = reinterpret_cast<ib::sim::eth::IEthController*>(controller);

    ib::sim::eth::EthFrame ef;
    std::vector<uint8_t> rawFrame(frame->data, frame->data + frame->size);
    ef.SetRawFrame(rawFrame);
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

#pragma endregion ETHERNET

