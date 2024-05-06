/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstring>

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "CapiImpl.hpp"

#include "silkit/experimental/netsim/all.hpp"

#include "silkit/detail/impl/HourglassConversions.hpp"
#include "participant/ParticipantExtensionsImpl.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {

namespace Can {

// --------------------------------
// CAN
// --------------------------------

class CApiSimulatedCanController : public ISimulatedCanController
{
    void* _userController{nullptr};
    const SilKit_Experimental_SimulatedCanControllerFunctions* _functions{nullptr};

public:
    CApiSimulatedCanController() = default;

    CApiSimulatedCanController(void* controller, const SilKit_Experimental_SimulatedCanControllerFunctions* functions)
        : _userController{controller}
        , _functions{functions}
    {
    }

    void OnSetControllerMode(const CanControllerMode& cxxMsg) override
    {
        SilKit_Experimental_NetSim_CanControllerMode cMsg;
        SilKit_Struct_Init(SilKit_Experimental_NetSim_CanControllerMode, cMsg);

        assignCxxToC(cxxMsg, cMsg);

        _functions->OnSetControllerMode(_userController, &cMsg);
    }

    void OnSetBaudrate(const CanConfigureBaudrate& cxxMsg) override
    {
        SilKit_Experimental_NetSim_CanConfigureBaudrate cMsg;
        SilKit_Struct_Init(SilKit_Experimental_NetSim_CanConfigureBaudrate, cMsg);
        assignCxxToC(cxxMsg, cMsg);

        _functions->OnSetBaudrate(_userController, &cMsg);
    }

    void OnFrameRequest(const CanFrameRequest& cxxMsg) override
    {
        SilKit_Experimental_NetSim_CanFrameRequest cMsg;
        SilKit_Struct_Init(SilKit_Experimental_NetSim_CanFrameRequest, cMsg);
        SilKit_CanFrame canFrame;
        SilKit_Struct_Init(SilKit_CanFrame, canFrame);
        cMsg.frame = &canFrame;
        assignCxxToC(cxxMsg, cMsg);

        _functions->OnFrameRequest(_userController, &cMsg);
    }
};

} // namespace Can

namespace Flexray {

// --------------------------------
// Flexray
// --------------------------------

class CApiSimulatedFlexRayController : public ISimulatedFlexRayController
{
    void* _controller{nullptr};
    const SilKit_Experimental_SimulatedFlexRayControllerFunctions* _functions{nullptr};

public:
    CApiSimulatedFlexRayController() = default;

    CApiSimulatedFlexRayController(void* controller,
                                   const SilKit_Experimental_SimulatedFlexRayControllerFunctions* functions)
        : _controller{controller}
        , _functions{functions}
    {
    }

    void OnHostCommand(const FlexrayHostCommand& cxxMsg)
    {
        SilKit_Experimental_NetSim_FlexrayHostCommand cMsg;
        SilKit_Struct_Init(SilKit_Experimental_NetSim_FlexrayHostCommand, cMsg);
        assignCxxToC(cxxMsg, cMsg);

        _functions->OnHostCommand(_controller, &cMsg);
    }

    void OnControllerConfig(const FlexrayControllerConfig& cxxMsg)
    {
        SilKit_Experimental_NetSim_FlexrayControllerConfig cMsg;
        SilKit_Struct_Init(SilKit_Experimental_NetSim_FlexrayControllerConfig, cMsg);
        SilKit_FlexrayControllerConfig controllerConfig;
        SilKit_Struct_Init(SilKit_FlexrayControllerConfig, controllerConfig);
        SilKit_FlexrayClusterParameters clusterParameters;
        SilKit_Struct_Init(SilKit_FlexrayClusterParameters, clusterParameters);
        SilKit_FlexrayNodeParameters nodeParameters;
        SilKit_Struct_Init(SilKit_FlexrayNodeParameters, nodeParameters);

        std::vector<SilKit_FlexrayTxBufferConfig> cTxBufferConfigs;
        for (uint32_t idx = 0; idx < cxxMsg.bufferConfigs.size(); idx++)
        {
            SilKit_FlexrayTxBufferConfig cTxBufferConfig;
            SilKit_Struct_Init(SilKit_FlexrayTxBufferConfig, cTxBufferConfig);
            cTxBufferConfigs.emplace_back(cTxBufferConfig);
        }
        cMsg.bufferConfigs = cTxBufferConfigs.data();
        cMsg.clusterParams = &clusterParameters;
        cMsg.nodeParams = &nodeParameters;

        // Assign the data (including bufferConfigs data)
        assignCxxToC(cxxMsg, cMsg);

        _functions->OnControllerConfig(_controller, &cMsg);
    }

