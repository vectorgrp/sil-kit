// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <cassert>
#include <sstream>
#include <chrono>

#include "CanController.hpp"
#include "CanControllerProxy.hpp"
#include "CanControllerFacade.hpp"
#include "EthController.hpp"
#include "EthControllerProxy.hpp"
#include "EthControllerFacade.hpp"
#include "EthControllerReplay.hpp"
#include "FrControllerProxy.hpp"
#include "FrControllerFacade.hpp"
#include "LinController.hpp"
#include "LinControllerReplay.hpp"
#include "LinControllerProxy.hpp"
#include "LinControllerFacade.hpp"
#include "GenericPublisher.hpp"
#include "GenericPublisherReplay.hpp"
#include "GenericSubscriber.hpp"
#include "GenericSubscriberReplay.hpp"
#include "DataPublisher.hpp"
#include "DataSubscriber.hpp"
#include "DataSubscriberInternal.hpp"
#include "RpcClient.hpp"
#include "RpcServer.hpp"
#include "RpcServerInternal.hpp"
#include "RpcDiscoverer.hpp"
#include "ParticipantController.hpp"
#include "SystemController.hpp"
#include "SystemMonitor.hpp"
#include "LogMsgSender.hpp"
#include "LogMsgReceiver.hpp"
#include "Logger.hpp"
#include "TimeProvider.hpp"
#include "ServiceDiscovery.hpp"
#include "ParticipantConfiguration.hpp"
#include "YamlConfig.hpp"

#include "tuple_tools/bind.hpp"
#include "tuple_tools/for_each.hpp"
#include "tuple_tools/predicative_get.hpp"

#include "ib/cfg/string_utils.hpp"
#include "ib/version.hpp"

#include "ComAdapter.hpp"

#include "MessageTracing.hpp" // log tracing
#include "UuidRandom.hpp"

#ifdef SendMessage
#if SendMessage == SendMessageA
#undef SendMessage
#endif
#endif //SendMessage


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


// Helper to find all the NetworkSimulator configuration blocks,
// which now reside in the participant configuration.
auto FindNetworkSimulators(const cfg::SimulationSetup& simulationSetup) 
    -> std::vector<cfg::NetworkSimulator>
{
    std::vector<cfg::NetworkSimulator> result;
    for (const auto& participant : simulationSetup.participants)
    {
        std::copy(participant.networkSimulators.begin(), participant.networkSimulators.end(),
            std::back_inserter(result));
    }
    return result;
}

template<typename ConfigT>
bool ControllerUsesReplay(const ConfigT& controllerConfig)
{
    return controllerConfig.replay.direction != cfg::Replay::Direction::Undefined
        && !controllerConfig.replay.useTraceSource.empty();
}

} // namespace anonymous

