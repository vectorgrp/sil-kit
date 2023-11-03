// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <unordered_map>

#include "silkit/capi/NetworkSimulator.h"
#include "silkit/experimental/netsim/INetworkSimulator.hpp"
#include "silkit/experimental/netsim/string_utils.hpp"

#include "silkit/detail/impl/HourglassConversions.hpp"
#include "silkit/detail/impl/ThrowOnError.hpp"

#include "EventProducer.hpp"

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Experimental {
namespace NetworkSimulation {

// --------------------------------
// CAN
// --------------------------------

inline auto MakeSimulatedCanControllerFunctions() -> SilKit_Experimental_SimulatedCanControllerFunctions
{
    SilKit_Experimental_SimulatedCanControllerFunctions functions;
    SilKit_Struct_Init(SilKit_Experimental_SimulatedCanControllerFunctions, functions);

    functions.OnFrameRequest = [](void* controller, const SilKit_Experimental_NetSim_CanFrameRequest* cMsg) {
        auto cppSimulatedController =
            static_cast<SilKit::Experimental::NetworkSimulation::Can::ISimulatedCanController*>(controller);
        SilKit::Experimental::NetworkSimulation::Can::CanFrameRequest cxxMsg{};
        assignCToCxx(cMsg, cxxMsg);
        cppSimulatedController->OnFrameRequest(std::move(cxxMsg));
    };

    functions.OnSetBaudrate = [](void* controller, const SilKit_Experimental_NetSim_CanConfigureBaudrate* cMsg) {
        auto cppSimulatedController =
            static_cast<SilKit::Experimental::NetworkSimulation::Can::ISimulatedCanController*>(controller);
        SilKit::Experimental::NetworkSimulation::Can::CanConfigureBaudrate cxxMsg{};
        assignCToCxx(cMsg, cxxMsg);
        cppSimulatedController->OnSetBaudrate(std::move(cxxMsg));
    };

    functions.OnSetControllerMode = [](void* controller, const SilKit_Experimental_NetSim_CanControllerMode* cMsg) {
        auto cppSimulatedController =
            static_cast<SilKit::Experimental::NetworkSimulation::Can::ISimulatedCanController*>(controller);
        SilKit::Experimental::NetworkSimulation::Can::CanControllerMode cxxMsg{};
        assignCToCxx(cMsg, cxxMsg);
        cppSimulatedController->OnSetControllerMode(std::move(cxxMsg));
    };

    return functions;
}

// --------------------------------
// FlexRay
// --------------------------------

inline auto MakeSimulatedFlexRayControllerFunctions() -> SilKit_Experimental_SimulatedFlexRayControllerFunctions
{
    SilKit_Experimental_SimulatedFlexRayControllerFunctions functions;
    SilKit_Struct_Init(SilKit_Experimental_SimulatedFlexRayControllerFunctions, functions);

    functions.OnHostCommand = [](void* controller, const SilKit_Experimental_NetSim_FlexrayHostCommand* cMsg) {
        auto cppSimulatedController =
            static_cast<SilKit::Experimental::NetworkSimulation::Flexray::ISimulatedFlexRayController*>(controller);
        SilKit::Experimental::NetworkSimulation::Flexray::FlexrayHostCommand cxxMsg{};
        assignCToCxx(cMsg, cxxMsg);
        cppSimulatedController->OnHostCommand(std::move(cxxMsg));
    };

    functions.OnControllerConfig = [](void* controller,
                                      const SilKit_Experimental_NetSim_FlexrayControllerConfig* cMsg) {
        auto cppSimulatedController =
            static_cast<SilKit::Experimental::NetworkSimulation::Flexray::ISimulatedFlexRayController*>(controller);
        SilKit::Experimental::NetworkSimulation::Flexray::FlexrayControllerConfig cxxMsg{};
        assignCToCxx(cMsg, cxxMsg);
        cppSimulatedController->OnControllerConfig(std::move(cxxMsg));
    };

    functions.OnTxBufferConfigUpdate = [](void* controller,
                                          const SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate* cMsg) {
        auto cppSimulatedController =
            static_cast<SilKit::Experimental::NetworkSimulation::Flexray::ISimulatedFlexRayController*>(controller);
        SilKit::Experimental::NetworkSimulation::Flexray::FlexrayTxBufferConfigUpdate cxxMsg{};
        assignCToCxx(cMsg, cxxMsg);
        cppSimulatedController->OnTxBufferConfigUpdate(std::move(cxxMsg));
    };

    functions.OnTxBufferUpdate = [](void* controller, const SilKit_Experimental_NetSim_FlexrayTxBufferUpdate* cMsg) {
        auto cppSimulatedController =
            static_cast<SilKit::Experimental::NetworkSimulation::Flexray::ISimulatedFlexRayController*>(controller);
        SilKit::Experimental::NetworkSimulation::Flexray::FlexrayTxBufferUpdate cxxMsg{};
        assignCToCxx(cMsg, cxxMsg);
        cppSimulatedController->OnTxBufferUpdate(std::move(cxxMsg));
    };

    return functions;
}

// --------------------------------
// Ethernet
// --------------------------------

inline auto MakeSimulatedEthernetControllerFunctions() -> SilKit_Experimental_SimulatedEthernetControllerFunctions
{
    SilKit_Experimental_SimulatedEthernetControllerFunctions functions;
    SilKit_Struct_Init(SilKit_Experimental_SimulatedEthernetControllerFunctions, functions);

    functions.OnFrameRequest = [](void* controller, const SilKit_Experimental_NetSim_EthernetFrameRequest* cMsg) {
        auto cppSimulatedController =
            static_cast<SilKit::Experimental::NetworkSimulation::Ethernet::ISimulatedEthernetController*>(controller);
        SilKit::Experimental::NetworkSimulation::Ethernet::EthernetFrameRequest cxxMsg{};
        assignCToCxx(cMsg, cxxMsg);
        cppSimulatedController->OnFrameRequest(std::move(cxxMsg));
    };

    functions.OnSetControllerMode = [](void* controller,
                                       const SilKit_Experimental_NetSim_EthernetControllerMode* cMsg) {
        auto cppSimulatedController =
            static_cast<SilKit::Experimental::NetworkSimulation::Ethernet::ISimulatedEthernetController*>(controller);
        SilKit::Experimental::NetworkSimulation::Ethernet::EthernetControllerMode cxxMsg{};
        assignCToCxx(cMsg, cxxMsg);
        cppSimulatedController->OnSetControllerMode(std::move(cxxMsg));
    };

    return functions;
}

// --------------------------------
// Lin
// --------------------------------

inline auto MakeSimulatedLinControllerFunctions() -> SilKit_Experimental_SimulatedLinControllerFunctions
{
    SilKit_Experimental_SimulatedLinControllerFunctions functions;
    SilKit_Struct_Init(SilKit_Experimental_SimulatedLinControllerFunctions, functions);

    functions.OnFrameRequest = [](void* controller, const SilKit_Experimental_NetSim_LinFrameRequest* cMsg) {
        auto cppSimulatedController =
            static_cast<SilKit::Experimental::NetworkSimulation::Lin::ISimulatedLinController*>(controller);
        SilKit::Experimental::NetworkSimulation::Lin::LinFrameRequest cxxMsg{};
        assignCToCxx(cMsg, cxxMsg);
        cppSimulatedController->OnFrameRequest(std::move(cxxMsg));
    };

    functions.OnFrameHeaderRequest = [](void* controller,
                                        const SilKit_Experimental_NetSim_LinFrameHeaderRequest* cMsg) {
        auto cppSimulatedController =
            static_cast<SilKit::Experimental::NetworkSimulation::Lin::ISimulatedLinController*>(controller);
        SilKit::Experimental::NetworkSimulation::Lin::LinFrameHeaderRequest cxxMsg{};
        assignCToCxx(cMsg, cxxMsg);
        cppSimulatedController->OnFrameHeaderRequest(std::move(cxxMsg));
    };

    functions.OnWakeupPulse = [](void* controller, const SilKit_Experimental_NetSim_LinWakeupPulse* cMsg) {
        auto cppSimulatedController =
            static_cast<SilKit::Experimental::NetworkSimulation::Lin::ISimulatedLinController*>(controller);
        SilKit::Experimental::NetworkSimulation::Lin::LinWakeupPulse cxxMsg{};
        assignCToCxx(cMsg, cxxMsg);
        cppSimulatedController->OnWakeupPulse(std::move(cxxMsg));
    };

    functions.OnControllerConfig = [](void* controller, const SilKit_Experimental_NetSim_LinControllerConfig* cMsg) {
        auto cppSimulatedController =
            static_cast<SilKit::Experimental::NetworkSimulation::Lin::ISimulatedLinController*>(controller);
        SilKit::Experimental::NetworkSimulation::Lin::LinControllerConfig cxxMsg{};
        assignCToCxx(cMsg, cxxMsg);
        cppSimulatedController->OnControllerConfig(std::move(cxxMsg));
    };

    functions.OnFrameResponseUpdate = [](void* controller,
                                         const SilKit_Experimental_NetSim_LinFrameResponseUpdate* cMsg) {
        auto cppSimulatedController =
            static_cast<SilKit::Experimental::NetworkSimulation::Lin::ISimulatedLinController*>(controller);
        SilKit::Experimental::NetworkSimulation::Lin::LinFrameResponseUpdate cxxMsg{};
        assignCToCxx(cMsg, cxxMsg);
        cppSimulatedController->OnFrameResponseUpdate(std::move(cxxMsg));
    };

    functions.OnControllerStatusUpdate = [](void* controller,
                                            const SilKit_Experimental_NetSim_LinControllerStatusUpdate* cMsg) {
        auto cppSimulatedController =
            static_cast<SilKit::Experimental::NetworkSimulation::Lin::ISimulatedLinController*>(controller);
        SilKit::Experimental::NetworkSimulation::Lin::LinControllerStatusUpdate cxxMsg{};
        assignCToCxx(cMsg, cxxMsg);
        cppSimulatedController->OnControllerStatusUpdate(std::move(cxxMsg));
    };

    return functions;
}

// --------------------------------
// SimulatedNetwork
// --------------------------------

inline auto MakeSimulatedNetworkFunctions(SilKit::Experimental::NetworkSimulation::SimulatedNetworkType networkType)
    -> SilKit_Experimental_SimulatedNetworkFunctions
{
    SilKit_Experimental_SimulatedNetworkFunctions functions;
    SilKit_Struct_Init(SilKit_Experimental_SimulatedNetworkFunctions, functions);
    switch (networkType)
    {
    case SilKit::Experimental::NetworkSimulation::SimulatedNetworkType::CAN:
        functions.ProvideSimulatedController = [](void** outSimulatedController,
                                                  const void** outSimulatedControllerFunctions,
                                                  SilKit_Experimental_ControllerDescriptor controllerDescriptor,
                                                  void* simulatedNetwork) {
            static const auto userSimulatedCanControllerFunctions = MakeSimulatedCanControllerFunctions();
            *outSimulatedControllerFunctions = &userSimulatedCanControllerFunctions;
            auto* cppUserSimulatedNetwork =
                static_cast<SilKit::Experimental::NetworkSimulation::ISimulatedNetwork*>(simulatedNetwork);
            auto cppUserSimulatedController = cppUserSimulatedNetwork->ProvideSimulatedController(controllerDescriptor);
            *outSimulatedController = static_cast<void*>(cppUserSimulatedController);
        };
        functions.SetEventProducer = [](void* eventProducer, void* simulatedNetwork) {
            auto* cppUserSimulatedNetwork =
                static_cast<SilKit::Experimental::NetworkSimulation::ISimulatedNetwork*>(simulatedNetwork);
            auto cppEventProducer =
                std::make_unique<CanEventProducer>(static_cast<SilKit_Experimental_CanEventProducer*>(eventProducer));
            cppUserSimulatedNetwork->SetEventProducer(std::move(cppEventProducer));
        };
        break;
    case SilKit::Experimental::NetworkSimulation::SimulatedNetworkType::FlexRay:
        functions.ProvideSimulatedController = [](void** outSimulatedController,
                                                  const void** outSimulatedControllerFunctions,
                                                  SilKit_Experimental_ControllerDescriptor controllerDescriptor,
                                                  void* simulatedNetwork) {
            static const auto userSimulatedFlexRayControllerFunctions = MakeSimulatedFlexRayControllerFunctions();
            *outSimulatedControllerFunctions = &userSimulatedFlexRayControllerFunctions;
            auto* cppUserSimulatedNetwork =
                static_cast<SilKit::Experimental::NetworkSimulation::ISimulatedNetwork*>(simulatedNetwork);
            auto cppUserSimulatedController = cppUserSimulatedNetwork->ProvideSimulatedController(controllerDescriptor);
            *outSimulatedController = static_cast<void*>(cppUserSimulatedController);
        };
        functions.SetEventProducer = [](void* eventProducer, void* simulatedNetwork) {
            auto* cppUserSimulatedNetwork =
                static_cast<SilKit::Experimental::NetworkSimulation::ISimulatedNetwork*>(simulatedNetwork);
            auto cppEventProducer = std::make_unique<FlexRayEventProducer>(
                static_cast<SilKit_Experimental_FlexRayEventProducer*>(eventProducer));
            cppUserSimulatedNetwork->SetEventProducer(std::move(cppEventProducer));
        };
        break;
    case SilKit::Experimental::NetworkSimulation::SimulatedNetworkType::Ethernet:
        functions.ProvideSimulatedController = [](void** outSimulatedController,
                                                  const void** outSimulatedControllerFunctions,
                                                  SilKit_Experimental_ControllerDescriptor controllerDescriptor,

                                                  void* simulatedNetwork) {
            static const auto userSimulatedEthernetControllerFunctions = MakeSimulatedEthernetControllerFunctions();
            *outSimulatedControllerFunctions = &userSimulatedEthernetControllerFunctions;
            auto* cppUserSimulatedNetwork =
                static_cast<SilKit::Experimental::NetworkSimulation::ISimulatedNetwork*>(simulatedNetwork);
            auto cppUserSimulatedController = cppUserSimulatedNetwork->ProvideSimulatedController(controllerDescriptor);
            *outSimulatedController = static_cast<void*>(cppUserSimulatedController);
        };
        functions.SetEventProducer = [](void* eventProducer, void* simulatedNetwork) {
            auto* cppUserSimulatedNetwork =
                static_cast<SilKit::Experimental::NetworkSimulation::ISimulatedNetwork*>(simulatedNetwork);
            auto cppEventProducer = std::make_unique<EthernetEventProducer>(
                static_cast<SilKit_Experimental_EthernetEventProducer*>(eventProducer));
            cppUserSimulatedNetwork->SetEventProducer(std::move(cppEventProducer));
        };
        break;
    case SilKit::Experimental::NetworkSimulation::SimulatedNetworkType::LIN:
        functions.ProvideSimulatedController = [](void** outSimulatedController,
                                                  const void** outSimulatedControllerFunctions,
                                                  SilKit_Experimental_ControllerDescriptor controllerDescriptor,

                                                  void* simulatedNetwork) {
            static const auto userSimulatedLinControllerFunctions = MakeSimulatedLinControllerFunctions();
            *outSimulatedControllerFunctions = &userSimulatedLinControllerFunctions;
            auto* cppUserSimulatedNetwork =
                static_cast<SilKit::Experimental::NetworkSimulation::ISimulatedNetwork*>(simulatedNetwork);
            auto cppUserSimulatedController = cppUserSimulatedNetwork->ProvideSimulatedController(controllerDescriptor);
            *outSimulatedController = static_cast<void*>(cppUserSimulatedController);
        };
        functions.SetEventProducer = [](void* eventProducer, void* simulatedNetwork) {
            auto* cppUserSimulatedNetwork =
                static_cast<SilKit::Experimental::NetworkSimulation::ISimulatedNetwork*>(simulatedNetwork);
            auto cppEventProducer =
                std::make_unique<LinEventProducer>(static_cast<SilKit_Experimental_LinEventProducer*>(eventProducer));
            cppUserSimulatedNetwork->SetEventProducer(std::move(cppEventProducer));
        };
        break;
    default:
        break;
    }

    // Functions not depending on the network type

    functions.SimulatedControllerRemoved = [](SilKit_Experimental_ControllerDescriptor controllerDescriptor,
                                              void* simulatedNetwork) {
        auto* cppUserSimulatedNetwork =
            static_cast<SilKit::Experimental::NetworkSimulation::ISimulatedNetwork*>(simulatedNetwork);
        cppUserSimulatedNetwork->SimulatedControllerRemoved(controllerDescriptor);
    };

    return functions;
}

// --------------------------------
// NetworkSimulator
// --------------------------------

class NetworkSimulator : public SilKit::Experimental::NetworkSimulation::INetworkSimulator
{
public:
    inline explicit NetworkSimulator(SilKit_Participant* participant);

    // INetworkSimulator
    inline void SimulateNetwork(
        const std::string& networkName, SilKit::Experimental::NetworkSimulation::SimulatedNetworkType networkType,
        std::unique_ptr<SilKit::Experimental::NetworkSimulation::ISimulatedNetwork> simulatedNetwork) override;

    inline void Start() override;

private:
    SilKit_Experimental_NetworkSimulator* _networkSimulator{nullptr};

    using SimulatedNetworksByNetworkName =
        std::unordered_map<std::string /*networkName*/,
                           std::unique_ptr<SilKit::Experimental::NetworkSimulation::ISimulatedNetwork>>;
    using SimulatedNetworksByTypeAndNetworkName =
        std::unordered_map<SilKit::Experimental::NetworkSimulation::SimulatedNetworkType /*networkType*/,
                           SimulatedNetworksByNetworkName>;
    SimulatedNetworksByTypeAndNetworkName _simulatedNetworks;
};

NetworkSimulator::NetworkSimulator(SilKit_Participant* participant)
{
    const auto returnCode = SilKit_Experimental_NetworkSimulator_Create(&_networkSimulator, participant);
    ThrowOnError(returnCode);
}

void NetworkSimulator::SimulateNetwork(
    const std::string& networkName, SilKit::Experimental::NetworkSimulation::SimulatedNetworkType networkType,
    std::unique_ptr<SilKit::Experimental::NetworkSimulation::ISimulatedNetwork> simulatedNetwork)
{
    if (simulatedNetwork == nullptr)
    {
        const std::string errorMsg = "NetworkSimulation: Provided simulated network must not be null.";
        throw SilKitError{errorMsg};
    }

    auto userSimulatedNetwork = simulatedNetwork.get();

    auto networksOfType_it = _simulatedNetworks.find(networkType);
    if (networksOfType_it != _simulatedNetworks.end())
    {
        auto simulatedNetwork_it = networksOfType_it->second.find(networkName);
        if (simulatedNetwork_it != networksOfType_it->second.end())
        {
            const std::string errorMsg =
                "Network '" + networkName + "' of type " + to_string(networkType) + " is already simulated.";
            throw SilKitError{errorMsg};
        }
    }
    _simulatedNetworks[networkType][networkName] = std::move(simulatedNetwork);

    auto returnCode = SilKit_ReturnCode_SUCCESS;

    switch (networkType)
    {
    case SilKit::Experimental::NetworkSimulation::SimulatedNetworkType::CAN:
        const static auto userSimulatedCanNetworkFunctions =
            MakeSimulatedNetworkFunctions(SilKit::Experimental::NetworkSimulation::SimulatedNetworkType::CAN);
        returnCode = SilKit_Experimental_NetworkSimulator_SimulateNetwork(
            _networkSimulator, networkName.c_str(), static_cast<SilKit_Experimental_SimulatedNetworkType>(networkType),
            userSimulatedNetwork, &userSimulatedCanNetworkFunctions);
        break;
    case SilKit::Experimental::NetworkSimulation::SimulatedNetworkType::FlexRay:
        const static auto userSimulatedFlexRayNetworkFunctions =
            MakeSimulatedNetworkFunctions(SilKit::Experimental::NetworkSimulation::SimulatedNetworkType::FlexRay);
        returnCode = SilKit_Experimental_NetworkSimulator_SimulateNetwork(
            _networkSimulator, networkName.c_str(), static_cast<SilKit_Experimental_SimulatedNetworkType>(networkType),
            userSimulatedNetwork, &userSimulatedFlexRayNetworkFunctions);
        break;
    case SilKit::Experimental::NetworkSimulation::SimulatedNetworkType::Ethernet:
        const static auto userSimulatedEthernetNetworkFunctions =
            MakeSimulatedNetworkFunctions(SilKit::Experimental::NetworkSimulation::SimulatedNetworkType::Ethernet);
        returnCode = SilKit_Experimental_NetworkSimulator_SimulateNetwork(
            _networkSimulator, networkName.c_str(), static_cast<SilKit_Experimental_SimulatedNetworkType>(networkType),
            userSimulatedNetwork, &userSimulatedEthernetNetworkFunctions);
        break;
    case SilKit::Experimental::NetworkSimulation::SimulatedNetworkType::LIN:
        const static auto userSimulatedLinNetworkFunctions =
            MakeSimulatedNetworkFunctions(SilKit::Experimental::NetworkSimulation::SimulatedNetworkType::LIN);
        returnCode = SilKit_Experimental_NetworkSimulator_SimulateNetwork(
            _networkSimulator, networkName.c_str(), static_cast<SilKit_Experimental_SimulatedNetworkType>(networkType),
            userSimulatedNetwork, &userSimulatedLinNetworkFunctions);
        break;

    default:
        break;
    }

    ThrowOnError(returnCode);
}

void NetworkSimulator::Start()
{
    const auto returnCode = SilKit_Experimental_NetworkSimulator_Start(_networkSimulator);
    ThrowOnError(returnCode);
}

} // namespace NetworkSimulation
} // namespace Experimental
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
