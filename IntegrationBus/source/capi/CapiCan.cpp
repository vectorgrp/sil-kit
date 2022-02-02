/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#include <string>
//#include <iostream>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstring>
#include <sstream>

#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "CapiImpl.h"
#include "ib/sim/can/all.hpp"

struct PendingTransmits {
  std::map<uint32_t, void*> userContextById;
  std::map<uint32_t, std::function<void()>> callbacksById;
  std::mutex mutex;
};
PendingTransmits pendingTransmits;

IntegrationBusAPI ib_ReturnCode ib_Can_Controller_Create(ib_Can_Controller** outController, ib_SimulationParticipant* participant, const char* cName)
{
  ASSERT_VALID_OUT_PARAMETER(outController);
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_POINTER_PARAMETER(cName);
  CAPI_ENTER
  {
    std::string name(cName);
    auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
    auto canController = comAdapter->CreateCanController(name);
    *outController = reinterpret_cast<ib_Can_Controller*>(canController);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Can_Controller_RegisterReceiveMessageHandler(ib_Can_Controller* controller, void* context, ib_Can_ReceiveMessageHandler_t callback, ib_Direction directionMask)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(callback);
  CAPI_ENTER
  {
    auto canController = reinterpret_cast<ib::sim::can::ICanController*>(controller);
    canController->RegisterReceiveMessageHandler(
      [context, controller, callback](ib::sim::can::ICanController* /*ctrl*/, const ib::sim::can::CanMessage& msg)
      {
        ib_Can_Message cmm;
        ib_Can_Frame cm{ 0,0,0, {(uint8_t*)msg.dataField.data(), (uint32_t)msg.dataField.size()} };
        cmm.timestamp = msg.timestamp.count();
        cmm.interfaceId = ib_InterfaceIdentifier_CanFrame_Meta;
        cmm.canFrame = &cm;
        cmm.userContext = msg.userContext;

        cm.id = msg.canId;
        uint32_t flags = 0;
        flags |= msg.flags.ide ? ib_Can_FrameFlag_ide : 0;
        flags |= msg.flags.rtr ? ib_Can_FrameFlag_rtr : 0;
        flags |= msg.flags.fdf ? ib_Can_FrameFlag_fdf : 0;
        flags |= msg.flags.brs ? ib_Can_FrameFlag_brs : 0;
        flags |= msg.flags.esi ? ib_Can_FrameFlag_esi : 0;
        cm.flags = flags;
        cm.dlc = msg.dlc;

        callback(context, controller, &cmm);
      }, directionMask);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Can_Controller_RegisterTransmitStatusHandler(ib_Can_Controller* controller, void* context, ib_Can_TransmitStatusHandler_t callback, ib_Can_TransmitStatus statusMask)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(callback);
  CAPI_ENTER
  {
    auto canController = reinterpret_cast<ib::sim::can::ICanController*>(controller);
    canController->RegisterTransmitStatusHandler(
      [callback, context, controller](ib::sim::can::ICanController* /*ctrl*/, const ib::sim::can::CanTransmitAcknowledge& ack)
      {
        ib_Can_TransmitAcknowledge tcack;
        tcack.userContext = ack.userContext;
        tcack.timestamp = ack.timestamp.count();
        tcack.status = (ib_Can_TransmitStatus)ack.status;
        callback(context, controller, &tcack);
      }, statusMask);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Can_Controller_RegisterStateChangedHandler(ib_Can_Controller* controller, void* context, ib_Can_StateChangedHandler_t callback)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(callback);
  CAPI_ENTER
  {
    auto canController = reinterpret_cast<ib::sim::can::ICanController*>(controller);
    canController->RegisterStateChangedHandler(
      [callback, context, controller](ib::sim::can::ICanController* /*ctrl*/, const ib::sim::can::CanControllerState state)
      {
        callback(context, controller, (ib_Can_ControllerState)state);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Can_Controller_RegisterErrorStateChangedHandler(ib_Can_Controller* controller, void* context, ib_Can_ErrorStateChangedHandler_t callback)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(callback);
  CAPI_ENTER
  {
    auto canController = reinterpret_cast<ib::sim::can::ICanController*>(controller);
    canController->RegisterErrorStateChangedHandler(
      [callback, context, controller](ib::sim::can::ICanController* /*ctrl*/, const ib::sim::can::CanErrorState state)
      {
          callback(context, controller, (ib_Can_ErrorState)state);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Can_Controller_SetBaudRate(ib_Can_Controller* controller, uint32_t rate, uint32_t fdRate)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  CAPI_ENTER
  {
    auto canController = reinterpret_cast<ib::sim::can::ICanController*>(controller);
    canController->SetBaudRate(rate, fdRate);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Can_Controller_SendFrame(ib_Can_Controller* controller, ib_Can_Frame* message, void* transmitContext)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_POINTER_PARAMETER(message);
  CAPI_ENTER
  {
    using std::chrono::duration;
    auto canController = reinterpret_cast<ib::sim::can::ICanController*>(controller);

    ib::sim::can::CanMessage cm{};
    cm.timestamp = {};
    cm.canId = message->id;
    cm.flags.ide = message->flags & ib_Can_FrameFlag_ide ? 1 : 0;
    cm.flags.rtr = message->flags & ib_Can_FrameFlag_rtr ? 1 : 0;
    cm.flags.fdf = message->flags & ib_Can_FrameFlag_fdf ? 1 : 0;
    cm.flags.brs = message->flags & ib_Can_FrameFlag_brs ? 1 : 0;
    cm.flags.esi = message->flags & ib_Can_FrameFlag_esi ? 1 : 0;

    static int msgId = 0;
    std::stringstream payloadBuilder;
    payloadBuilder << "CAN " << msgId++;
    auto payloadStr = payloadBuilder.str();

    cm.dlc = message->dlc;
    cm.dataField = std::vector<uint8_t>(&(message->data.pointer[0]), &(message->data.pointer[0]) + message->data.size);

    // ack queue is empty 
    auto transmitId = canController->SendMessage(std::move(cm), transmitContext); // AckCallback -> fügt in queue hinzu weil er keine userContext findet
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Can_Controller_Start(ib_Can_Controller* controller)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  CAPI_ENTER
  {
    auto canController = reinterpret_cast<ib::sim::can::ICanController*>(controller);
    canController->Start();
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Can_Controller_Stop(ib_Can_Controller* controller)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  CAPI_ENTER
  {
    auto canController = reinterpret_cast<ib::sim::can::ICanController*>(controller);
    canController->Stop();
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Can_Controller_Reset(ib_Can_Controller* controller)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  CAPI_ENTER
  {
    auto canController = reinterpret_cast<ib::sim::can::ICanController*>(controller);
    canController->Reset();
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Can_Controller_Sleep(ib_Can_Controller* controller)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  CAPI_ENTER
  {
    auto canController = reinterpret_cast<ib::sim::can::ICanController*>(controller);
    canController->Sleep();
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