    void OnTxBufferConfigUpdate(const FlexrayTxBufferConfigUpdate& cxxMsg)
    {
        SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate cMsg;
        SilKit_Struct_Init(SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate, cMsg);
        SilKit_FlexrayTxBufferConfig txBufferConfig;
        SilKit_Struct_Init(SilKit_FlexrayTxBufferConfig, txBufferConfig);
        cMsg.txBufferConfig = &txBufferConfig;
        assignCxxToC(cxxMsg, cMsg);

        _functions->OnTxBufferConfigUpdate(_controller, &cMsg);
    }

    void OnTxBufferUpdate(const FlexrayTxBufferUpdate& cxxMsg)
    {
        SilKit_Experimental_NetSim_FlexrayTxBufferUpdate cMsg;
        SilKit_Struct_Init(SilKit_Experimental_NetSim_FlexrayTxBufferUpdate, cMsg);
        assignCxxToC(cxxMsg, cMsg);

        _functions->OnTxBufferUpdate(_controller, &cMsg);
    }
};

} // namespace Flexray

namespace Ethernet {

// --------------------------------
// Ethernet
// --------------------------------

class CApiSimulatedEthernetController : public ISimulatedEthernetController
{
    void* _userController{nullptr};
    const SilKit_Experimental_SimulatedEthernetControllerFunctions* _functions{nullptr};

public:
    CApiSimulatedEthernetController() = default;

    CApiSimulatedEthernetController(void* controller,
                                    const SilKit_Experimental_SimulatedEthernetControllerFunctions* functions)
        : _userController{controller}
        , _functions{functions}
    {
    }

    void OnFrameRequest(const EthernetFrameRequest& cxxMsg) override
    {
        SilKit_Experimental_NetSim_EthernetFrameRequest cMsg;
        SilKit_Struct_Init(SilKit_Experimental_NetSim_EthernetFrameRequest, cMsg);
        SilKit_EthernetFrame ethernetFrame;
        SilKit_Struct_Init(SilKit_EthernetFrame, ethernetFrame);
        cMsg.ethernetFrame = &ethernetFrame;
        assignCxxToC(cxxMsg, cMsg);

        _functions->OnFrameRequest(_userController, &cMsg);
    }

    void OnSetControllerMode(const EthernetControllerMode& cxxMsg) override
    {
        SilKit_Experimental_NetSim_EthernetControllerMode cMsg;
        SilKit_Struct_Init(SilKit_Experimental_NetSim_EthernetControllerMode, cMsg);
        assignCxxToC(cxxMsg, cMsg);

        _functions->OnSetControllerMode(_userController, &cMsg);
    }
};

} // namespace Ethernet


namespace Lin {

// --------------------------------
// Lin
// --------------------------------


class CApiSimulatedLinController : public ISimulatedLinController
{
    void* _userController{nullptr};
    const SilKit_Experimental_SimulatedLinControllerFunctions* _functions{nullptr};

public:
    CApiSimulatedLinController() = default;

    CApiSimulatedLinController(void* controller, const SilKit_Experimental_SimulatedLinControllerFunctions* functions)
        : _userController{controller}
        , _functions{functions}
    {
    }

    void OnFrameRequest(const LinFrameRequest& cxxMsg) override
    {
        SilKit_Experimental_NetSim_LinFrameRequest cMsg;
        SilKit_Struct_Init(SilKit_Experimental_NetSim_LinFrameRequest, cMsg);
        SilKit_LinFrame linFrame;
        SilKit_Struct_Init(SilKit_LinFrame, linFrame);
        cMsg.frame = &linFrame;
        assignCxxToC(cxxMsg, cMsg);

        _functions->OnFrameRequest(_userController, &cMsg);
    }

