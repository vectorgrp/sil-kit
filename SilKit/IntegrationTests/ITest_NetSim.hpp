// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <thread>
#include <string>
#include <chrono>
#include <iostream>
#include <atomic>

#include "gtest/gtest.h"

#include "ITestFixture.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/experimental/netsim/all.hpp"

namespace {


// ----------------------------------
// CAN
// ----------------------------------

struct CallCountsNetSimCan
{
    std::atomic_uint OnSetControllerMode{0};
    std::atomic_uint OnSetBaudrate{0};
    std::atomic_uint OnFrameRequest{0};

    void Reset()
    {
        OnSetControllerMode = 0;
        OnSetBaudrate = 0;
        OnFrameRequest = 0;
    }
};

struct CallCountsSilKitHandlersCan
{
    std::atomic_uint FrameHandler{0};
    std::atomic_uint FrameTransmitHandler{0};
    std::atomic_uint StateChangeHandler{0};
    std::atomic_uint ErrorStateChangeHandler{0};

    void Reset()
    {
        FrameHandler = 0;
        FrameTransmitHandler = 0;
        StateChangeHandler = 0;
        ErrorStateChangeHandler = 0;
    }
};

struct CallCountsSilKitSentMsgCan
{
    std::atomic_uint SentFramesSimulated{0};
    std::atomic_uint SentFramesTrivial{0};

    void Reset()
    {
        SentFramesSimulated = 0;
        SentFramesTrivial = 0;
    }
};

// ----------------------------------
// Ethernet
// ----------------------------------

struct CallCountsNetSimEthernet
{
    std::atomic_uint OnSetControllerMode{0};
    std::atomic_uint OnFrameRequest{0};

    void Reset()
    {
        OnSetControllerMode = 0;
        OnFrameRequest = 0;
    }
};

struct CallCountsSilKitHandlersEthernet
{
    std::atomic_uint FrameHandler{0};
    std::atomic_uint FrameTransmitHandler{0};
    std::atomic_uint StateChangeHandler{0};
    std::atomic_uint BitrateChangeHandler{0};

    void Reset()
    {
        FrameHandler = 0;
        FrameTransmitHandler = 0;
        StateChangeHandler = 0;
        BitrateChangeHandler = 0;
    }
};

struct CallCountsSilKitSentMsgEthernet
{
    std::atomic_uint SentFramesSimulated{0};
    std::atomic_uint SentFramesTrivial{0};

    void Reset()
    {
        SentFramesSimulated = 0;
        SentFramesTrivial = 0;
    }
};

// ----------------------------------
// Lin
// ----------------------------------

struct CallCountsNetSimLin
{
    std::atomic_uint OnFrameRequest{0};
    std::atomic_uint OnFrameHeaderRequest{0};
    std::atomic_uint OnWakeupPulse{0};
    std::atomic_uint OnControllerConfig{0};
    std::atomic_uint OnFrameResponseUpdate{0};
    std::atomic_uint OnControllerStatusUpdate{0};

    void Reset()
    {
        OnFrameRequest = 0;
        OnFrameHeaderRequest = 0;
        OnWakeupPulse = 0;
        OnControllerConfig = 0;
        OnFrameResponseUpdate = 0;
        OnControllerStatusUpdate = 0;
    }
};

struct CallCountsSilKitHandlersLin
{
    std::atomic_uint FrameStatusHandler{0};
    std::atomic_uint FrameHeaderHandler{0};
    std::atomic_uint GoToSleepHandler{0};
    std::atomic_uint WakeupHandler{0};

    void Reset()
    {
        FrameStatusHandler = 0;
        FrameHeaderHandler = 0;
        GoToSleepHandler = 0;
        WakeupHandler = 0;
    }
};

struct CallCountsSilKitSentMsgLin
{
    std::atomic_uint SentFramesSimulated{0};
    std::atomic_uint SentFramesTrivial{0};
    std::atomic_uint SentFrameHeadersSimulated{0};
    std::atomic_uint SentFrameHeadersTrivial{0};

    void Reset()
    {
        SentFramesSimulated = 0;
        SentFramesTrivial = 0;
        SentFrameHeadersSimulated = 0;
        SentFrameHeadersTrivial = 0;
    }
};


// ----------------------------------
// FlexRay
// ----------------------------------


struct CallCountsNetSimFlexray
{
    std::atomic_uint OnHostCommand{0};
    std::atomic_uint OnControllerConfig{0};
    std::atomic_uint OnTxBufferConfigUpdate{0};
    std::atomic_uint OnTxBufferUpdate{0};