template <class IbConnectionT>
ComAdapter<IbConnectionT>::ComAdapter(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                                      const std::string& participantName, bool isSynchronized)
    : _participantName{ participantName }
    , _isSynchronized{ isSynchronized }
    , _participantId{ hash(participantName) }
    , _participantConfig{ std::dynamic_pointer_cast<ib::cfg::v1::datatypes::ParticipantConfiguration>(participantConfig) }
    , _ibConnection{ _participantConfig, participantName, _participantId }
{
    // NB: do not create the _logger in the initializer list. If participantName is empty,
    //  this will cause a fairly unintuitive exception in spdlog.
    _logger = std::make_unique<logging::Logger>(_participantName, _participantConfig->logging);
    _ibConnection.SetLogger(_logger.get());
    
    _logger->Info("Creating ComAdapter for Participant {}, IntegrationBus-Version: {} {}, Middleware: {}",
                  _participantName, version::String(), version::SprintName(),
                  "VAsio");

    //set up default time provider used for controller instantiation
    // TODO decide upon timePeriod
    _timeProvider = std::make_shared<sync::WallclockProvider>(1ms);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::joinIbDomain(uint32_t domainId)
{
    _ibConnection.JoinDomain(domainId);
    onIbDomainJoined();

    _logger->Info("Participant {} has joined the IB-Domain {}", _participantName, domainId);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::onIbDomainJoined()
{
    SetupRemoteLogging();

    //Ensure Service discovery is started
    (void)GetServiceDiscovery();

    // Create the participants trace message sinks as declared in the configuration.
    //_traceSinks = tracing::CreateTraceMessageSinks(GetLogger(), _config, _participant);

    // NB: Create the participantController directly for synchronized participants.
    if (_isSynchronized)
    {
        auto* participantController =
            static_cast<sync::ParticipantController*>(GetParticipantController());
        _timeProvider = participantController->GetTimeProvider();
    }
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
    monitor->RegisterSystemStateHandler([&conn = GetIbConnection()](auto newState)
    {
        if (newState == sync::SystemState::ShuttingDown)
        {
            conn.NotifyShutdown();
        }
    });
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SetupRemoteLogging()
{
    auto* logger = dynamic_cast<logging::Logger*>(_logger.get());
    if (logger)
    {
        if (_participantConfig->logging.logFromRemotes)
        {
            mw::SupplementalData supplementalData;
            supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeLoggerReceiver;

            CreateInternalController<logging::LogMsgReceiver>("LogMsgReceiver", mw::ServiceType::InternalController,
                                                      std::move(supplementalData), logger);
        }

        auto sinkIter = std::find_if(_participantConfig->logging.sinks.begin(), _participantConfig->logging.sinks.end(),
            [](const cfg::v1::datatypes::Sink& sink) { return sink.type == cfg::v1::datatypes::Sink::Type::Remote; });

        if (sinkIter != _participantConfig->logging.sinks.end())
        {
            mw::SupplementalData supplementalData;
            supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeLoggerSender;

            auto&& logMsgSender = CreateInternalController<logging::LogMsgSender>(
                "LogMsgSender", mw::ServiceType::InternalController, std::move(supplementalData));

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
inline void ComAdapter<IbConnectionT>::SetTimeProvider(sync::ITimeProvider* newClock)
{
    // register the time provider with all already instantiated controllers
    auto setTimeProvider = [this, newClock](auto& controllers) {
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
auto ComAdapter<IbConnectionT>::CreateCanController(const std::string& canonicalName, const std::string& networkName) -> can::ICanController*
{
    // retrieve CAN controller
    auto& canControllers = _participantConfig->canControllers;
    auto controllerIter =
        std::find_if(canControllers.begin(), canControllers.end(), [&canonicalName, &networkName](auto&& controller) {
            return controller.name == canonicalName && controller.network == networkName;
        });
    ib::cfg::v1::datatypes::CanController controllerConfig;
    if (controllerIter != canControllers.end())
    {
        controllerConfig = *controllerIter;
        if (controllerConfig.network != networkName)
        {
            PrintWrongNetworkNameForControllerWarning(canonicalName, networkName, controllerConfig.network,
                                                      ib::cfg::v1::datatypes::NetworkType::CAN);
        }
    }
    else
    {
        controllerConfig.name = canonicalName;
        controllerConfig.network = networkName;
    }
    
    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeCan;

    return CreateController<ib::cfg::v1::datatypes::CanController, can::CanControllerFacade>(
        controllerConfig, mw::ServiceType::Controller, std::move(supplementalData), controllerConfig,
        _timeProvider.get());
}


template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateCanController(const std::string& canonicalName)
    -> can::ICanController*
{
    return CreateCanController(canonicalName, canonicalName);
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateEthController(const std::string& canonicalName, const std::string& networkName)
    -> eth::IEthController*
{
    // retrieve Ethernet controller
    auto& ethernetControllerConfigs = _participantConfig->ethernetControllers;
    auto controllerConfigIter = std::find_if(ethernetControllerConfigs.begin(), ethernetControllerConfigs.end(),
                                             [&canonicalName, &networkName](auto&& controllerConfig) {
                                                 return controllerConfig.name == canonicalName;
                                             });
    ib::cfg::v1::datatypes::EthernetController controllerConfig;
    if (controllerConfigIter != ethernetControllerConfigs.end())
    {
        controllerConfig = *controllerConfigIter;
        if (controllerConfig.network != networkName)
        {
            PrintWrongNetworkNameForControllerWarning(canonicalName, networkName, controllerConfig.network,
                                                      ib::cfg::v1::datatypes::NetworkType::Ethernet);
        }
    }
    else
    {
        controllerConfig.name = canonicalName;
        controllerConfig.network = networkName;
    }

    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeEthernet;

    return CreateController<ib::cfg::v1::datatypes::EthernetController, eth::EthControllerFacade>(
        controllerConfig, mw::ServiceType::Controller, std::move(supplementalData), controllerConfig,
        _timeProvider.get());
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateEthController(const std::string& canonicalName) -> eth::IEthController*
{
    return CreateEthController(canonicalName, canonicalName);
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateFlexrayController(const std::string& canonicalName, const std::string& networkName)
    -> sim::fr::IFrController*
{
    // retrieve FR controller
    auto& flexRayControllerConfigs = _participantConfig->flexRayControllers;
    auto controllerConfigIter =
        std::find_if(flexRayControllerConfigs.begin(), flexRayControllerConfigs.end(), [&canonicalName, &networkName](auto&& controllerConfig) {
            return controllerConfig.name == canonicalName;
        });
    ib::cfg::v1::datatypes::FlexRayController controllerConfig;
    if (controllerConfigIter != flexRayControllerConfigs.end())
    {
        controllerConfig = *controllerConfigIter;
        if (controllerConfig.network != networkName)
        {
            PrintWrongNetworkNameForControllerWarning(canonicalName, networkName, controllerConfig.network,
                                                      ib::cfg::v1::datatypes::NetworkType::FlexRay);
        }
    }
    else
    {
        controllerConfig.name = canonicalName;
        controllerConfig.network = networkName;
    }

    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeFlexRay;

    return CreateController<ib::cfg::v1::datatypes::FlexRayController, fr::FrControllerFacade>(
        controllerConfig, mw::ServiceType::Controller, std::move(supplementalData), controllerConfig,
        _timeProvider.get());
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateFlexrayController(const std::string& canonicalName) -> sim::fr::IFrController*
{
    return CreateFlexrayController(canonicalName, canonicalName);
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateLinController(const std::string& canonicalName, const std::string& networkName)
    -> lin::ILinController*
{
    // retrieve LIN controller
    auto& linControllerConfigs = _participantConfig->linControllers;
    auto controllerConfigIter = std::find_if(linControllerConfigs.begin(), linControllerConfigs.end(),
                                             [&canonicalName, &networkName](auto&& controllerConfig) {
                                                 return controllerConfig.name == canonicalName;
                                             });
    ib::cfg::v1::datatypes::LinController controllerConfig;
    if (controllerConfigIter != linControllerConfigs.end())
    {
        controllerConfig = *controllerConfigIter;
        if (controllerConfig.network != networkName)
        {
            PrintWrongNetworkNameForControllerWarning(canonicalName, networkName, controllerConfig.network,
                                                      ib::cfg::v1::datatypes::NetworkType::LIN);
        }
    }
    else
    {
        controllerConfig.name = canonicalName;
        controllerConfig.network = networkName;
    }

    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeLin;

    return CreateController<ib::cfg::v1::datatypes::LinController, lin::LinControllerFacade>(
        controllerConfig, mw::ServiceType::Controller, std::move(supplementalData), controllerConfig,
        _timeProvider.get());
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateLinController(const std::string& canonicalName) -> lin::ILinController*
{
    return CreateLinController(canonicalName, canonicalName);
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateDataSubscriberInternal(const std::string& topic, const std::string& linkName,
                                                             const sim::data::DataExchangeFormat& dataExchangeFormat,
                                                             const std::map<std::string, std::string>& publisherLabels,
                                                             sim::data::DataHandlerT defaultHandler,
                                                             sim::data::IDataSubscriber* parent)
    -> sim::data::DataSubscriberInternal*
{
    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeDataSubscriberInternal;

    ib::cfg::v1::datatypes::DataSubscriber controllerConfig;
    controllerConfig.name = topic;
    controllerConfig.network = linkName;

    return CreateController<ib::cfg::v1::datatypes::DataSubscriber, sim::data::DataSubscriberInternal>(
        controllerConfig, mw::ServiceType::Controller, std::move(supplementalData), _timeProvider.get(), topic,
        dataExchangeFormat, publisherLabels, defaultHandler, parent);
}


template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateDataPublisher(const std::string& topic,
    const ib::sim::data::DataExchangeFormat& dataExchangeFormat, const std::map<std::string, std::string>& labels,
    size_t history) -> sim::data::IDataPublisher*
{
    if (history > 1)
    {
        throw cfg::Misconfiguration("DataPublishers do not support history > 1.");
    }

    std::string pubUUID = util::uuid::to_string(util::uuid::generate());

    // Retrieve controller
    auto& cfgs = _participantConfig->dataPublishers;
    auto it = std::find_if(cfgs.begin(), cfgs.end(),
                                             [topic](auto&& controllerConfig) {
                                                 return controllerConfig.name == topic;
                                             });
    ib::cfg::v1::datatypes::DataPublisher controllerConfig;
    if (it != cfgs.end())
    {
        controllerConfig = *it;
        controllerConfig.network = pubUUID;
    }
    else
    {
        controllerConfig.name = topic;
        controllerConfig.network = pubUUID;
    }

    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeDataPublisher;
    supplementalData[ib::mw::service::supplKeyDataPublisherTopic] = topic;
    supplementalData[ib::mw::service::supplKeyDataPublisherPubUUID] = pubUUID;
    supplementalData[ib::mw::service::supplKeyDataPublisherPubDxf] = dataExchangeFormat.mediaType;
    auto labelStr = ib::cfg::Serialize<std::decay_t<decltype(labels)>>(labels);
    supplementalData[ib::mw::service::supplKeyDataPublisherPubLabels] = labelStr;

    auto controller = CreateController<ib::cfg::v1::datatypes::DataPublisher, ib::sim::data::DataPublisher>(
        controllerConfig, mw::ServiceType::Controller, std::move(supplementalData), _timeProvider.get(), topic,
        dataExchangeFormat, labels, pubUUID, history);

    _ibConnection.SetHistoryLengthForLink(pubUUID, history, controller);

    return controller;
    
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateDataSubscriber(const std::string& topic,
                                                     const ib::sim::data::DataExchangeFormat& dataExchangeFormat,
                                                     const std::map<std::string, std::string>& labels,
                                                     ib::sim::data::DataHandlerT defaultDataHandler,
                                                     ib::sim::data::NewDataSourceHandlerT newDataSourceHandler)
    -> sim::data::IDataSubscriber*
{
    // Retrieve controller
    auto& cfgs = _participantConfig->dataSubscribers;
    auto it = std::find_if(cfgs.begin(), cfgs.end(), [topic](auto&& controllerConfig) {
        return controllerConfig.name == topic;
    });
    ib::cfg::v1::datatypes::DataSubscriber controllerConfig;
    if (it != cfgs.end())
    {
        controllerConfig = *it;
        if (controllerConfig.network != topic)
        {
            PrintWrongNetworkNameForControllerWarning(topic, topic, controllerConfig.network,
                                                      ib::cfg::v1::datatypes::NetworkType::Data);
        }
    }
    else
    {
        controllerConfig.name = topic;
        controllerConfig.network = topic;
    }

    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeDataSubscriber;

    auto controller = CreateController<ib::cfg::v1::datatypes::DataSubscriber, sim::data::DataSubscriber>(
        controllerConfig, mw::ServiceType::Controller, std::move(supplementalData), _timeProvider.get(), topic,
        dataExchangeFormat, labels, defaultDataHandler, newDataSourceHandler);

    controller->RegisterServiceDiscovery();

    return controller;
    
}


template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateGenericPublisher(const std::string& canonicalName) -> sim::generic::IGenericPublisher*
{
    _logger->Error("GenericPublisher is deprecated");
    return nullptr;
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateGenericSubscriber(const std::string& canonicalName)->sim::generic::IGenericSubscriber*
{
    _logger->Error("GenericSubscriber is deprecated");
    return nullptr;
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateRpcServerInternal(const std::string& functionName, const std::string& clientUUID,
                                                        const sim::rpc::RpcExchangeFormat exchangeFormat,
                                                        const std::map<std::string, std::string>& clientLabels,
                                                        sim::rpc::CallProcessor handler, sim::rpc::IRpcServer* parent)
    -> sim::rpc::RpcServerInternal*
{
    _logger->Trace("Creating internal server for functionName={}, clientUUID={}", functionName, clientUUID);

    ib::cfg::v1::datatypes::RpcServer controllerConfig;
    controllerConfig.name = functionName;
    controllerConfig.network = clientUUID;

    // RpcServerInternal gets discovered by RpcClient which is then ready to detach calls
    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeRpcServerInternal;
    supplementalData[ib::mw::service::supplKeyRpcServerInternalClientUUID] = clientUUID;

    return CreateController<ib::cfg::v1::datatypes::RpcServer, sim::rpc::RpcServerInternal>(
        controllerConfig, mw::ServiceType::Controller, std::move(supplementalData), _timeProvider.get(), functionName,
        exchangeFormat, clientLabels, clientUUID, handler, parent);
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateRpcClient(const std::string& functionName,
                                                const sim::rpc::RpcExchangeFormat exchangeFormat,
                                                const std::map<std::string, std::string>& labels,
                                                sim::rpc::CallReturnHandler handler) -> sim::rpc::IRpcClient*
{
    auto clientUUID = util::uuid::to_string(util::uuid::generate());

    // Retrieve controller
    auto& cfgs = _participantConfig->rpcClients;
    auto it = std::find_if(cfgs.begin(), cfgs.end(), [functionName](auto&& controllerConfig) {
        return controllerConfig.name == functionName;
    });
    ib::cfg::v1::datatypes::RpcClient controllerConfig;
    if (it != cfgs.end())
    {
        controllerConfig = *it;
        controllerConfig.network = clientUUID;
    }
    else
    {
        controllerConfig.name = functionName;
        controllerConfig.network = clientUUID;
    }

    // RpcClient gets discovered by RpcServer which creates RpcServerInternal on a matching connection
    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeRpcClient;
    supplementalData[ib::mw::service::supplKeyRpcClientFunctionName] = functionName;
    supplementalData[ib::mw::service::supplKeyRpcClientDxf] = exchangeFormat.mediaType;
    auto labelStr = ib::cfg::Serialize<std::decay_t<decltype(labels)>>(labels);
    supplementalData[ib::mw::service::supplKeyRpcClientLabels] = labelStr;
    supplementalData[ib::mw::service::supplKeyRpcClientUUID] = clientUUID;

    auto controller = CreateController<ib::cfg::v1::datatypes::RpcClient, ib::sim::rpc::RpcClient>(
        controllerConfig, mw::ServiceType::Controller, std::move(supplementalData), _timeProvider.get(), functionName,
        exchangeFormat, labels, clientUUID, handler);

    // RpcClient discovers RpcServerInternal and is ready to dispatch calls
    controller->RegisterServiceDiscovery();
 
    return controller;
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::CreateRpcServer(const std::string& functionName,
                                                const sim::rpc::RpcExchangeFormat exchangeFormat,
                                                const std::map<std::string, std::string>& labels,
                                                sim::rpc::CallProcessor handler) -> sim::rpc::IRpcServer*
{
    // Retrieve controller
    auto& cfgs = _participantConfig->rpcServers;
    auto it = std::find_if(cfgs.begin(), cfgs.end(), [functionName](auto&& controllerConfig) {
        return controllerConfig.name == functionName;
    });
    ib::cfg::v1::datatypes::RpcServer controllerConfig;
    if (it != cfgs.end())
    {
        controllerConfig = *it;
        if (controllerConfig.network != functionName)
        {
            PrintWrongNetworkNameForControllerWarning(functionName, functionName, controllerConfig.network,
                                                      ib::cfg::v1::datatypes::NetworkType::RPC);
        }
    }
    else
    {
        controllerConfig.name = functionName;
        controllerConfig.network = functionName;
    }
	
    // RpcServer announces himself to be found by DiscoverRpcServers()
    mw::SupplementalData supplementalData;
    supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeRpcServer;
    supplementalData[ib::mw::service::supplKeyRpcServerFunctionName] = functionName;
    supplementalData[ib::mw::service::supplKeyRpcServerDxf] = exchangeFormat.mediaType;
    auto labelStr = ib::cfg::Serialize<std::decay_t<decltype(labels)>>(labels);
    supplementalData[ib::mw::service::supplKeyRpcServerLabels] = labelStr;

    auto controller = CreateController<ib::cfg::v1::datatypes::RpcServer, sim::rpc::RpcServer>(
        controllerConfig, mw::ServiceType::Controller, supplementalData, _timeProvider.get(), functionName,
        exchangeFormat, labels, handler);

    // RpcServer discovers RpcClient and creates RpcServerInternal on a matching connection
    controller->RegisterServiceDiscovery();

    return controller;
    
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::DiscoverRpcServers(const std::string& functionName,
    const sim::rpc::RpcExchangeFormat& exchangeFormat,
    const std::map<std::string, std::string>& labels,
    sim::rpc::DiscoveryResultHandler handler)
{
    sim::rpc::RpcDiscoverer rpcDiscoverer{ GetServiceDiscovery() };
    handler(rpcDiscoverer.GetMatchingRpcServers(functionName, exchangeFormat, labels));
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::GetParticipantController() -> sync::IParticipantController*
{
    auto* controller = GetController<sync::ParticipantController>("default", "ParticipantController");
    if (!controller)
    {
        mw::SupplementalData supplementalData;
        supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeParticipantController;
        controller = CreateInternalController<sync::ParticipantController>(
            "ParticipantController", mw::ServiceType::InternalController, std::move(supplementalData), _participantName,
            _isSynchronized, _participantConfig->healthCheck);
    }

    return controller;
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::GetSystemMonitor() -> sync::ISystemMonitor*
{
    auto* controller = GetController<sync::SystemMonitor>("default", "SystemMonitor");
    if (!controller)
    {
        mw::SupplementalData supplementalData;
        supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeSystemMonitor;

        controller = CreateInternalController<sync::SystemMonitor>("SystemMonitor", mw::ServiceType::InternalController,
                                                           std::move(supplementalData));
    }
    return controller;
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::GetServiceDiscovery() -> service::IServiceDiscovery*
{
    auto* controller = GetController<service::ServiceDiscovery>("default", "ServiceDiscovery");
    if (!controller)
    {
        mw::SupplementalData supplementalData;
        supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeServiceDiscovery;

        controller = CreateInternalController<service::ServiceDiscovery>(
            "ServiceDiscovery", mw::ServiceType::InternalController, std::move(supplementalData),
                                                                 _participantName);
    }
    return controller;
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::GetSystemController() -> sync::ISystemController*
{
    auto* controller = GetController<sync::SystemController>("default", "SystemController");
    if (!controller)
    {
        mw::SupplementalData supplementalData;
        supplementalData[ib::mw::service::controllerType] = ib::mw::service::controllerTypeSystemController;

        return CreateInternalController<sync::SystemController>("SystemController", mw::ServiceType::InternalController,
                                                        std::move(supplementalData));
    }
    return controller;
}

template <class IbConnectionT>
auto ComAdapter<IbConnectionT>::GetLogger() -> logging::ILogger*
{
    return _logger.get();
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::RegisterCanSimulator(can::IIbToCanSimulator* busSim,  const std::vector<std::string>& networkNames)
{
    RegisterSimulator(busSim, cfg::Link::Type::CAN, networkNames);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::RegisterEthSimulator(sim::eth::IIbToEthSimulator* busSim,  const std::vector<std::string>& networkNames)
{
    RegisterSimulator(busSim, cfg::Link::Type::Ethernet, networkNames);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::RegisterFlexraySimulator(sim::fr::IIbToFrBusSimulator* busSim,  const std::vector<std::string>& networkNames)
{
    RegisterSimulator(busSim, cfg::Link::Type::FlexRay, networkNames);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::RegisterLinSimulator(sim::lin::IIbToLinSimulator* busSim,  const std::vector<std::string>& networkNames)
{
    RegisterSimulator(busSim, cfg::Link::Type::LIN, networkNames);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const can::CanMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, can::CanMessage&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const can::CanTransmitAcknowledge& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const can::CanControllerStatus& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const can::CanConfigureBaudrate& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const can::CanSetControllerMode& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const eth::EthMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, eth::EthMessage&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const eth::EthTransmitAcknowledge& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const eth::EthStatus& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const eth::EthSetMode& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FrMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, sim::fr::FrMessage&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FrMessageAck& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, sim::fr::FrMessageAck&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FrSymbol& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FrSymbolAck& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::CycleStart& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::HostCommand& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::ControllerConfig& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::TxBufferConfigUpdate& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::TxBufferUpdate& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::PocStatus& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::SendFrameRequest& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::SendFrameHeaderRequest& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::Transmission& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::WakeupPulse& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::ControllerConfig& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::ControllerStatusUpdate& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::FrameResponseUpdate& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::generic::GenericMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, sim::generic::GenericMessage&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::data::DataMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, sim::data::DataMessage&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::rpc::FunctionCall& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, sim::rpc::FunctionCall&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sim::rpc::FunctionCallResponse& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, sim::rpc::FunctionCallResponse&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sync::NextSimTask& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sync::ParticipantStatus& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sync::ParticipantCommand& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sync::SystemCommand& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const sync::ExpectedParticipants& msg)
{
    SendIbMessageImpl(from, msg);
}


template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const logging::LogMsg& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, logging::LogMsg&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const service::ServiceAnnouncement& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const service::ServiceDiscoveryEvent& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
template <typename IbMessageT>
void ComAdapter<IbConnectionT>::SendIbMessageImpl(const IIbServiceEndpoint* from, IbMessageT&& msg)
{
    TraceTx(_logger.get(), from, msg);
    _ibConnection.SendIbMessage(from, std::forward<IbMessageT>(msg));
}

// targeted messaging
template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const can::CanMessage& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, can::CanMessage&& msg)
{
    SendIbMessageImpl(from, targetParticipantName, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const can::CanTransmitAcknowledge& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const can::CanControllerStatus& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const can::CanConfigureBaudrate& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const can::CanSetControllerMode& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const eth::EthMessage& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, eth::EthMessage&& msg)
{
    SendIbMessageImpl(from, targetParticipantName, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const eth::EthTransmitAcknowledge& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const eth::EthStatus& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const eth::EthSetMode& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FrMessage& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::fr::FrMessage&& msg)
{
    SendIbMessageImpl(from, targetParticipantName, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FrMessageAck& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::fr::FrMessageAck&& msg)
{
    SendIbMessageImpl(from, targetParticipantName, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FrSymbol& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FrSymbolAck& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::CycleStart& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::HostCommand& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::ControllerConfig& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::TxBufferConfigUpdate& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::TxBufferUpdate& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::PocStatus& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::SendFrameRequest& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::SendFrameHeaderRequest& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::Transmission& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::WakeupPulse& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::ControllerConfig& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::ControllerStatusUpdate& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::FrameResponseUpdate& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::generic::GenericMessage& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::generic::GenericMessage&& msg)
{
    SendIbMessageImpl(from, targetParticipantName, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName,
                                              const sim::data::DataMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName,
                                              sim::data::DataMessage&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName,
                                              const sim::rpc::FunctionCall& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName,
                                              sim::rpc::FunctionCall&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName,
                                              const sim::rpc::FunctionCallResponse& msg)
{
    SendIbMessageImpl(from, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName,
                                              sim::rpc::FunctionCallResponse&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName,
                                              const sync::NextSimTask& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sync::ParticipantStatus& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sync::ParticipantCommand& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sync::SystemCommand& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sync::ExpectedParticipants& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const logging::LogMsg& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, logging::LogMsg&& msg)
{
    SendIbMessageImpl(from, targetParticipantName, std::move(msg));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const service::ServiceAnnouncement& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const service::ServiceDiscoveryEvent& msg)
{
    SendIbMessageImpl(from, targetParticipantName, msg);
}

template <class IbConnectionT>
template <typename IbMessageT>
void ComAdapter<IbConnectionT>::SendIbMessageImpl(const IIbServiceEndpoint* from, const std::string& targetParticipantName, IbMessageT&& msg)
{
    TraceTx(_logger.get(), from, msg);
    _ibConnection.SendIbMessage(from, targetParticipantName, std::forward<IbMessageT>(msg));
}


template <class IbConnectionT>
template <class ControllerT>
auto ComAdapter<IbConnectionT>::GetController(const std::string& networkName, const std::string& serviceName) -> ControllerT*
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
template<class ControllerT, typename... Arg>
auto ComAdapter<IbConnectionT>::CreateInternalController(const std::string& serviceName, const mw::ServiceType serviceType,
                                                 const mw::SupplementalData& supplementalData,
                                                 Arg&&... arg) -> ControllerT*
{
    cfg::v1::datatypes::InternalController config;
    config.name = serviceName;
    config.network = "default";

    return CreateController<cfg::v1::datatypes::InternalController, ControllerT>(
        config, serviceType, supplementalData, std::forward<Arg>(arg)...);
}

template <class IbConnectionT>
template <class ConfigT, class ControllerT, typename... Arg>
auto ComAdapter<IbConnectionT>::CreateController(const ConfigT& config,
                                                    const mw::ServiceType serviceType,
                                                    const mw::SupplementalData& supplementalData, Arg&&... arg)
    -> ControllerT*
{
    if (config.name == "")
    {
        throw ib::cfg::Misconfiguration("Services must have a non-empty name.");
    }

    // If possible, load controller from cache
    auto* controllerPtr = GetController<ControllerT>(config.network, config.name);
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
    descriptor.SetNetworkName(config.network);
    descriptor.SetParticipantName(_participantName);
    descriptor.SetServiceName(config.name);
    descriptor.SetNetworkType(static_cast<cfg::Link::Type>(config.networkType));
    descriptor.SetServiceId(localEndpoint);
    descriptor.SetServiceType(serviceType);
    descriptor.SetSupplementalData(std::move(supplementalData));

    controller->SetServiceDescriptor(std::move(descriptor));

    _ibConnection.RegisterIbService(config.network, localEndpoint, controllerPtr);
    const auto qualifiedName = config.network + "/" + config.name;
    controllerMap[qualifiedName] = std::move(controller);

    // TODO uncomment once trace & replay work again
    //auto* traceSource = dynamic_cast<extensions::ITraceMessageSource*>(controllerPtr);
    //if (traceSource)
    //{
    //    AddTraceSinksToSource(traceSource, config);
    //}

    GetServiceDiscovery()->NotifyServiceCreated(controllerPtr->GetServiceDescriptor());
    return controllerPtr;
}

template <class IbConnectionT>
template <class ConfigT>
void ComAdapter<IbConnectionT>::AddTraceSinksToSource(extensions::ITraceMessageSource* traceSource, ConfigT config)
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
            throw cfg::Misconfiguration(ss.str());
        }
        traceSource->AddSink((*sinkIter).get());
    }
}

template <class IbConnectionT>
template <class IIbToSimulatorT>
void ComAdapter<IbConnectionT>::RegisterSimulator(IIbToSimulatorT* busSim,
        cfg::Link::Type linkType,
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
bool ComAdapter<IbConnectionT>::ControllerUsesNetworkSimulator(const std::string& controllerName) const
{
    /*
    auto endpointName = _participantName + "/" + controllerName;
    const auto networkSimulators = FindNetworkSimulators(_config.simulationSetup);
  
    if (networkSimulators.empty())
    {
        // no participant with a network simulators present in config
        return false;
    }

    for (auto&& link : _config.simulationSetup.links)
    {
        auto endpointIter = std::find(link.endpoints.begin(), link.endpoints.end(), endpointName);

        if (endpointIter == link.endpoints.end())
            continue;

        //check if the link is a network simulator's simulated link
        for (const auto& simulator : networkSimulators)
        {
            auto linkIter = std::find(simulator.simulatedLinks.begin(), simulator.simulatedLinks.end(), link.name);
            if (linkIter != simulator.simulatedLinks.end())
                return true;
        }
    }
    */
    return false;
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::OnAllMessagesDelivered(std::function<void()> callback)
{
    _ibConnection.OnAllMessagesDelivered(std::move(callback));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::FlushSendBuffers()
{
    _ibConnection.FlushSendBuffers();
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::ExecuteDeferred(std::function<void()> callback)
{
    _ibConnection.ExecuteDeferred(std::move(callback));
}

template <class IbConnectionT>
void ComAdapter<IbConnectionT>::PrintWrongNetworkNameForControllerWarning(const std::string& canonicalName,
                                                                          const std::string& providedNetworkName,
                                                                          const std::string& configuredNetworkName,
                                                                          cfg::v1::datatypes::NetworkType networkType)
{
    std::stringstream ss;
    ss << "The provided configuration contained a " << to_string(networkType) << " controller with the provided name, "
          "but a different network name.The preconfigured network name will be used." << std::endl
       << "Controller name: " << canonicalName << std::endl
       << "Provided network name: " << providedNetworkName << std::endl
       << "Configured network name: " << configuredNetworkName << std::endl;

    _logger->Warn(ss.str());
}

} // namespace mw
} // namespace ib
