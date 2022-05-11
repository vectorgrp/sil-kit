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

ib_ReturnCode ib_Can_Controller_Create(ib_Can_Controller** outController, ib_Participant* participant, const char* cName, const char* cNetwork)
{
  ASSERT_VALID_OUT_PARAMETER(outController);
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_POINTER_PARAMETER(cName);
  ASSERT_VALID_POINTER_PARAMETER(cNetwork);
  CAPI_ENTER
  {
    std::string name(cName);
    std::string network(cNetwork);
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    auto canController = cppParticipant->CreateCanController(name, network);
    *outController = reinterpret_cast<ib_Can_Controller*>(canController);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Can_Controller_AddFrameHandler(ib_Can_Controller* controller, void* context,
                                                ib_Can_FrameHandler_t callback, ib_Direction directionMask)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(callback);
  CAPI_ENTER
  {
    auto canController = reinterpret_cast<ib::sim::can::ICanController*>(controller);
    canController->AddFrameHandler(
      [context, controller, callback](ib::sim::can::ICanController* /*ctrl*/, const ib::sim::can::CanFrameEvent& cppCanFrameEvent)
      {
        ib_Can_Frame frame{};
        frame.id = cppCanFrameEvent.frame.canId;
        uint32_t flags = 0;
        flags |= cppCanFrameEvent.frame.flags.ide ? ib_Can_FrameFlag_ide : 0;
        flags |= cppCanFrameEvent.frame.flags.rtr ? ib_Can_FrameFlag_rtr : 0;
        flags |= cppCanFrameEvent.frame.flags.fdf ? ib_Can_FrameFlag_fdf : 0;
        flags |= cppCanFrameEvent.frame.flags.brs ? ib_Can_FrameFlag_brs : 0;
        flags |= cppCanFrameEvent.frame.flags.esi ? ib_Can_FrameFlag_esi : 0;
        frame.flags = flags;
        frame.dlc = cppCanFrameEvent.frame.dlc;
        frame.data = { (uint8_t*)cppCanFrameEvent.frame.dataField.data(), (uint32_t)cppCanFrameEvent.frame.dataField.size() };

        ib_Can_FrameEvent frameEvent{};
        frameEvent.interfaceId = ib_InterfaceIdentifier_CanFrameEvent;
        frameEvent.timestamp = cppCanFrameEvent.timestamp.count();
        frameEvent.interfaceId = ib_InterfaceIdentifier_CanFrameEvent;
        frameEvent.frame = &frame;
        frameEvent.userContext = cppCanFrameEvent.frame.userContext;

        callback(context, controller, &frameEvent);
      }, directionMask);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Can_Controller_AddFrameTransmitHandler(ib_Can_Controller* controller, void* context, ib_Can_FrameTransmitHandler_t callback, ib_Can_TransmitStatus statusMask)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(callback);
  CAPI_ENTER
  {
    auto canController = reinterpret_cast<ib::sim::can::ICanController*>(controller);
    canController->AddFrameTransmitHandler(
      [callback, context, controller](ib::sim::can::ICanController* /*ctrl*/, const ib::sim::can::CanFrameTransmitEvent& cppFrameTransmitEvent)
      {
        ib_Can_FrameTransmitEvent frameTransmitEvent{};
        frameTransmitEvent.interfaceId = ib_InterfaceIdentifier_CanFrameTransmitEvent;
        frameTransmitEvent.userContext = cppFrameTransmitEvent.userContext;
        frameTransmitEvent.timestamp = cppFrameTransmitEvent.timestamp.count();
        frameTransmitEvent.status = (ib_Can_TransmitStatus)cppFrameTransmitEvent.status;
        callback(context, controller, &frameTransmitEvent);
      }, static_cast<ib::sim::can::CanTransmitStatusMask>(statusMask));
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Can_Controller_AddStateChangeHandler(ib_Can_Controller* controller, void* context,
                                                      ib_Can_StateChangeHandler_t callback)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(callback);
  CAPI_ENTER
  {
    auto canController = reinterpret_cast<ib::sim::can::ICanController*>(controller);
    canController->AddStateChangeHandler(
        [callback, context, controller](ib::sim::can::ICanController* /*ctrl*/,
                                        const ib::sim::can::CanStateChangeEvent cppStateChangeEvent)
      {
        ib_Can_StateChangeEvent stateChangeEvent;
        stateChangeEvent.interfaceId = ib_InterfaceIdentifier_CanStateChangeEvent;
        stateChangeEvent.timestamp = cppStateChangeEvent.timestamp.count();
        stateChangeEvent.state = (ib_Can_ControllerState)cppStateChangeEvent.state;
        callback(context, controller, stateChangeEvent);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Can_Controller_AddErrorStateChangeHandler(ib_Can_Controller* controller, void* context, ib_Can_ErrorStateChangeHandler_t callback)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(callback);
  CAPI_ENTER
  {
    auto canController = reinterpret_cast<ib::sim::can::ICanController*>(controller);
    canController->AddErrorStateChangeHandler(
        [callback, context, controller](ib::sim::can::ICanController* /*ctrl*/,
                                        const ib::sim::can::CanErrorStateChangeEvent cppErrorStateChangeEvent)
      {
        ib_Can_ErrorStateChangeEvent errorStateChangeEvent;
        errorStateChangeEvent.interfaceId = ib_InterfaceIdentifier_CanErrorStateChangeEvent;
        errorStateChangeEvent.timestamp = cppErrorStateChangeEvent.timestamp.count();
        errorStateChangeEvent.errorState = (ib_Can_ErrorState)cppErrorStateChangeEvent.errorState;
        callback(context, controller, errorStateChangeEvent);
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

    ib::sim::can::CanFrame frame{};
    frame.canId = message->id;
    frame.flags.ide = message->flags & ib_Can_FrameFlag_ide ? 1 : 0;
    frame.flags.rtr = message->flags & ib_Can_FrameFlag_rtr ? 1 : 0;
    frame.flags.fdf = message->flags & ib_Can_FrameFlag_fdf ? 1 : 0;
    frame.flags.brs = message->flags & ib_Can_FrameFlag_brs ? 1 : 0;
    frame.flags.esi = message->flags & ib_Can_FrameFlag_esi ? 1 : 0;

    frame.dlc = message->dlc;
    frame.dataField = std::vector<uint8_t>(&(message->data.data[0]), &(message->data.data[0]) + message->data.size);

    canController->SendFrame(std::move(frame), transmitContext);
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