    void OnFrameHeaderRequest(const LinFrameHeaderRequest& cxxMsg) override
    {
        SilKit_Experimental_NetSim_LinFrameHeaderRequest cMsg;
        SilKit_Struct_Init(SilKit_Experimental_NetSim_LinFrameHeaderRequest, cMsg);
        assignCxxToC(cxxMsg, cMsg);

        _functions->OnFrameHeaderRequest(_userController, &cMsg);
    }

    void OnWakeupPulse(const LinWakeupPulse& cxxMsg) override
    {
        SilKit_Experimental_NetSim_LinWakeupPulse cMsg;
        SilKit_Struct_Init(SilKit_Experimental_NetSim_LinWakeupPulse, cMsg);
        assignCxxToC(cxxMsg, cMsg);

        _functions->OnWakeupPulse(_userController, &cMsg);
    }
    void OnControllerConfig(const LinControllerConfig& cxxMsg) override
    {
        SilKit_Experimental_NetSim_LinControllerConfig cMsg;
        SilKit_Struct_Init(SilKit_Experimental_NetSim_LinControllerConfig, cMsg);

        std::vector<SilKit_LinFrame> cLinFrames;
        cLinFrames.reserve(cxxMsg.frameResponses.size());
        std::vector<SilKit_LinFrameResponse> cLinFrameResponses;
        cLinFrameResponses.reserve(cxxMsg.frameResponses.size());
        for (uint32_t idx = 0; idx < cxxMsg.frameResponses.size(); idx++)
        {
            SilKit_LinFrameResponse cLinFrameResponse;
            SilKit_Struct_Init(SilKit_LinFrameResponse, cLinFrameResponse);

            SilKit_LinFrame cLinFrame;
            SilKit_Struct_Init(SilKit_LinFrame, cLinFrame);

            cLinFrames.emplace_back(cLinFrame);
            cLinFrameResponse.frame = &cLinFrames.back();
            cLinFrameResponses.emplace_back(cLinFrameResponse);
        }
        cMsg.frameResponses = cLinFrameResponses.data();

        assignCxxToC(cxxMsg, cMsg);

        _functions->OnControllerConfig(_userController, &cMsg);
    }
    void OnFrameResponseUpdate(const LinFrameResponseUpdate& cxxMsg) override
    {
        SilKit_Experimental_NetSim_LinFrameResponseUpdate cMsg;
        SilKit_Struct_Init(SilKit_Experimental_NetSim_LinFrameResponseUpdate, cMsg);

        std::vector<SilKit_LinFrame> cLinFrames;
        cLinFrames.reserve(cxxMsg.frameResponses.size());
        std::vector<SilKit_LinFrameResponse> cLinFrameResponses;
        cLinFrameResponses.reserve(cxxMsg.frameResponses.size());
        for (uint32_t idx = 0; idx < cxxMsg.frameResponses.size(); idx++)
        {
            SilKit_LinFrameResponse cLinFrameResponse;
            SilKit_Struct_Init(SilKit_LinFrameResponse, cLinFrameResponse);

            SilKit_LinFrame cLinFrame;
            SilKit_Struct_Init(SilKit_LinFrame, cLinFrame);

            cLinFrames.emplace_back(cLinFrame);
            cLinFrameResponse.frame = &cLinFrames.back();
            cLinFrameResponses.emplace_back(cLinFrameResponse);
        }
        cMsg.frameResponses = cLinFrameResponses.data();

        assignCxxToC(cxxMsg, cMsg);

        _functions->OnFrameResponseUpdate(_userController, &cMsg);
    }
    void OnControllerStatusUpdate(const LinControllerStatusUpdate& cxxMsg) override
    {
        SilKit_Experimental_NetSim_LinControllerStatusUpdate cMsg;
        SilKit_Struct_Init(SilKit_Experimental_NetSim_LinControllerStatusUpdate, cMsg);
        assignCxxToC(cxxMsg, cMsg);

        _functions->OnControllerStatusUpdate(_userController, &cMsg);
    }
};

} // namespace Lin

