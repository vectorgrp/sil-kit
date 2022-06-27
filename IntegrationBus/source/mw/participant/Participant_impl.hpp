// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <cassert>
#include <sstream>
#include <chrono>

#include "CanController.hpp"
#include "EthController.hpp"
#include "FlexrayController.hpp"
#include "LinController.hpp"
//#include "LinControllerReplay.hpp"
#include "DataPublisher.hpp"
#include "DataSubscriber.hpp"
#include "DataSubscriberInternal.hpp"
#include "RpcClient.hpp"
#include "RpcServer.hpp"
#include "RpcServerInternal.hpp"
#include "RpcDiscoverer.hpp"

#include "LifecycleService.hpp"
#include "SystemController.hpp"
#include "SystemMonitor.hpp"
#include "LogMsgSender.hpp"
#include "LogMsgReceiver.hpp"
#include "Logger.hpp"
#include "TimeProvider.hpp"
#include "TimeSyncService.hpp"
#include "ServiceDiscovery.hpp"
#include "ParticipantConfiguration.hpp"
#include "YamlParser.hpp"

#include "tuple_tools/bind.hpp"
#include "tuple_tools/for_each.hpp"
#include "tuple_tools/predicative_get.hpp"

#include "ib/version.hpp"

#include "Participant.hpp"

#include "MessageTracing.hpp" // log tracing
#include "UuidRandom.hpp"