    void Reset()
    {
        OnHostCommand = 0;
        OnControllerConfig = 0;
        OnTxBufferConfigUpdate = 0;
        OnTxBufferUpdate = 0;
    }
};

struct CallCountsSilKitHandlersFlexray
{
    std::atomic_uint FrameHandler{0};
    std::atomic_uint FrameTransmitHandler{0};
    std::atomic_uint WakeupHandler{0};
    std::atomic_uint PocStatusHandler{0};
    std::atomic_uint SymbolHandler{0};
    std::atomic_uint SymbolTransmitHandler{0};
    std::atomic_uint CycleStartHandler{0};

    void Reset()
    {
        FrameHandler = 0;
        FrameTransmitHandler = 0;
        WakeupHandler = 0;
        PocStatusHandler = 0;
        SymbolHandler = 0;
        SymbolTransmitHandler = 0;
        CycleStartHandler = 0;
    }
};

struct CallCountsSilKitSentMsgFlexray
{
    std::atomic_uint SentFrames{0};

    void Reset()
    {
        SentFrames = 0;
    }
};

// ----------------------------------
// Common
// ----------------------------------

struct CallCountsSimulatedNetwork
{
    std::atomic_uint ProvideSimulatedController{0};
    std::atomic_uint EventProducer{0};
    std::atomic_uint SimulatedControllerRemoved{0};

    void Reset()
    {
        ProvideSimulatedController = 0;
        EventProducer = 0;
        SimulatedControllerRemoved = 0;
    }
};

struct CallCounts
{
    CallCountsSimulatedNetwork simulatedNetwork{};

    // CAN
    CallCountsNetSimCan netSimCan{};
    CallCountsSilKitSentMsgCan silKitSentMsgCan{};
    CallCountsSilKitHandlersCan silKitHandlersCanSimulated{};
    CallCountsSilKitHandlersCan silKitHandlersCanTrivial{};

    // Ethernet
    CallCountsNetSimEthernet netSimEthernet{};
    CallCountsSilKitSentMsgEthernet silKitSentMsgEthernet{};
    CallCountsSilKitHandlersEthernet silKitHandlersEthernetSimulated{};
    CallCountsSilKitHandlersEthernet silKitHandlersEthernetTrivial{};

    // Lin
    CallCountsNetSimLin netSimLin{};
    CallCountsSilKitSentMsgLin silKitSentMsgLin{};
    CallCountsSilKitHandlersLin silKitHandlersLinSimulated{};
    CallCountsSilKitHandlersLin silKitHandlersLinTrivial{};

    // FlexRay
    CallCountsNetSimFlexray netSimFlexray{};
    CallCountsSilKitSentMsgFlexray silKitSentMsgFlexray{};
    CallCountsSilKitHandlersFlexray silKitHandlersFlexray{};

    void Reset()
    {
        simulatedNetwork.Reset();

        netSimCan.Reset();
        silKitHandlersCanSimulated.Reset();
        silKitHandlersCanTrivial.Reset();
        silKitSentMsgCan.Reset();

        netSimEthernet.Reset();
        silKitHandlersEthernetSimulated.Reset();
        silKitHandlersEthernetTrivial.Reset();
        silKitSentMsgEthernet.Reset();

        netSimLin.Reset();
        silKitHandlersLinSimulated.Reset();
        silKitHandlersLinTrivial.Reset();
        silKitSentMsgLin.Reset();

        netSimFlexray.Reset();
        silKitSentMsgFlexray.Reset();
        silKitHandlersFlexray.Reset();
    }
};

CallCounts callCounts{};

using namespace SilKit::Experimental::NetworkSimulation;
using namespace SilKit::Experimental::NetworkSimulation::Can;
using namespace SilKit::Experimental::NetworkSimulation::Ethernet;
using namespace SilKit::Experimental::NetworkSimulation::Lin;
using namespace SilKit::Experimental::NetworkSimulation::Flexray;
using namespace SilKit::Tests;

auto suffixedStrings(std::string body, size_t num) -> std::vector<std::string>
{
    std::vector<std::string> res{};
    auto firstLetter = 'A';
    auto lastLetter = static_cast<char>(firstLetter + num - 1);
    for (char suffix = firstLetter; suffix <= lastLetter; ++suffix)
    {
        res.emplace_back(body + suffix);
    }
    return res;
}

struct ITest_NetSim : ITest_SimTestHarness
{
    using ITest_SimTestHarness::ITest_SimTestHarness;