class CApiSimulatedNetwork : public ISimulatedNetwork
{
public:
    void* _simulatedNetwork{nullptr};

private:
    SilKit_Experimental_SimulatedNetworkType _networkType;
    std::string _networkName;
    const SilKit_Experimental_SimulatedNetworkFunctions* _functions{nullptr};
    std::unique_ptr<Experimental::NetworkSimulation::IEventProducer> _eventProducer;
    std::vector<std::unique_ptr<Experimental::NetworkSimulation::ISimulatedController>> _controllers;

public:
    CApiSimulatedNetwork() = default;

    CApiSimulatedNetwork(SilKit_Experimental_SimulatedNetworkType networkType, const std::string& networkName,
                         void* simulatedNetwork, const SilKit_Experimental_SimulatedNetworkFunctions* functions)
        : _simulatedNetwork{simulatedNetwork}
        , _networkType{networkType}
        , _networkName{networkName}
        , _functions{functions}
    {
    }

    auto ProvideSimulatedController(ControllerDescriptor controllerDescriptor) -> ISimulatedController* override
    {
        void* userSimulatedController{nullptr};
        const void* functions{nullptr};
        _functions->ProvideSimulatedController(&userSimulatedController, &functions, controllerDescriptor,
                                               _simulatedNetwork);

        if (userSimulatedController == nullptr)
        {
            const std::string errorMsg =
                "NetworkSimulation: No simulated controller was provided on network '" + _networkName + "'.";
            throw SilKitError{errorMsg};
        }

        switch (_networkType)
        {
        case SilKit_NetworkType_CAN:
            _controllers.emplace_back(std::make_unique<Can::CApiSimulatedCanController>(
                userSimulatedController, (SilKit_Experimental_SimulatedCanControllerFunctions*)functions));
            return _controllers.back().get();
        case SilKit_NetworkType_FlexRay:
            _controllers.emplace_back(std::make_unique<Flexray::CApiSimulatedFlexRayController>(
                userSimulatedController, (SilKit_Experimental_SimulatedFlexRayControllerFunctions*)functions));
            return _controllers.back().get();
        case SilKit_NetworkType_Ethernet:
            _controllers.emplace_back(std::make_unique<Ethernet::CApiSimulatedEthernetController>(
                userSimulatedController, (SilKit_Experimental_SimulatedEthernetControllerFunctions*)functions));
            return _controllers.back().get();
        case SilKit_NetworkType_LIN:
            _controllers.emplace_back(std::make_unique<Lin::CApiSimulatedLinController>(
                userSimulatedController, (SilKit_Experimental_SimulatedLinControllerFunctions*)functions));
            return _controllers.back().get();
        default:
            break;
        }

        return {};
    }

    void SimulatedControllerRemoved(ControllerDescriptor controllerDescriptor) override
    {
        _functions->SimulatedControllerRemoved(controllerDescriptor, _simulatedNetwork);
    }

    void SetEventProducer(std::unique_ptr<IEventProducer> eventProducer) override
    {
        auto* cEventProducer = reinterpret_cast<SilKit_Experimental_EventProducer*>(eventProducer.get());
        _functions->SetEventProducer(cEventProducer, _simulatedNetwork);
        _eventProducer = std::move(eventProducer);
    }
};

} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit


SilKit_ReturnCode SilKitCALL SilKit_Experimental_NetworkSimulator_Create(
    SilKit_Experimental_NetworkSimulator** outNetworkSimulator, SilKit_Participant* participant)