namespace ib {
namespace mw {

using namespace ib::sim;
using namespace std::chrono_literals;

namespace tt = util::tuple_tools;

// Anonymous namespace for Helper Traits and Functions
namespace {

template<class T, class U>
struct IsControllerMap : std::false_type {};
template<class T, class U>
struct IsControllerMap<std::unordered_map<std::string, std::unique_ptr<T>>, U> : std::is_base_of<T, U> {};

} // namespace anonymous

template <class IbConnectionT>
Participant<IbConnectionT>::Participant(cfg::ParticipantConfiguration participantConfig,
                                        const std::string& participantName, ProtocolVersion version)
    : _participantName{participantName}
    , _participantConfig{participantConfig}
    , _participantId{util::hash::Hash(participantName)}
    , _ibConnection{_participantConfig, participantName, _participantId, version}
{
    std::string logParticipantNotice; //!< We defer logging the notice until the logger is created
    if (!_participantConfig.participantName.empty() && _participantConfig.participantName != participantName)
    {
        logParticipantNotice = fmt::format(
            "The provided participant name '{}' differs from the configured name '{}'. The latter will be used.",
            _participantName, _participantConfig.participantName);
        _participantName = _participantConfig.participantName;
    }
    // NB: do not create the _logger in the initializer list. If participantName is empty,
    //  this will cause a fairly unintuitive exception in spdlog.
    _logger = std::make_unique<logging::Logger>(_participantName, _participantConfig.logging);
    _ibConnection.SetLogger(_logger.get());

    _logger->Info("Creating Participant for Participant {}, IntegrationBus-Version: {}, Middleware: {}",
                  _participantName, version::String(), "VAsio");

    //set up default time provider used for controller instantiation
    _timeProvider = std::make_shared<sync::WallclockProvider>(1ms);

    if (!logParticipantNotice.empty())
    {
        _logger->Info(logParticipantNotice);
    }
}

template <class IbConnectionT>
void Participant<IbConnectionT>::joinIbDomain(uint32_t domainId)
{
    _ibConnection.JoinDomain(domainId);
    onIbDomainJoined();

    _logger->Info("Participant {} has joined the IB-Domain {}", _participantName, domainId);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::onIbDomainJoined()
{
    SetupRemoteLogging();

    //Ensure Service discovery is started
    (void)GetServiceDiscovery();

    // Create the participants trace message sinks as declared in the configuration.
    //_traceSinks = tracing::CreateTraceMessageSinks(GetLogger(), _config, participant);

    // NB: Create the lifecycleService to prevent nested controller creation in SystemMonitor
    auto* lifecycleService = GetLifecycleService();

    auto* timeSyncService = dynamic_cast<mw::sync::TimeSyncService*>(lifecycleService->GetTimeSyncService());

    _timeProvider = timeSyncService->GetTimeProvider();
    _logger->Info("Time provider: {}", _timeProvider->TimeProviderName());

    //// Enable replaying mechanism.
    //const auto& participantConfig = get_by_name(_config.simulationSetup.participants, _participantName);
    //if (tracing::HasReplayConfig(participantConfig))
    //{
    //    _replayScheduler = std::make_unique<tracing::ReplayScheduler>(_config,
    //        participantConfig,
    //        _config.simulationSetup.timeSync.tickPeriod,
    //        this,
    //        _timeProvider.get()
    //    );
    //    _logger->Info("Replay Scheduler active.");
    //}

    // Ensure shutdowns are cleanly handled.
    auto&& monitor = GetSystemMonitor();
    monitor->AddSystemStateHandler([&conn = GetIbConnection()](auto newState) {
        if (newState == sync::SystemState::ShuttingDown)
        {
            conn.NotifyShutdown();
        }
    });
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SetupRemoteLogging()
{
    auto* logger = dynamic_cast<logging::Logger*>(_logger.get());
    if (logger)
    {
        if (_participantConfig.logging.logFromRemotes)
        {
            mw::SupplementalData supplementalData;
            supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeLoggerReceiver;

            CreateInternalController<logging::LogMsgReceiver>("LogMsgReceiver", mw::ServiceType::InternalController,
                                                      std::move(supplementalData), true, logger);
        }

        auto sinkIter = std::find_if(_participantConfig.logging.sinks.begin(), _participantConfig.logging.sinks.end(),
            [](const cfg::Sink& sink) { return sink.type == cfg::Sink::Type::Remote; });

        if (sinkIter != _participantConfig.logging.sinks.end())
        {
            mw::SupplementalData supplementalData;
            supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeLoggerSender;

            auto&& logMsgSender = CreateInternalController<logging::LogMsgSender>(
                "LogMsgSender", mw::ServiceType::InternalController, std::move(supplementalData), true);

            logger->RegisterRemoteLogging([logMsgSender](logging::LogMsg logMsg) {

                logMsgSender->SendLogMsg(std::move(logMsg));

            });
        }
    }
    else
    {
        _logger->Warn("Failed to setup remote logging. Participant {} will not send and receive remote logs.", _participantName);
    }
}

template<class IbConnectionT>
inline void Participant<IbConnectionT>::SetTimeProvider(sync::ITimeProvider* newClock)
{
    // register the time provider with all already instantiated controllers
    auto setTimeProvider = [newClock](auto& controllers) {
        for (auto& controller: controllers)
        {
            auto* ctl = dynamic_cast<ib::mw::sync::ITimeConsumer*>(controller.second.get());
            if (ctl)
            {
                ctl->SetTimeProvider(newClock);
            }
        }
    };
    tt::for_each(_controllers, setTimeProvider);
}

template <class IbConnectionT>
template <typename ConfigT>
auto Participant<IbConnectionT>::GetConfigByControllerName(const std::vector<ConfigT>& controllers,
                                                          const std::string& canonicalName) -> ConfigT
{
    ConfigT controllerConfig;
    auto it = std::find_if(controllers.begin(), controllers.end(), [canonicalName](auto&& controllerConfig) {
        return controllerConfig.name == canonicalName;
    });
    if (it != controllers.end())
    {
        controllerConfig = *it;
    }
    else
    {
        // Controller is not found in config. Just set the controller name.
        controllerConfig.name = canonicalName;
    }
    return controllerConfig;
}

template <class IbConnectionT>
template <typename ValueT>
void Participant<IbConnectionT>::UpdateOptionalConfigValue(const std::string& controllerName,
                                                           ib::util::Optional<ValueT>& configuredValue,
                                                           const ValueT& passedValue)
{
    if (!configuredValue.has_value())
    {
        // Optional value is not set. Use passed value.
        configuredValue = passedValue;
    }
    else if (configuredValue.value() != passedValue)
    {
        // Value is configured but differs from passed value. Keep configured value and inform about mismatch.
        LogMismatchBetweenConfigAndPassedValue(controllerName, passedValue, configuredValue.value());
    }
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateCanController(const std::string& canonicalName, const std::string& networkName) -> can::ICanController*
{
    ib::cfg::CanController controllerConfig = GetConfigByControllerName(_participantConfig.canControllers, canonicalName);
    UpdateOptionalConfigValue(canonicalName, controllerConfig.network, networkName);

    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeCan;

    auto controller = CreateController<ib::cfg::CanController, can::CanController>(
        controllerConfig, mw::ServiceType::Controller, std::move(supplementalData), true, controllerConfig,
        _timeProvider.get());

    controller->RegisterServiceDiscovery();
    
    return controller;
}


template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateCanController(const std::string& canonicalName)
    -> can::ICanController*
{
    return CreateCanController(canonicalName, canonicalName);
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateEthernetController(const std::string& canonicalName, const std::string& networkName)
    -> eth::IEthernetController*
{
    ib::cfg::EthernetController controllerConfig = GetConfigByControllerName(_participantConfig.ethernetControllers, canonicalName);
    UpdateOptionalConfigValue(canonicalName, controllerConfig.network, networkName);

    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeEthernet;

    auto controller = CreateController<ib::cfg::EthernetController, eth::EthController>(
        controllerConfig, mw::ServiceType::Controller, std::move(supplementalData), true, controllerConfig,
        _timeProvider.get());

    controller->RegisterServiceDiscovery();
    return controller;
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateEthernetController(const std::string& canonicalName) -> eth::IEthernetController*
{
    return CreateEthernetController(canonicalName, canonicalName);
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateFlexrayController(const std::string& canonicalName, const std::string& networkName)
    -> sim::fr::IFlexrayController*
{
    ib::cfg::FlexrayController controllerConfig = GetConfigByControllerName(_participantConfig.flexrayControllers, canonicalName);
    UpdateOptionalConfigValue(canonicalName, controllerConfig.network, networkName);

    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeFlexray;

    auto controller = CreateController<ib::cfg::FlexrayController, fr::FlexrayController>(
        controllerConfig, mw::ServiceType::Controller, std::move(supplementalData), true, controllerConfig,
        _timeProvider.get());

    controller->RegisterServiceDiscovery();
    return controller;
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateFlexrayController(const std::string& canonicalName) -> sim::fr::IFlexrayController*
{
    return CreateFlexrayController(canonicalName, canonicalName);
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateLinController(const std::string& canonicalName, const std::string& networkName)
    -> lin::ILinController*
{
    ib::cfg::LinController controllerConfig = GetConfigByControllerName(_participantConfig.linControllers, canonicalName);
    UpdateOptionalConfigValue(canonicalName, controllerConfig.network, networkName);

    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeLin;

    auto controller = CreateController<ib::cfg::LinController, lin::LinController>(
        controllerConfig, mw::ServiceType::Controller, std::move(supplementalData), true, controllerConfig,
        _timeProvider.get());

    controller->RegisterServiceDiscovery();

    return controller;
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateLinController(const std::string& canonicalName) -> lin::ILinController*
{
    return CreateLinController(canonicalName, canonicalName);
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateDataSubscriberInternal(const std::string& topic, const std::string& linkName,
                                                             const std::string& mediaType,
                                                             const std::map<std::string, std::string>& publisherLabels,
                                                             sim::data::DataMessageHandlerT defaultHandler,
                                                             sim::data::IDataSubscriber* parent)
    -> sim::data::DataSubscriberInternal*
{
    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeDataSubscriberInternal;

    ib::cfg::DataSubscriber controllerConfig;
    // Use a unique name to avoid collisions of several subscribers on same topic on one participant
    controllerConfig.name = util::uuid::to_string(util::uuid::generate());
    std::string network = linkName;

    return CreateController<ib::cfg::DataSubscriber, sim::data::DataSubscriberInternal>(
        controllerConfig, network, mw::ServiceType::Controller, std::move(supplementalData), true, _timeProvider.get(),
        topic, mediaType, publisherLabels, defaultHandler, parent);
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateDataPublisher(const std::string& canonicalName, const std::string& topic,
    const std::string& mediaType, const std::map<std::string, std::string>& labels,
    size_t history) -> sim::data::IDataPublisher*
{
    if (history > 1)
    {
        throw ib::ConfigurationError("DataPublishers do not support history > 1.");
    }

    std::string network = util::uuid::to_string(util::uuid::generate());

    ib::cfg::DataPublisher controllerConfig = GetConfigByControllerName(_participantConfig.dataPublishers, canonicalName);
    UpdateOptionalConfigValue(canonicalName, controllerConfig.topic, topic);

    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeDataPublisher;
    supplementalData[ib::mw::service::supplKeyDataPublisherTopic] = controllerConfig.topic.value();
    supplementalData[ib::mw::service::supplKeyDataPublisherPubUUID] = network;
    supplementalData[ib::mw::service::supplKeyDataPublisherMediaType] = mediaType;
    auto labelStr = ib::cfg::Serialize<std::decay_t<decltype(labels)>>(labels);
    supplementalData[ib::mw::service::supplKeyDataPublisherPubLabels] = labelStr;

    auto controller = CreateController<ib::cfg::DataPublisher, ib::sim::data::DataPublisher>(
        controllerConfig, network, mw::ServiceType::Controller, std::move(supplementalData), true, _timeProvider.get(),
        controllerConfig.topic.value(), mediaType, labels, network);

    _ibConnection.SetHistoryLengthForLink(network, history, controller);

    return controller;
    
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateDataPublisher(const std::string& canonicalName) -> sim::data::IDataPublisher*
{
    return CreateDataPublisher(canonicalName, canonicalName, { "" }, {}, 0);
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateDataSubscriber(const std::string& canonicalName, const std::string& topic,
                                                      const std::string& mediaType,
                                                      const std::map<std::string, std::string>& labels,
                                                      ib::sim::data::DataMessageHandlerT defaultDataHandler,
                                                      ib::sim::data::NewDataPublisherHandlerT newDataSourceHandler)
    -> sim::data::IDataSubscriber*
{
    ib::cfg::DataSubscriber controllerConfig = GetConfigByControllerName(_participantConfig.dataSubscribers, canonicalName);
    UpdateOptionalConfigValue(canonicalName, controllerConfig.topic, topic);

    // Use unique network name that same topic for multiple DataSubscribers on one participant works
    std::string network = util::uuid::to_string(util::uuid::generate());

    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeDataSubscriber;

    auto controller = CreateController<ib::cfg::DataSubscriber, sim::data::DataSubscriber>(
        controllerConfig, network, mw::ServiceType::Controller, std::move(supplementalData), true, _timeProvider.get(),
        controllerConfig.topic.value(), mediaType, labels, defaultDataHandler, newDataSourceHandler);

    controller->RegisterServiceDiscovery();

    return controller;
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateDataSubscriber(const std::string& canonicalName) -> sim::data::IDataSubscriber*
{
    return CreateDataSubscriber(canonicalName, canonicalName, { "" }, {}, nullptr, nullptr);
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateRpcServerInternal(const std::string& functionName, const std::string& clientUUID,
                                                         const std::string& mediaType,
                                                         const std::map<std::string, std::string>& clientLabels,
                                                         sim::rpc::RpcCallHandler handler,
                                                         sim::rpc::IRpcServer* parent) -> sim::rpc::RpcServerInternal*
{
    _logger->Trace("Creating internal server for functionName={}, clientUUID={}", functionName, clientUUID);

    ib::cfg::RpcServer controllerConfig;
    // Use a unique name to avoid collisions of several RpcSevers on same functionName on one participant
    controllerConfig.name = util::uuid::to_string(util::uuid::generate());
    std::string network = clientUUID;

    // RpcServerInternal gets discovered by RpcClient which is then ready to detach calls
    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeRpcServerInternal;
    supplementalData[ib::mw::service::supplKeyRpcServerInternalClientUUID] = clientUUID;

    return CreateController<ib::cfg::RpcServer, sim::rpc::RpcServerInternal>(
        controllerConfig, network, mw::ServiceType::Controller, std::move(supplementalData), true, _timeProvider.get(),
        functionName, mediaType, clientLabels, clientUUID, handler, parent);
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateRpcClient(const std::string& canonicalName, const std::string& functionName,
                                                 const std::string& mediaType,
                                                 const std::map<std::string, std::string>& labels,
                                                 sim::rpc::RpcCallResultHandler handler) -> sim::rpc::IRpcClient*
{
    auto network = util::uuid::to_string(util::uuid::generate());

    ib::cfg::RpcClient controllerConfig = GetConfigByControllerName(_participantConfig.rpcClients, canonicalName);
    UpdateOptionalConfigValue(canonicalName, controllerConfig.functionName, functionName);

    // RpcClient gets discovered by RpcServer which creates RpcServerInternal on a matching connection
    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeRpcClient;
    supplementalData[ib::mw::service::supplKeyRpcClientFunctionName] = controllerConfig.functionName.value();
    supplementalData[ib::mw::service::supplKeyRpcClientMediaType] = mediaType;
    auto labelStr = ib::cfg::Serialize<std::decay_t<decltype(labels)>>(labels);
    supplementalData[ib::mw::service::supplKeyRpcClientLabels] = labelStr;
    supplementalData[ib::mw::service::supplKeyRpcClientUUID] = network;

    auto controller = CreateController<ib::cfg::RpcClient, ib::sim::rpc::RpcClient>(
        controllerConfig, network, mw::ServiceType::Controller, std::move(supplementalData), true, _timeProvider.get(),
        controllerConfig.functionName.value(), mediaType, labels, network, handler);

    // RpcClient discovers RpcServerInternal and is ready to dispatch calls
    controller->RegisterServiceDiscovery();
 
    return controller;
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateRpcClient(const std::string& canonicalName) -> sim::rpc::IRpcClient*
{
    return CreateRpcClient(canonicalName, canonicalName, { "" }, {}, nullptr);
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateRpcServer(const std::string& canonicalName, const std::string& functionName,
                                                 const std::string& mediaType,
                                                 const std::map<std::string, std::string>& labels,
                                                 sim::rpc::RpcCallHandler handler) -> sim::rpc::IRpcServer*
{
    // Use unique network name that same functionName for multiple RpcServers on one participant works
    std::string network = util::uuid::to_string(util::uuid::generate());

    ib::cfg::RpcServer controllerConfig = GetConfigByControllerName(_participantConfig.rpcServers, canonicalName);
    UpdateOptionalConfigValue(canonicalName, controllerConfig.functionName, functionName);
	
    // RpcServer announces himself to be found by DiscoverRpcServers()
    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeRpcServer;
    supplementalData[ib::mw::service::supplKeyRpcServerFunctionName] = controllerConfig.functionName.value();
    supplementalData[ib::mw::service::supplKeyRpcServerMediaType] = mediaType;
    auto labelStr = ib::cfg::Serialize<std::decay_t<decltype(labels)>>(labels);
    supplementalData[ib::mw::service::supplKeyRpcServerLabels] = labelStr;

    auto controller = CreateController<ib::cfg::RpcServer, sim::rpc::RpcServer>(
        controllerConfig, network, mw::ServiceType::Controller, supplementalData, true, _timeProvider.get(),
        controllerConfig.functionName.value(), mediaType, labels, handler);

    // RpcServer discovers RpcClient and creates RpcServerInternal on a matching connection
    controller->RegisterServiceDiscovery();

    return controller;
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateRpcServer(const std::string& canonicalName) -> sim::rpc::IRpcServer*
{
    return CreateRpcServer(canonicalName, canonicalName, { "" }, {}, nullptr);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::DiscoverRpcServers(const std::string& functionName, const std::string& mediaType,
                                                    const std::map<std::string, std::string>& labels,
                                                    sim::rpc::RpcDiscoveryResultHandler handler)
{
    sim::rpc::RpcDiscoverer rpcDiscoverer{GetServiceDiscovery()};
    handler(rpcDiscoverer.GetMatchingRpcServers(functionName, mediaType, labels));
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::CreateTimeSyncService(sync::LifecycleService* service) -> sync::TimeSyncService*
{
    auto* timeSyncService =
        GetController<sync::TimeSyncService>("default", ib::mw::service::controllerTypeTimeSyncService);

    if (timeSyncService)
    {
        throw std::runtime_error("Tried to instantiate TimeSyncService multiple times!");
    }

    mw::SupplementalData timeSyncSupplementalData;
    timeSyncSupplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeTimeSyncService;

    timeSyncService = CreateInternalController<sync::TimeSyncService>(
        ib::mw::service::controllerTypeTimeSyncService, mw::ServiceType::InternalController,
        std::move(timeSyncSupplementalData), false, service, _participantConfig.healthCheck);

    return timeSyncService;
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::GetLifecycleService() -> sync::ILifecycleService*
{
    auto* lifecycleService =
        GetController<sync::LifecycleService>("default", ib::mw::service::controllerTypeLifecycleService);

    if (!lifecycleService)
    {
        mw::SupplementalData lifecycleSupplementalData;
        lifecycleSupplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeLifecycleService;

        lifecycleService = CreateInternalController<sync::LifecycleService>(
            ib::mw::service::controllerTypeLifecycleService, mw::ServiceType::InternalController,
            std::move(lifecycleSupplementalData), false,
            _participantConfig.healthCheck);
    }
    return lifecycleService;
}


template <class IbConnectionT>
auto Participant<IbConnectionT>::GetSystemMonitor() -> sync::ISystemMonitor*
{
    auto* controller = GetController<sync::SystemMonitor>("default", "SystemMonitor");
    if (!controller)
    {
        mw::SupplementalData supplementalData;
        supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeSystemMonitor;

        controller = CreateInternalController<sync::SystemMonitor>("SystemMonitor", mw::ServiceType::InternalController,
                                                                   std::move(supplementalData), true);

        _ibConnection.RegisterMessageReceiver([controller](IVAsioPeer* peer, const ParticipantAnnouncement&) {
            controller->OnParticipantConnected(peer->GetInfo().participantName);
        });

        _ibConnection.RegisterPeerShutdownCallback([controller](IVAsioPeer* peer) {
            controller->OnParticipantDisconnected(peer->GetInfo().participantName);
        });
    }
    return controller;
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::GetServiceDiscovery() -> service::IServiceDiscovery*
{
    auto* controller = GetController<service::ServiceDiscovery>("default", "ServiceDiscovery");
    if (!controller)
    {
        mw::SupplementalData supplementalData;
        supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeServiceDiscovery;

        controller = CreateInternalController<service::ServiceDiscovery>(
            "ServiceDiscovery", mw::ServiceType::InternalController, std::move(supplementalData), true, _participantName);
        
        _ibConnection.RegisterPeerShutdownCallback([controller](IVAsioPeer* peer) {
            controller->OnParticpantRemoval(peer->GetInfo().participantName);
        });
    }
    return controller;
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::GetSystemController() -> sync::ISystemController*
{
    auto* controller = GetController<sync::SystemController>("default", "SystemController");
    if (!controller)
    {
        mw::SupplementalData supplementalData;
        supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeSystemController;

        return CreateInternalController<sync::SystemController>("SystemController", mw::ServiceType::InternalController,
                                                                std::move(supplementalData), true);
    }
    return controller;
}

template <class IbConnectionT>
auto Participant<IbConnectionT>::GetLogger() -> logging::ILogger*
{
    return _logger.get();
}

template <class IbConnectionT>
void Participant<IbConnectionT>::RegisterCanSimulator(can::IIbToCanSimulator* busSim,  const std::vector<std::string>& networkNames)
{
    RegisterSimulator(busSim, cfg::NetworkType::CAN, networkNames);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::RegisterEthSimulator(sim::eth::IIbToEthSimulator* busSim,  const std::vector<std::string>& networkNames)
{
    RegisterSimulator(busSim, cfg::NetworkType::Ethernet, networkNames);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::RegisterFlexraySimulator(sim::fr::IIbToFlexrayBusSimulator* busSim,  const std::vector<std::string>& networkNames)
{
    RegisterSimulator(busSim, cfg::NetworkType::FlexRay, networkNames);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::RegisterLinSimulator(sim::lin::IIbToLinSimulator* busSim,  const std::vector<std::string>& networkNames)
{
    RegisterSimulator(busSim, cfg::NetworkType::LIN, networkNames);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const can::CanFrameEvent& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, can::CanFrameEvent&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const can::CanFrameTransmitEvent& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const can::CanControllerStatus& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const can::CanConfigureBaudrate& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const can::CanSetControllerMode& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const eth::EthernetFrameEvent& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, eth::EthernetFrameEvent&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const eth::EthernetFrameTransmitEvent& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const eth::EthernetStatus& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const eth::EthernetSetMode& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexrayFrameEvent& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, sim::fr::FlexrayFrameEvent&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexrayFrameTransmitEvent& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, sim::fr::FlexrayFrameTransmitEvent&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexraySymbolEvent& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexraySymbolTransmitEvent& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexrayCycleStartEvent& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexrayHostCommand& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexrayControllerConfig& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexrayTxBufferConfigUpdate& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexrayTxBufferUpdate& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexrayPocStatusEvent& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::LinSendFrameRequest& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::LinSendFrameHeaderRequest& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::LinTransmission& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::LinWakeupPulse& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::LinControllerConfig& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::LinControllerStatusUpdate& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::LinFrameResponseUpdate& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::data::DataMessageEvent& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, sim::data::DataMessageEvent&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::rpc::FunctionCall& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, sim::rpc::FunctionCall&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::rpc::FunctionCallResponse& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, sim::rpc::FunctionCallResponse&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sync::NextSimTask& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sync::ParticipantStatus& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sync::ParticipantCommand& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sync::SystemCommand& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sync::WorkflowConfiguration& msg)
{
    SendIbMessageImpl(from, msg);
}


template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const logging::LogMsg& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, logging::LogMsg&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const service::ParticipantDiscoveryEvent& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const service::ServiceDiscoveryEvent& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
template <typename IbMessageT>
void Participant<IbConnectionT>::SendIbMessageImpl(const IIbServiceEndpoint* from, IbMessageT&& msg)
{
    TraceTx(_logger.get(), from, msg);
    _ibConnection.SendIbMessage(from, std::forward<IbMessageT>(msg));
}

// targeted messaging
template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const can::CanFrameEvent& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, can::CanFrameEvent&& msg)
{
    SendIbMessageImpl(from, targetParticipantName, std::move(msg));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const can::CanFrameTransmitEvent& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const can::CanControllerStatus& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const can::CanConfigureBaudrate& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const can::CanSetControllerMode& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const eth::EthernetFrameEvent& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, eth::EthernetFrameEvent&& msg)
{
    SendIbMessageImpl(from, targetParticipantName, std::move(msg));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const eth::EthernetFrameTransmitEvent& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const eth::EthernetStatus& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const eth::EthernetSetMode& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexrayFrameEvent& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::fr::FlexrayFrameEvent&& msg)
{
    SendIbMessageImpl(from, targetParticipantName, std::move(msg));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexrayFrameTransmitEvent& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::fr::FlexrayFrameTransmitEvent&& msg)
{
    SendIbMessageImpl(from, targetParticipantName, std::move(msg));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexraySymbolEvent& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexraySymbolTransmitEvent& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexrayCycleStartEvent& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexrayHostCommand& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexrayControllerConfig& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexrayTxBufferConfigUpdate& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexrayTxBufferUpdate& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexrayPocStatusEvent& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinSendFrameRequest& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinSendFrameHeaderRequest& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinTransmission& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinWakeupPulse& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinControllerConfig& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinControllerStatusUpdate& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinFrameResponseUpdate& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName,
                                              const sim::data::DataMessageEvent& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName,
                                              sim::data::DataMessageEvent&& msg)
{
    SendIbMessageImpl(from, targetParticipantName, std::move(msg));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName,
                                              const sim::rpc::FunctionCall& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName,
                                              sim::rpc::FunctionCall&& msg)
{
    SendIbMessageImpl(from, targetParticipantName, std::move(msg));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName,
                                              const sim::rpc::FunctionCallResponse& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName,
                                              sim::rpc::FunctionCallResponse&& msg)
{
    SendIbMessageImpl(from, targetParticipantName, std::move(msg));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName,
                                              const sync::NextSimTask& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sync::ParticipantStatus& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sync::ParticipantCommand& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sync::SystemCommand& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sync::WorkflowConfiguration& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const logging::LogMsg& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, logging::LogMsg&& msg)
{
    SendIbMessageImpl(from, targetParticipantName, std::move(msg));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const service::ParticipantDiscoveryEvent& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void Participant<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const service::ServiceDiscoveryEvent& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
template <typename IbMessageT>
void Participant<IbConnectionT>::SendIbMessageImpl(const IIbServiceEndpoint* from, const std::string& targetParticipantName, IbMessageT&& msg)
{
    TraceTx(_logger.get(), from, msg);
    _ibConnection.SendIbMessage(from, targetParticipantName, std::forward<IbMessageT>(msg));
}


template <class IbConnectionT>
template <class ControllerT>
auto Participant<IbConnectionT>::GetController(const std::string& networkName, const std::string& serviceName) -> ControllerT*
{
    auto&& controllerMap = tt::predicative_get<tt::rbind<IsControllerMap, ControllerT>::template type>(_controllers);
    const auto&& qualifiedName = networkName + "/" + serviceName;
    if (controllerMap.count(qualifiedName))
    {
        return static_cast<ControllerT*>(controllerMap.at(qualifiedName).get());
    }
    else
    {
        return nullptr;
    }
}

template <class IbConnectionT>
template <class ControllerT, typename... Arg>
auto Participant<IbConnectionT>::CreateInternalController(const std::string& serviceName,
                                                          const mw::ServiceType serviceType,
                                                          const mw::SupplementalData& supplementalData,
                                                          const bool publishService, Arg&&... arg) -> ControllerT*
{
    cfg::InternalController config;
    config.name = serviceName;
    config.network = "default";

    return CreateController<cfg::InternalController, ControllerT>(config, serviceType, supplementalData, publishService, 
                                                                  std::forward<Arg>(arg)...);
}

template <class IbConnectionT>
template <class ConfigT, class ControllerT, typename... Arg>
auto Participant<IbConnectionT>::CreateController(const ConfigT& config, const mw::ServiceType serviceType,
                                                  const mw::SupplementalData& supplementalData,
                                                  const bool publishService,
                                                  Arg&&... arg) -> ControllerT*
{
    assert(config.network.has_value());
    return CreateController<ConfigT, ControllerT>(config, *config.network, serviceType, supplementalData,
                                                  publishService, std::forward<Arg>(arg)...);
}

template <class IbConnectionT>
template <class ConfigT, class ControllerT, typename... Arg>
auto Participant<IbConnectionT>::CreateController(const ConfigT& config, const std::string& network,
                                                  const mw::ServiceType serviceType,
                                                  const mw::SupplementalData& supplementalData,
                                                  const bool publishService,
                                                  Arg&&... arg) -> ControllerT*
{
    if (config.name == "")
    {
        throw ib::ConfigurationError("Services must have a non-empty name.");
    }

    // If possible, load controller from cache
    auto* controllerPtr = GetController<ControllerT>(network, config.name);
    if (controllerPtr != nullptr)
    {
        // We cache the controller and return it here.
        return controllerPtr;
    }

    auto&& controllerMap = tt::predicative_get<tt::rbind<IsControllerMap, ControllerT>::template type>(_controllers);
    auto controller = std::make_unique<ControllerT>(this, std::forward<Arg>(arg)...);
    controllerPtr = controller.get();

    auto localEndpoint = _localEndpointId++;

    auto descriptor = ServiceDescriptor{};
    descriptor.SetNetworkName(network);
    descriptor.SetParticipantName(_participantName);
    descriptor.SetServiceName(config.name);
    descriptor.SetNetworkType(config.networkType);
    descriptor.SetServiceId(localEndpoint);
    descriptor.SetServiceType(serviceType);
    descriptor.SetSupplementalData(std::move(supplementalData));

    controller->SetServiceDescriptor(std::move(descriptor));

    _ibConnection.RegisterIbService(network, localEndpoint, controllerPtr);
    const auto qualifiedName = network + "/" + config.name;
    controllerMap[qualifiedName] = std::move(controller);

    // TODO uncomment once trace & replay work again
    //auto* traceSource = dynamic_cast<extensions::ITraceMessageSource*>(controllerPtr);
    //if (traceSource)
    //{
    //    AddTraceSinksToSource(traceSource, config);
    //}
    if (publishService)
    {
        GetServiceDiscovery()->NotifyServiceCreated(controllerPtr->GetServiceDescriptor());
    }
    return controllerPtr;
}

template <class IbConnectionT>
template <class ConfigT>
void Participant<IbConnectionT>::AddTraceSinksToSource(extensions::ITraceMessageSource* traceSource, ConfigT config)
{
    if (config.useTraceSinks.empty())
    {
        GetLogger()->Debug("Tracer on {}/{} not enabled, skipping", _participantName, config.name);
        return;
    }
    auto findSinkByName = [this](const auto& name)
    {
       return std::find_if(_traceSinks.begin(), _traceSinks.end(),
            [&name](const auto& sinkPtr) {
                return sinkPtr->Name() == name;
            });
    };

    for (const auto& sinkName : config.useTraceSinks)
    {
        auto sinkIter = findSinkByName(sinkName);
        if (sinkIter == _traceSinks.end())
        {
            std::stringstream ss;
            ss << "Controller " << config.name << " refers to non-existing sink "
                << sinkName;

            GetLogger()->Error(ss.str());
            throw ib::ConfigurationError(ss.str());
        }
        traceSource->AddSink((*sinkIter).get());
    }
}

template <class IbConnectionT>
template <class IIbToSimulatorT>
void Participant<IbConnectionT>::RegisterSimulator(IIbToSimulatorT* busSim, cfg::NetworkType linkType,
                                                  const std::vector<std::string>& simulatedNetworkNames)
{
    auto& serviceEndpoint = dynamic_cast<mw::IIbServiceEndpoint&>(*busSim);
    auto oldDescriptor = serviceEndpoint.GetServiceDescriptor();
    //XXX we temporarily overwrite the simulator's serviceEndpoint (not used internally)
    //    only for RegisterIbService: we should refactor RegisterIbService to accept the ServiceDescriptor directly
    for (const auto& network: simulatedNetworkNames)
    {
        auto id = ServiceDescriptor{};
        id.SetNetworkName(network);
        id.SetServiceName(network);
        id.SetNetworkType(linkType);
        id.SetParticipantName(GetParticipantName());

        serviceEndpoint.SetServiceDescriptor(id);
        // Tell the middle-ware we are interested in this named network of the given type
        EndpointId unused{}; //not used in VAsioConnection.hpp
        _ibConnection.RegisterIbService(network, unused, busSim);
    }
    serviceEndpoint.SetServiceDescriptor(oldDescriptor); //restore 
}

template <class IbConnectionT>
void Participant<IbConnectionT>::OnAllMessagesDelivered(std::function<void()> callback)
{
    _ibConnection.OnAllMessagesDelivered(std::move(callback));
}

template <class IbConnectionT>
void Participant<IbConnectionT>::FlushSendBuffers()
{
    _ibConnection.FlushSendBuffers();
}

template <class IbConnectionT>
void Participant<IbConnectionT>::ExecuteDeferred(std::function<void()> callback)
{
    _ibConnection.ExecuteDeferred(std::move(callback));
}

template <class IbConnectionT>
template <typename ValueT>
void Participant<IbConnectionT>::LogMismatchBetweenConfigAndPassedValue(const std::string& canonicalName,
                                                                        const ValueT& passedValue,
                                                                        const ValueT& configuredValue)
{
    std::stringstream ss;
    ss << "Mismatch between a configured and programmatically passed value. The configured value will be used."
       << std::endl
       << "Controller name: " << canonicalName << std::endl
       << "Passed value: " << passedValue << std::endl
       << "Configured value: " << configuredValue << std::endl;

    _logger->Info(ss.str());
}

} // namespace mw
} // namespace ib