    void SetUp() override
    {
        callCounts.Reset();

        _participantNamesSimulated = suffixedStrings("ParticipantSimulated", _numParticipantsSimulated);
        _participantNamesTrivial = suffixedStrings("ParticipantTrivial", _numParticipantsTrivial);
        _emptyNetworks = suffixedStrings("EmtpyNetwork", _numEmptyNetworks);

        _sendUntilMs = std::chrono::milliseconds{_numSimSteps};
        _stopAtMs = std::chrono::milliseconds{_numSimSteps + 1}; // Stop in NetSim one step later to receive all

        // System controller setup
        {
            auto syncParticipantNames = _participantNamesSimulated;
            syncParticipantNames.insert(syncParticipantNames.end(), _participantNamesTrivial.begin(),
                                        _participantNamesTrivial.end());
            syncParticipantNames.emplace_back(_participantNameNetSim);
            SetupFromParticipantList(syncParticipantNames);
        }
    }

    const size_t _numParticipantsSimulated = 3;
    const size_t _numParticipantsTrivial = 4;
    const size_t _numEmptyNetworks = 3;

    const std::string _participantNameNetSim = "NetworkSimulator";
    const std::string _simulatedNetworkName = "SIMULATED";
    const std::string _trivialNetworkName = "TRIVIAL";

    // Deterministic send/receive setup
    const size_t _numSimulatedNetworks = 1;
    const size_t _numSimSteps = 5;
    const std::chrono::milliseconds _stepSize{1};
    const size_t _numFramesPerSimStep = 5;

    std::chrono::milliseconds _sendUntilMs;
    std::chrono::milliseconds _stopAtMs;

    std::vector<std::string> _participantNamesSimulated;
    std::vector<std::string> _participantNamesTrivial;
    std::vector<std::string> _emptyNetworks;
};


class MySimulatedNetwork;

class MySimulatedController
{
protected:
    ControllerDescriptor _controllerDescriptor;
    MySimulatedNetwork* _mySimulatedNetwork;

public:
    MySimulatedController(MySimulatedNetwork* mySimulatedNetwork, ControllerDescriptor controllerDescriptor)
        : _controllerDescriptor{controllerDescriptor}
        , _mySimulatedNetwork{mySimulatedNetwork}
    {
    }
};

class MySimulatedNetwork : public ISimulatedNetwork
{
    SimulatedNetworkType _networkType;
    std::string _networkName;
    bool _isLinDynamic;

    std::unique_ptr<IEventProducer> _eventProducer;
    std::vector<ControllerDescriptor> _controllerDescriptors;
    std::vector<std::unique_ptr<ISimulatedController>> _mySimulatedControllers;

public:
    MySimulatedNetwork(SimulatedNetworkType networkType, std::string networkName, bool isLinDynamic = false)
        : _networkType{networkType}
        , _networkName{networkName}
        , _isLinDynamic{isLinDynamic}
    {
        (void)_isLinDynamic;
    }

    // ISimulatedNetwork
    void SetEventProducer(std::unique_ptr<IEventProducer> eventProducer) override;
    auto ProvideSimulatedController(ControllerDescriptor controllerDescriptor) -> ISimulatedController* override;
    void SimulatedControllerRemoved(ControllerDescriptor controllerDescriptor) override;

    auto GetCanEventProducer()
    {
        return static_cast<ICanEventProducer*>(_eventProducer.get());
    }
    auto GetEthernetEventProducer()
    {
        return static_cast<IEthernetEventProducer*>(_eventProducer.get());
    }
    auto GetLinEventProducer()
    {
        return static_cast<ILinEventProducer*>(_eventProducer.get());
    }
    auto GetFlexRayEventProducer()
    {
        return static_cast<IFlexRayEventProducer*>(_eventProducer.get());
    }

    auto GetAllControllerDescriptors()
    {
        return SilKit::Util::ToSpan(_controllerDescriptors);
    }
};

// ISimulatedNetwork

void MySimulatedNetwork::SetEventProducer(std::unique_ptr<IEventProducer> eventProducer)
{
    callCounts.simulatedNetwork.EventProducer++;

    _eventProducer = std::move(eventProducer);
}

void MySimulatedNetwork::SimulatedControllerRemoved(ControllerDescriptor /*controllerDescriptor*/)
{
    callCounts.simulatedNetwork.SimulatedControllerRemoved++;
}

} //end namespace