try
{
    ASSERT_VALID_OUT_PARAMETER(outNetworkSimulator);
    ASSERT_VALID_POINTER_PARAMETER(participant);

    auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
    auto* cppNetworkSimulator = SilKit::Experimental::Participant::CreateNetworkSimulatorImpl(cppParticipant);
    *outNetworkSimulator = reinterpret_cast<SilKit_Experimental_NetworkSimulator*>(cppNetworkSimulator);

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_NetworkSimulator_SimulateNetwork(
    SilKit_Experimental_NetworkSimulator* networkSimulator, const char* networkName,
    SilKit_Experimental_SimulatedNetworkType networkType, void* userSimulatedNetwork,
    const SilKit_Experimental_SimulatedNetworkFunctions* functions)
try
{
    ASSERT_VALID_POINTER_PARAMETER(networkSimulator);
    ASSERT_VALID_POINTER_PARAMETER(networkName);
    ASSERT_VALID_POINTER_PARAMETER(userSimulatedNetwork);
    ASSERT_VALID_POINTER_PARAMETER(functions);
    ASSERT_VALID_STRUCT_HEADER(functions);

    // Create simulatedNetwork with user functions
    auto simulatedNetwork = std::make_unique<SilKit::Experimental::NetworkSimulation::CApiSimulatedNetwork>(
        networkType, networkName, userSimulatedNetwork, functions);
    auto cppNetworkSimulator =
        reinterpret_cast<SilKit::Experimental::NetworkSimulation::INetworkSimulator*>(networkSimulator);
    cppNetworkSimulator->SimulateNetwork(
        networkName, static_cast<SilKit::Experimental::NetworkSimulation::SimulatedNetworkType>(networkType),
        std::move(simulatedNetwork));

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL
SilKit_Experimental_NetworkSimulator_Start(SilKit_Experimental_NetworkSimulator* networkSimulator)
try
{
    ASSERT_VALID_POINTER_PARAMETER(networkSimulator);

    auto cppNetworkSimulator =
        reinterpret_cast<SilKit::Experimental::NetworkSimulation::INetworkSimulator*>(networkSimulator);
    cppNetworkSimulator->Start();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS

// --------------------------------
// Can
// --------------------------------

template <typename MsgT>
void Produce(MsgT msg, const SilKit_Experimental_EventReceivers* receivers,
             SilKit_Experimental_CanEventProducer* eventProducer)
{
    auto cppEventProducer =
        reinterpret_cast<SilKit::Experimental::NetworkSimulation::Can::ICanEventProducer*>(eventProducer);
    auto cppReceivers = SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>(
        receivers->controllerDescriptors, receivers->numReceivers);
    cppEventProducer->Produce(msg, cppReceivers);
}

SilKit_ReturnCode SilKitCALL SilKit_Experimental_CanEventProducer_Produce(
    SilKit_Experimental_CanEventProducer* eventProducer, SilKit_StructHeader* cEvent,
    const SilKit_Experimental_EventReceivers* receivers)
try
{
    ASSERT_VALID_POINTER_PARAMETER(eventProducer);
    ASSERT_VALID_POINTER_PARAMETER(cEvent);
    ASSERT_VALID_POINTER_PARAMETER(receivers);
    ASSERT_VALID_PLAIN_STRUCT_HEADER(cEvent);
    ASSERT_VALID_STRUCT_HEADER(receivers);

    auto msgDatatypeId = SK_ID_GET_DATATYPE(cEvent->version);
    switch (msgDatatypeId)
    {
    case SilKit_CanFrameEvent_DATATYPE_ID:
    {
        SilKit::Services::Can::CanFrameEvent cxxEvent;
        assignCToCxx((SilKit_CanFrameEvent*)cEvent, cxxEvent);
        Produce(cxxEvent, receivers, eventProducer);
        break;
    }
    case SilKit_CanFrameTransmitEvent_DATATYPE_ID:
    {
        SilKit::Services::Can::CanFrameTransmitEvent cxxEvent;
        assignCToCxx((SilKit_CanFrameTransmitEvent*)cEvent, cxxEvent);
        Produce(cxxEvent, receivers, eventProducer);
        break;
    }
    case SilKit_CanStateChangeEvent_DATATYPE_ID:
    {
        SilKit::Services::Can::CanStateChangeEvent cxxEvent;
        assignCToCxx((SilKit_CanStateChangeEvent*)cEvent, cxxEvent);
        Produce(cxxEvent, receivers, eventProducer);
        break;
    }
    case SilKit_CanErrorStateChangeEvent_DATATYPE_ID:
    {
        SilKit::Services::Can::CanErrorStateChangeEvent cxxEvent;
        assignCToCxx((SilKit_CanErrorStateChangeEvent*)cEvent, cxxEvent);
        Produce(cxxEvent, receivers, eventProducer);
        break;
    }
    default:
        break;
    }

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS

// --------------------------------
// FlexRay
// --------------------------------

template <typename MsgT>
void Produce(MsgT msg, const SilKit_Experimental_EventReceivers* receivers,
             SilKit_Experimental_FlexRayEventProducer* eventProducer)
{
    auto cppEventProducer =
        reinterpret_cast<SilKit::Experimental::NetworkSimulation::Flexray::IFlexRayEventProducer*>(eventProducer);
    auto cppReceivers = SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>(
        receivers->controllerDescriptors, receivers->numReceivers);
    cppEventProducer->Produce(msg, cppReceivers);
}

SilKit_ReturnCode SilKitCALL SilKit_Experimental_FlexRayEventProducer_Produce(
    SilKit_Experimental_FlexRayEventProducer* eventProducer, SilKit_StructHeader* cEvent,
    const SilKit_Experimental_EventReceivers* receivers)
try
{
    ASSERT_VALID_POINTER_PARAMETER(eventProducer);
    ASSERT_VALID_POINTER_PARAMETER(cEvent);
    ASSERT_VALID_POINTER_PARAMETER(receivers);
    ASSERT_VALID_PLAIN_STRUCT_HEADER(cEvent);
    ASSERT_VALID_STRUCT_HEADER(receivers);

    auto msgDatatypeId = SK_ID_GET_DATATYPE(cEvent->version);
    switch (msgDatatypeId)
    {
    case SilKit_FlexrayFrameEvent_DATATYPE_ID:
    {
        SilKit::Services::Flexray::FlexrayFrameEvent cxxEvent;
        assignCToCxx((SilKit_FlexrayFrameEvent*)cEvent, cxxEvent);
        Produce(cxxEvent, receivers, eventProducer);
        break;
    }
    case SilKit_FlexrayFrameTransmitEvent_DATATYPE_ID:
    {
        SilKit::Services::Flexray::FlexrayFrameTransmitEvent cxxEvent;
        assignCToCxx((SilKit_FlexrayFrameTransmitEvent*)cEvent, cxxEvent);
        Produce(cxxEvent, receivers, eventProducer);
        break;
    }
    case SilKit_FlexraySymbolEvent_DATATYPE_ID:
    {
        SilKit::Services::Flexray::FlexraySymbolEvent cxxEvent;
        assignCToCxx((SilKit_FlexraySymbolEvent*)cEvent, cxxEvent);
        Produce(cxxEvent, receivers, eventProducer);
        break;
    }
    case SilKit_FlexraySymbolTransmitEvent_DATATYPE_ID:
    {
        SilKit::Services::Flexray::FlexraySymbolTransmitEvent cxxEvent;
        assignCToCxx((SilKit_FlexraySymbolTransmitEvent*)cEvent, cxxEvent);
        Produce(cxxEvent, receivers, eventProducer);
        break;
    }
    case SilKit_FlexrayCycleStartEvent_DATATYPE_ID:
    {
        SilKit::Services::Flexray::FlexrayCycleStartEvent cxxEvent;
        assignCToCxx((SilKit_FlexrayCycleStartEvent*)cEvent, cxxEvent);
        Produce(cxxEvent, receivers, eventProducer);
        break;
    }
    case SilKit_FlexrayPocStatusEvent_DATATYPE_ID:
    {
        SilKit::Services::Flexray::FlexrayPocStatusEvent cxxEvent;
        assignCToCxx((SilKit_FlexrayPocStatusEvent*)cEvent, cxxEvent);
        Produce(cxxEvent, receivers, eventProducer);
        break;
    }
    default:
        break;
    }

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS

// --------------------------------
// Ethernet
// --------------------------------

template <typename MsgT>
void Produce(MsgT msg, const SilKit_Experimental_EventReceivers* receivers,
             SilKit_Experimental_EthernetEventProducer* eventProducer)
{
    auto cppEventProducer =
        reinterpret_cast<SilKit::Experimental::NetworkSimulation::Ethernet::IEthernetEventProducer*>(eventProducer);
    auto cppReceivers = SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>(
        receivers->controllerDescriptors, receivers->numReceivers);
    cppEventProducer->Produce(msg, cppReceivers);
}

SilKit_ReturnCode SilKitCALL SilKit_Experimental_EthernetEventProducer_Produce(
    SilKit_Experimental_EthernetEventProducer* eventProducer, SilKit_StructHeader* cEvent,
    const SilKit_Experimental_EventReceivers* receivers)
try
{
    ASSERT_VALID_POINTER_PARAMETER(eventProducer);
    ASSERT_VALID_POINTER_PARAMETER(cEvent);
    ASSERT_VALID_POINTER_PARAMETER(receivers);
    ASSERT_VALID_PLAIN_STRUCT_HEADER(cEvent);
    ASSERT_VALID_STRUCT_HEADER(receivers);

    auto msgDatatypeId = SK_ID_GET_DATATYPE(cEvent->version);
    switch (msgDatatypeId)
    {
    case SilKit_EthernetFrameEvent_DATATYPE_ID:
    {
        SilKit::Services::Ethernet::EthernetFrameEvent cxxEvent;
        assignCToCxx((SilKit_EthernetFrameEvent*)cEvent, cxxEvent);
        Produce(cxxEvent, receivers, eventProducer);
        break;
    }
    case SilKit_EthernetFrameTransmitEvent_DATATYPE_ID:
    {
        SilKit::Services::Ethernet::EthernetFrameTransmitEvent cxxEvent;
        assignCToCxx((SilKit_EthernetFrameTransmitEvent*)cEvent, cxxEvent);
        Produce(cxxEvent, receivers, eventProducer);
        break;
    }
    case SilKit_EthernetStateChangeEvent_DATATYPE_ID:
    {
        SilKit::Services::Ethernet::EthernetStateChangeEvent cxxEvent;
        assignCToCxx((SilKit_EthernetStateChangeEvent*)cEvent, cxxEvent);
        Produce(cxxEvent, receivers, eventProducer);
        break;
    }
    case SilKit_EthernetBitrateChangeEvent_DATATYPE_ID:
    {
        SilKit::Services::Ethernet::EthernetBitrateChangeEvent cxxEvent;
        assignCToCxx((SilKit_EthernetBitrateChangeEvent*)cEvent, cxxEvent);
        Produce(cxxEvent, receivers, eventProducer);
        break;
    }
    default:
        break;
    }

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS

// --------------------------------
// Lin
// --------------------------------

template <typename MsgT>
void Produce(MsgT msg, const SilKit_Experimental_EventReceivers* receivers,
             SilKit_Experimental_LinEventProducer* eventProducer)
{
    auto cppEventProducer =
        reinterpret_cast<SilKit::Experimental::NetworkSimulation::Lin::ILinEventProducer*>(eventProducer);
    auto cppReceivers = SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>(
        receivers->controllerDescriptors, receivers->numReceivers);
    cppEventProducer->Produce(msg, cppReceivers);
}

SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinEventProducer_Produce(
    SilKit_Experimental_LinEventProducer* eventProducer, SilKit_StructHeader* cEvent,
    const SilKit_Experimental_EventReceivers* receivers)
try
{
    ASSERT_VALID_POINTER_PARAMETER(eventProducer);
    ASSERT_VALID_POINTER_PARAMETER(cEvent);
    ASSERT_VALID_POINTER_PARAMETER(receivers);
    ASSERT_VALID_PLAIN_STRUCT_HEADER(cEvent);
    ASSERT_VALID_STRUCT_HEADER(receivers);

    auto msgDatatypeId = SK_ID_GET_DATATYPE(cEvent->version);


    switch (msgDatatypeId)
    {
    case SilKit_LinFrameStatusEvent_DATATYPE_ID:
    {
        SilKit::Services::Lin::LinFrameStatusEvent cxxEvent;
        assignCToCxx((SilKit_LinFrameStatusEvent*)cEvent, cxxEvent);
        Produce(cxxEvent, receivers, eventProducer);
        break;
    }
    case SilKit_LinSendFrameHeaderRequest_DATATYPE_ID:
    {
        SilKit::Services::Lin::LinSendFrameHeaderRequest cxxEvent;
        assignCToCxx((SilKit_LinSendFrameHeaderRequest*)cEvent, cxxEvent);
        Produce(cxxEvent, receivers, eventProducer);
        break;
    }
    case SilKit_LinWakeupEvent_DATATYPE_ID:
    {
        SilKit::Services::Lin::LinWakeupEvent cxxEvent;
        assignCToCxx((SilKit_LinWakeupEvent*)cEvent, cxxEvent);
        Produce(cxxEvent, receivers, eventProducer);
        break;
    }
    default:
        break;
    }

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS
