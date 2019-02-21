// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IComAdapter.hpp"

#include <memory>
#include <vector>
#include <unordered_map>
#include <tuple>

#include "ib/cfg/Config.hpp"
#include "ib/mw/all.hpp"
#include "ib/sim/all.hpp"

#include "fastrtps_fwd.h"
#include "fastrtps/publisher/PublisherListener.h"
#include "fastrtps/subscriber/SubscriberListener.h"

#include "idl/all.hpp"

#include "IdlTypeConversion.hpp"
#include "IbSubListener.hpp"
#include "memory_fastrtps.hpp"
#include "FastRtpsGuard.hpp"

namespace ib {
namespace mw {

class FastRtpsComAdapter : public IComAdapter
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    FastRtpsComAdapter() = default;
    FastRtpsComAdapter(const FastRtpsComAdapter&) = default;
    FastRtpsComAdapter(FastRtpsComAdapter&&) = default;
    FastRtpsComAdapter(cfg::Config config, const std::string& participantName);

public:
    // ----------------------------------------
    // Operator Implementations
    FastRtpsComAdapter& operator=(FastRtpsComAdapter& other) = default;
    FastRtpsComAdapter& operator=(FastRtpsComAdapter&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IComAdapter
    auto CreateCanController(const std::string& canonicalName) -> sim::can::ICanController* override;
    auto CreateEthController(const std::string& canonicalName) -> sim::eth::IEthController* override;
    auto CreateFlexrayController(const std::string& canonicalName) -> sim::fr::IFrController* override;
    auto CreateLinController(const std::string& canonicalName) -> sim::lin::ILinController* override;
    auto CreateAnalogIn(const std::string& canonicalName) -> sim::io::IAnalogInPort* override;
    auto CreateDigitalIn(const std::string& canonicalName) -> sim::io::IDigitalInPort* override;
    auto CreatePwmIn(const std::string& canonicalName) -> sim::io::IPwmInPort* override;
    auto CreatePatternIn(const std::string& canonicalName) -> sim::io::IPatternInPort* override;
    auto CreateAnalogOut(const std::string& canonicalName) -> sim::io::IAnalogOutPort* override;
    auto CreateDigitalOut(const std::string& canonicalName) -> sim::io::IDigitalOutPort* override;
    auto CreatePwmOut(const std::string& canonicalName) -> sim::io::IPwmOutPort* override;
    auto CreatePatternOut(const std::string& canonicalName) -> sim::io::IPatternOutPort* override;
    auto CreateGenericPublisher(const std::string& canonicalName) -> sim::generic::IGenericPublisher* override;
    auto CreateGenericSubscriber(const std::string& canonicalName) -> sim::generic::IGenericSubscriber* override;
    auto GetSyncMaster() -> sync::ISyncMaster* override;
    auto GetParticipantController() -> sync::IParticipantController* override;
    auto GetSystemMonitor() -> sync::ISystemMonitor* override;
    auto GetSystemController() -> sync::ISystemController* override;

    void RegisterCanSimulator(sim::can::IIbToCanSimulator* busSim) override;
    void RegisterEthSimulator(sim::eth::IIbToEthSimulator* busSim) override;
    void RegisterFlexraySimulator(sim::fr::IIbToFrBusSimulator* busSim) override;
    void RegisterLinSimulator(sim::lin::IIbToLinSimulator* busSim) override;

    void SendIbMessage(EndpointAddress from, const sim::can::CanMessage& msg) override;
    void SendIbMessage(EndpointAddress from, sim::can::CanMessage&& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::can::CanTransmitAcknowledge& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::can::CanControllerStatus& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::can::CanConfigureBaudrate& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::can::CanSetControllerMode& msg) override;

    void SendIbMessage(EndpointAddress from, const sim::eth::EthMessage& msg) override;
    void SendIbMessage(EndpointAddress from, sim::eth::EthMessage&& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::eth::EthTransmitAcknowledge& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::eth::EthStatus& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::eth::EthSetMode& msg) override;

    void SendIbMessage(EndpointAddress from, const sim::fr::FrMessage& msg) override;
    void SendIbMessage(EndpointAddress from, sim::fr::FrMessage&& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::fr::FrMessageAck& msg) override;
    void SendIbMessage(EndpointAddress from, sim::fr::FrMessageAck&& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::fr::FrSymbol& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::fr::FrSymbolAck& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::fr::HostCommand& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::fr::ControllerConfig& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::fr::TxBufferUpdate& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::fr::ControllerStatus& msg) override;

    void SendIbMessage(EndpointAddress from, const sim::lin::LinMessage& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::lin::RxRequest& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::lin::TxAcknowledge& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::lin::WakeupRequest& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::lin::ControllerConfig& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::lin::SlaveConfiguration& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::lin::SlaveResponse& msg) override;

    void SendIbMessage(EndpointAddress from, const sim::io::AnalogIoMessage& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::io::DigitalIoMessage& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::io::PatternIoMessage& msg) override;
    void SendIbMessage(EndpointAddress from, sim::io::PatternIoMessage&& msg) override;
    void SendIbMessage(EndpointAddress from, const sim::io::PwmIoMessage& msg) override;

    void SendIbMessage(EndpointAddress from, const sync::Tick& msg) override;
    void SendIbMessage(EndpointAddress from, const sync::TickDone& msg) override;
    void SendIbMessage(EndpointAddress from, const sync::QuantumRequest& msg) override;
    void SendIbMessage(EndpointAddress from, const sync::QuantumGrant& msg) override;
    void SendIbMessage(EndpointAddress from, const sync::ParticipantStatus& msg) override;
    void SendIbMessage(EndpointAddress from, const sync::ParticipantCommand& msg) override;
    void SendIbMessage(EndpointAddress from, const sync::SystemCommand& msg) override;

    void SendIbMessage(EndpointAddress from, const sim::generic::GenericMessage& msg) override;
    void SendIbMessage(EndpointAddress from, sim::generic::GenericMessage&& msg) override;

    void WaitForMessageDelivery() override;
    void FlushSendBuffers() override;

public:
    // ----------------------------------------
    // Public methods

    /*! \brief Join the middleware domain as a participant.
    * 
    * Join the middleware domain and become a participant.
    * \param domainId ID of the domain
    * 
    * \throw std::exception A participant was created previously, or a
    * participant could not be created.
    */
    void joinIbDomain(uint32_t domainId);

private:
    // ----------------------------------------
    // private datatypes
    struct RtpsPubListener
    {
        std::unique_ptr<eprosima::fastrtps::PublisherListener> listener;
        FastRtps::unique_ptr<eprosima::fastrtps::Publisher> publisher;
    };

    template<typename TopicT>
    struct RtpsSubListener
    {
        using TopicType = TopicT;
        IbSubListener<TopicType> listener;
        FastRtps::unique_ptr<eprosima::fastrtps::Subscriber> subscriber;
    };

    template<typename TopicT>
    struct RtpsTopics
    {
        using TopicType = TopicT;

        std::unique_ptr<eprosima::fastrtps::TopicDataType> pubSubType;

        std::unordered_map<std::string, RtpsPubListener> pubListeners;
        std::unordered_map<std::string, RtpsSubListener<TopicType>> subListeners;

        std::unordered_map<EndpointId, eprosima::fastrtps::Publisher*> endpointToPublisherMap;
    };

    template<typename ControllerT>
    using ControllerMap = std::unordered_map<EndpointId, std::unique_ptr<ControllerT>>;

private:
    // ----------------------------------------
    // private methods
    void joinDomain(uint32_t domainId);
    void OnFastrtpsDomainJoined();

    void registerTopicTypeIfNecessary(eprosima::fastrtps::TopicDataType* topicType);
    template <class AttrT>
    void SetupPubSubAttributes(AttrT& attributes, const std::string& topicName, eprosima::fastrtps::TopicDataType* topicType);
    auto MakeFastrtpsProfileName(const std::string& topicName, eprosima::fastrtps::TopicDataType* topicType) -> std::string;
    auto createPublisher(const std::string& topicName, eprosima::fastrtps::TopicDataType* topicType, eprosima::fastrtps::PublisherListener* listener = nullptr) -> eprosima::fastrtps::Publisher*;
    auto createSubscriber(const std::string& topicName, eprosima::fastrtps::TopicDataType* topicType, eprosima::fastrtps::SubscriberListener* listener = nullptr) -> eprosima::fastrtps::Subscriber*;

    template<class MsgT, class ConfigT>
    auto CreateInPort(const ConfigT& config) -> sim::io::IInPort<MsgT>*;
    template<class MsgT, class ConfigT>
    auto CreateOutPort(const ConfigT& config) -> sim::io::IOutPort<MsgT>*;

    template<class ControllerT>
    auto GetController(EndpointId endpointId) -> ControllerT*;
    template<class ControllerT, typename... Arg>
    auto CreateController(EndpointId endpointId, const std::string& topicname, Arg&&... arg) -> ControllerT*;

    auto GetLinkById(int16_t linkId) -> cfg::Link&;

    template<class ControllerT, class ConfigT, typename... Arg>
    auto CreateControllerForLink(const ConfigT& config, Arg&&... arg) -> ControllerT*;

    template<class IIbToSimulatorT>
    void RegisterSimulator(IIbToSimulatorT* busSim, cfg::Link::Type linkType);

    bool useNetworkSimulator() const;
    bool isSyncMaster() const;

    // --------------------------------------------------------------------------------
    // Manage RtpsTopics, which supports multiple topics with the same topic type.
    // --------------------------------------------------------------------------------
    template <class IdlMessageT>
    void PublishRtpsTopic(const std::string& topicName, EndpointId endpointId);
    template <class IdlMessageT>
    void SubscribeRtpsTopic(const std::string& topicName, IIbMessageReceiver<to_ib_message_t<IdlMessageT>>* receiver);

    template <class IControllerT>
    void PublishRtpsTopics(const std::string& topicName, EndpointId endpointId);
    template<class EndpointT>
    void SubscribeRtpsTopics(const std::string& topicName, EndpointT* receiver);

    template<typename IbMessageT>
    void SendIbMessageImpl(EndpointAddress from, IbMessageT&& msg);

private:
    // ----------------------------------------
    // private members
    FastRtps::FastRtpsGuard _fastRtpsGuard;

    cfg::Config _config;
    const cfg::Participant* _participant{nullptr};
    std::string _participantName;
    ParticipantId _participantId{0};
           
    std::tuple<
        ControllerMap<sim::can::IIbToCanController>,
        ControllerMap<sim::can::IIbToCanControllerProxy>,
        ControllerMap<sim::eth::IIbToEthController>,
        ControllerMap<sim::eth::IIbToEthControllerProxy>,
        ControllerMap<sim::fr::IIbToFrController>,
        ControllerMap<sim::fr::IIbToFrControllerProxy>,
        ControllerMap<sim::lin::IIbToLinController>,
        ControllerMap<sim::lin::IIbToLinControllerProxy>,
        ControllerMap<sim::generic::IIbToGenericPublisher>,
        ControllerMap<sim::generic::IIbToGenericSubscriber>,
        ControllerMap<sim::io::IIbToInPort<sim::io::DigitalIoMessage>>,
        ControllerMap<sim::io::IIbToInPort<sim::io::AnalogIoMessage>>,
        ControllerMap<sim::io::IIbToInPort<sim::io::PwmIoMessage>>,
        ControllerMap<sim::io::IIbToInPort<sim::io::PatternIoMessage>>,
        ControllerMap<sim::io::IIbToOutPort<sim::io::DigitalIoMessage>>,
        ControllerMap<sim::io::IIbToOutPort<sim::io::AnalogIoMessage>>,
        ControllerMap<sim::io::IIbToOutPort<sim::io::PwmIoMessage>>,
        ControllerMap<sim::io::IIbToOutPort<sim::io::PatternIoMessage>>,
        ControllerMap<sync::IParticipantController>,
        ControllerMap<sync::ISystemMonitor>,
        ControllerMap<sync::ISystemController>,
        ControllerMap<sync::ISyncMaster>
    > _controllers;

    std::tuple<
        sim::can::IIbToCanSimulator*,
        sim::eth::IIbToEthSimulator*,
        sim::fr::IIbToFrBusSimulator*,
        sim::lin::IIbToLinSimulator*
    > _simulators {nullptr, nullptr, nullptr, nullptr};

    FastRtps::unique_ptr<eprosima::fastrtps::Participant> _fastRtpsParticipant;

    std::tuple<
        RtpsTopics<sim::can::idl::CanMessage>,
        RtpsTopics<sim::can::idl::CanTransmitAcknowledge>,
        RtpsTopics<sim::can::idl::CanControllerStatus>,
        RtpsTopics<sim::can::idl::CanConfigureBaudrate>,
        RtpsTopics<sim::can::idl::CanSetControllerMode>,
        RtpsTopics<sim::eth::idl::EthMessage>,
        RtpsTopics<sim::eth::idl::EthTransmitAcknowledge>,
        RtpsTopics<sim::eth::idl::EthStatus>,
        RtpsTopics<sim::eth::idl::EthSetMode>,
        RtpsTopics<sim::fr::idl::FrMessage>,
        RtpsTopics<sim::fr::idl::FrMessageAck>,
        RtpsTopics<sim::fr::idl::FrSymbol>,
        RtpsTopics<sim::fr::idl::FrSymbolAck>,
        RtpsTopics<sim::fr::idl::HostCommand>,
        RtpsTopics<sim::fr::idl::ControllerConfig>,
        RtpsTopics<sim::fr::idl::TxBufferUpdate>,
        RtpsTopics<sim::fr::idl::ControllerStatus>,
        RtpsTopics<sim::lin::idl::LinMessage>,
        RtpsTopics<sim::lin::idl::RxRequest>,
        RtpsTopics<sim::lin::idl::TxAcknowledge>,
        RtpsTopics<sim::lin::idl::WakeupRequest>,
        RtpsTopics<sim::lin::idl::ControllerConfig>,
        RtpsTopics<sim::lin::idl::SlaveConfiguration>,
        RtpsTopics<sim::lin::idl::SlaveResponse>,
        RtpsTopics<sim::generic::idl::GenericMessage>,
        RtpsTopics<sim::io::idl::AnalogIoMessage>,
        RtpsTopics<sim::io::idl::DigitalIoMessage>,
        RtpsTopics<sim::io::idl::PatternIoMessage>,
        RtpsTopics<sim::io::idl::PwmIoMessage>,
        RtpsTopics<sync::idl::ParticipantCommand>,
        RtpsTopics<sync::idl::SystemCommand>,
        RtpsTopics<sync::idl::Tick>,
        RtpsTopics<sync::idl::TickDone>,
        RtpsTopics<sync::idl::QuantumGrant>,
        RtpsTopics<sync::idl::QuantumRequest>,
        RtpsTopics<sync::idl::ParticipantStatus>
    > _rtpsTopics;

    std::vector<eprosima::fastrtps::Publisher*> _allPublishers;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // mw
} // namespace ib
