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

#include <sstream>
#include <chrono>
#include <string>

#include "CanController.hpp"
#include "EthController.hpp"
#include "FlexrayController.hpp"
#include "LinController.hpp"
#include "DataPublisher.hpp"
#include "DataSubscriber.hpp"
#include "DataSubscriberInternal.hpp"
#include "RpcClient.hpp"
#include "RpcServer.hpp"
#include "RpcServerInternal.hpp"
#include "NetworkSimulatorDatatypesInternal.hpp"
#include "silkit/experimental/netsim/string_utils.hpp"

#include "LifecycleService.hpp"
#include "SystemController.hpp"
#include "SystemMonitor.hpp"
#include "LogMsgSender.hpp"
#include "LogMsgReceiver.hpp"
#include "Logger.hpp"
#include "TimeProvider.hpp"
#include "TimeSyncService.hpp"
#include "ServiceDiscovery.hpp"
#include "RequestReplyService.hpp"
#include "ParticipantConfiguration.hpp"
#include "YamlParser.hpp"
#include "NetworkSimulatorInternal.hpp"

#include "MetricsManager.hpp"
#include "MetricsSender.hpp"
#include "MetricsTimerThread.hpp"
#include "CreateMetricsSinksFromParticipantConfiguration.hpp"

#include "tuple_tools/bind.hpp"
#include "tuple_tools/for_each.hpp"
#include "tuple_tools/predicative_get.hpp"

#include "SilKitVersionImpl.hpp"

#include "Participant.hpp"

#include "Tracing.hpp"
#include "MessageTracing.hpp"
#include "Uuid.hpp"
#include "Assert.hpp"
#include "ExecutionEnvironment.hpp"

#include "ILoggerInternal.hpp"


namespace SilKit {
namespace Core {

using namespace SilKit::Services;
using namespace std::chrono_literals;

namespace tt = Util::tuple_tools;

// Anonymous namespace for Helper Traits and Functions
namespace {

template <class T, class U>
struct IsControllerMap : std::false_type
{
};
template <class T, class U>
struct IsControllerMap<std::unordered_map<std::string, std::unique_ptr<T>>, U> : std::is_base_of<T, U>
{
};

} // namespace

template <class SilKitConnectionT>
Participant<SilKitConnectionT>::Participant(Config::ParticipantConfiguration participantConfig, ProtocolVersion version)
    : _participantConfig{participantConfig}
    , _participantId{Util::Hash::Hash(participantConfig.participantName)}
    , _metricsProcessor{std::make_unique<VSilKit::MetricsProcessor>(GetParticipantName())}
    , _metricsManager{std::make_unique<VSilKit::MetricsManager>(GetParticipantName(), *_metricsProcessor)}
    , _connection{this,
                  _metricsManager.get(),
                  _participantConfig,
                  participantConfig.participantName,
                  _participantId,
                  &_timeProvider,
                  version}
    , _metricsTimerThread{MakeTimerThread()}
{
    // NB: do not create the _logger in the initializer list. If participantName is empty,
    //  this will cause a fairly unintuitive exception in spdlog.
    _logger = std::make_unique<Services::Logging::Logger>(GetParticipantName(), _participantConfig.logging);

    dynamic_cast<VSilKit::MetricsProcessor&>(*_metricsProcessor).SetLogger(*_logger);
    dynamic_cast<VSilKit::MetricsManager&>(*_metricsManager).SetLogger(*_logger);
    _connection.SetLogger(_logger.get());

    Logging::Info(_logger.get(), "Creating participant '{}' at '{}', SIL Kit version: {}", GetParticipantName(),
                  _participantConfig.middleware.registryUri, Version::StringImpl());
}


template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::JoinSilKitSimulation()
{
    _connection.JoinSimulation(GetRegistryUri());
    OnSilKitSimulationJoined();
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::OnSilKitSimulationJoined()
{
    SetupRemoteLogging();
    SetupMetrics();

    // NB: Create the systemController to abort the simulation already in the startup phase
    (void)GetSystemController();

    // Ensure Service discovery is started
    (void)GetServiceDiscovery();

    // Ensure RequestReplyService is started
    (void)GetRequestReplyService();

    // Create the participants trace message sinks as declared in the configuration.
    _traceSinks = Tracing::CreateTraceMessageSinks(GetLogger(), _participantConfig);

    // NB: Create the lifecycleService and timeSyncService
    (void)GetLifecycleService();

    // NB: Create the systemMonitor to receive WorkflowConfigurations
    (void)GetSystemMonitor();

    // Enable replaying mechanism.
    if (Tracing::HasReplayConfig(_participantConfig))
    {
        _replayScheduler = std::make_unique<Tracing::ReplayScheduler>(_participantConfig, this);
        _replayScheduler->ConfigureTimeProvider(&_timeProvider);
        _logger->Info("Replay Scheduler active.");
    }

    CreateSystemInformationMetrics();

    if (_metricsTimerThread)
    {
        _metricsTimerThread->Start();
    }
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SetupRemoteLogging()
{
    auto* logger = dynamic_cast<Services::Logging::Logger*>(GetLogger());
    if (logger)
    {
        if (_participantConfig.logging.logFromRemotes)
        {
            Core::SupplementalData supplementalData;
            supplementalData[SilKit::Core::Discovery::controllerType] =
                SilKit::Core::Discovery::controllerTypeLoggerReceiver;

            Config::InternalController config;
            config.name = "LogMsgReceiver";
            config.network = "default";
            CreateController<Services::Logging::LogMsgReceiver>(config, std::move(supplementalData), true, true,
                                                                logger);
        }

        auto sinkIter = std::find_if(_participantConfig.logging.sinks.begin(), _participantConfig.logging.sinks.end(),
                                     [](const Config::Sink& sink) { return sink.type == Config::Sink::Type::Remote; });

        if (sinkIter != _participantConfig.logging.sinks.end())
        {
            Core::SupplementalData supplementalData;
            supplementalData[SilKit::Core::Discovery::controllerType] =
                SilKit::Core::Discovery::controllerTypeLoggerSender;

            Config::InternalController config;
            config.name = "LogMsgSender";
            config.network = "default";
            auto&& logMsgSender =
                CreateController<Services::Logging::LogMsgSender>(config, std::move(supplementalData), true, true);

            logger->RegisterRemoteLogging([logMsgSender, this](Services::Logging::LogMsg logMsg) {
                _connection.SendMsg(logMsgSender, std::move(logMsg));
            });
        }
    }
    else
    {
        Logging::Warn(GetLogger(),
                      "Failed to setup remote logging. Participant {} will not send and receive remote logs.",
                      GetParticipantName());
    }
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SetupMetrics()
{
    auto sender = GetOrCreateMetricsSender();

    auto& processor = dynamic_cast<VSilKit::MetricsProcessor&>(*_metricsProcessor);
    {
        auto sinks = VSilKit::CreateMetricsSinksFromParticipantConfiguration(
            _logger.get(), sender, GetParticipantName(), _participantConfig.experimental.metrics.sinks);
        processor.SetSinks(std::move(sinks));
    }

    // NB: Create the metrics manager prior to anything that might need it (possibly needs to be split into
    //     manager/sender explicitly)
    (void)GetMetricsManager();

    // NB: Create the metrics receiver if enabled in the configuration
    if (_participantConfig.experimental.metrics.collectFromRemote)
    {
        Core::SupplementalData supplementalData;
        supplementalData[SilKit::Core::Discovery::controllerType] =
            SilKit::Core::Discovery::controllerTypeMetricsReceiver;

        SilKit::Config::InternalController config;
        config.name = "MetricsReceiver";
        config.network = "default";
        CreateController<VSilKit::MetricsReceiver>(config, std::move(supplementalData), true, true, *_logger,
                                                   processor);
    }
}

template <class SilKitConnectionT>
inline void Participant<SilKitConnectionT>::SetTimeProvider(Orchestration::ITimeProvider* newClock)
{
    // Register the time provider with all already instantiated controllers
    auto setTimeProvider = [newClock](auto& controllers) {
        for (auto& controller : controllers)
        {
            auto* ctl = dynamic_cast<SilKit::Services::Orchestration::ITimeConsumer*>(controller.second.get());
            if (ctl)
            {
                ctl->SetTimeProvider(newClock);
            }
        }
    };
    tt::for_each(_controllers, setTimeProvider);
}

template <class SilKitConnectionT>
template <typename ConfigT>
auto Participant<SilKitConnectionT>::GetConfigByControllerName(const std::vector<ConfigT>& controllers,
                                                               const std::string& canonicalName) -> ConfigT
{
    ConfigT controllerConfig;
    auto it = std::find_if(controllers.begin(), controllers.end(),
                           [canonicalName](auto&& controllerConfig) { return controllerConfig.name == canonicalName; });
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

template <class SilKitConnectionT>
template <typename ValueT>
void Participant<SilKitConnectionT>::UpdateOptionalConfigValue(const std::string& controllerName,
                                                               SilKit::Util::Optional<ValueT>& configuredValue,
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

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::CreateCanController(const std::string& canonicalName,
                                                         const std::string& networkName) -> Can::ICanController*
{
    SilKit::Config::CanController controllerConfig =
        GetConfigByControllerName(_participantConfig.canControllers, canonicalName);
    UpdateOptionalConfigValue(canonicalName, controllerConfig.network, networkName);

    Core::SupplementalData supplementalData;
    supplementalData[SilKit::Core::Discovery::controllerType] = SilKit::Core::Discovery::controllerTypeCan;

    auto controller = CreateController<Can::CanController>(controllerConfig, std::move(supplementalData), true, true,
                                                           controllerConfig, &_timeProvider);

    controller->RegisterServiceDiscovery();

    Logging::Trace(GetLogger(), "Created CAN controller '{}' for network '{}' with service name '{}'",
                   controllerConfig.name, controllerConfig.network.value(),
                   controller->GetServiceDescriptor().to_string());

    if (_replayScheduler)
    {
        _replayScheduler->ConfigureController(controllerConfig.name, controller, controllerConfig.replay,
                                              controllerConfig.network.value(), controllerConfig.GetNetworkType());
    }

    auto* traceSource = dynamic_cast<ITraceMessageSource*>(controller);
    if (traceSource)
    {
        AddTraceSinksToSourceInternal(traceSource, controllerConfig);
    }

    return controller;
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::CreateEthernetController(
    const std::string& canonicalName, const std::string& networkName) -> Ethernet::IEthernetController*
{
    SilKit::Config::EthernetController controllerConfig =
        GetConfigByControllerName(_participantConfig.ethernetControllers, canonicalName);
    UpdateOptionalConfigValue(canonicalName, controllerConfig.network, networkName);

    Core::SupplementalData supplementalData;
    supplementalData[SilKit::Core::Discovery::controllerType] = SilKit::Core::Discovery::controllerTypeEthernet;

    auto* controller = CreateController<Ethernet::EthController>(controllerConfig, std::move(supplementalData), true,
                                                                 true, controllerConfig, &_timeProvider);

    controller->RegisterServiceDiscovery();

    Logging::Trace(GetLogger(), "Created Ethernet controller '{}' for network '{}' with service name '{}'",
                   controllerConfig.name, controllerConfig.network.value(),
                   controller->GetServiceDescriptor().to_string());

    if (_replayScheduler)
    {
        _replayScheduler->ConfigureController(controllerConfig.name, controller, controllerConfig.replay,
                                              controllerConfig.network.value(), controllerConfig.GetNetworkType());
    }

    auto* traceSource = dynamic_cast<ITraceMessageSource*>(controller);
    if (traceSource)
    {
        AddTraceSinksToSourceInternal(traceSource, controllerConfig);
    }

    return controller;
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::CreateFlexrayController(
    const std::string& canonicalName, const std::string& networkName) -> Services::Flexray::IFlexrayController*
{
    SilKit::Config::FlexrayController controllerConfig =
        GetConfigByControllerName(_participantConfig.flexrayControllers, canonicalName);
    UpdateOptionalConfigValue(canonicalName, controllerConfig.network, networkName);

    Core::SupplementalData supplementalData;
    supplementalData[SilKit::Core::Discovery::controllerType] = SilKit::Core::Discovery::controllerTypeFlexray;

    auto controller = CreateController<Flexray::FlexrayController>(controllerConfig, std::move(supplementalData), true,
                                                                   true, controllerConfig, &_timeProvider);

    controller->RegisterServiceDiscovery();

    Logging::Trace(GetLogger(), "Created FlexRay controller '{}' for network '{}' with service name '{}'",
                   controllerConfig.name, controllerConfig.network.value(),
                   controller->GetServiceDescriptor().to_string());

    auto* traceSource = dynamic_cast<ITraceMessageSource*>(controller);
    if (traceSource)
    {
        AddTraceSinksToSourceInternal(traceSource, controllerConfig);
    }

    return controller;
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::CreateLinController(const std::string& canonicalName,
                                                         const std::string& networkName) -> Lin::ILinController*
{
    SilKit::Config::LinController controllerConfig =
        GetConfigByControllerName(_participantConfig.linControllers, canonicalName);
    UpdateOptionalConfigValue(canonicalName, controllerConfig.network, networkName);

    Core::SupplementalData supplementalData;
    supplementalData[SilKit::Core::Discovery::controllerType] = SilKit::Core::Discovery::controllerTypeLin;

    auto controller = CreateController<Lin::LinController>(controllerConfig, std::move(supplementalData), true, true,
                                                           controllerConfig, &_timeProvider);

    controller->RegisterServiceDiscovery();

    Logging::Trace(GetLogger(), "Created LIN controller '{}' for network '{}' with service name '{}'",
                   controllerConfig.name, controllerConfig.network.value(),
                   controller->GetServiceDescriptor().to_string());

    if (_replayScheduler)
    {
        _replayScheduler->ConfigureController(controllerConfig.name, controller, controllerConfig.replay,
                                              controllerConfig.network.value(), controllerConfig.GetNetworkType());
    }


    auto* traceSource = dynamic_cast<ITraceMessageSource*>(controller);
    if (traceSource)
    {
        AddTraceSinksToSourceInternal(traceSource, controllerConfig);
    }

    return controller;
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::CreateDataSubscriberInternal(
    const std::string& topic, const std::string& linkName, const std::string& mediaType,
    const std::vector<SilKit::Services::MatchingLabel>& publisherLabels,
    Services::PubSub::DataMessageHandler defaultHandler,
    Services::PubSub::IDataSubscriber* parent) -> Services::PubSub::DataSubscriberInternal*
{
    Core::SupplementalData supplementalData;
    supplementalData[SilKit::Core::Discovery::controllerType] =
        SilKit::Core::Discovery::controllerTypeDataSubscriberInternal;
    auto parentDataSubscriber = dynamic_cast<Services::PubSub::DataSubscriber*>(parent);
    if (parentDataSubscriber)
    {
        supplementalData[SilKit::Core::Discovery::supplKeyDataSubscriberInternalParentServiceID] =
            std::to_string(parentDataSubscriber->GetServiceDescriptor().GetServiceId());
    }
    SilKit::Config::DataSubscriber controllerConfig;

    // Use a unique name to avoid collisions of several subscribers on same topic on one participant
    controllerConfig.name = to_string(Util::Uuid::GenerateRandom());
    std::string network = linkName;

    auto controller = CreateController<PubSub::DataSubscriberInternal>(
        controllerConfig, network, std::move(supplementalData), true, true, &_timeProvider, topic, mediaType,
        publisherLabels, defaultHandler, parent);

    //Restore original DataSubscriber config for replay
    auto&& parentConfig = parentDataSubscriber->GetConfig();
    if (_replayScheduler)
    {
        _replayScheduler->ConfigureController(parentConfig.name, controller, parentConfig.replay,
                                              parentConfig.topic.value(), parentConfig.GetNetworkType());
    }
    return controller;
}

static inline auto FormatLabelsForLogging(const std::vector<MatchingLabel>& labels) -> std::string
{
    std::ostringstream os;

    if (labels.empty())
    {
        os << "(no labels)";
    }

    bool first = true;

    for (const auto& label : labels)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            os << ", ";
        }

        switch (label.kind)
        {
        case MatchingLabel::Kind::Optional:
            os << "Optional";
            break;
        case MatchingLabel::Kind::Mandatory:
            os << "Mandatory";
            break;
        default:
            os << "MatchingLabel::Kind(" << static_cast<std::underlying_type_t<MatchingLabel::Kind>>(label.kind) << ")";
            break;
        }

        os << " '" << label.key << "': '" << label.value << "'";
    }

    return os.str();
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::CreateDataPublisher(const std::string& canonicalName,
                                                         const SilKit::Services::PubSub::PubSubSpec& dataSpec,
                                                         size_t history) -> Services::PubSub::IDataPublisher*
{
    if (history > 1)
    {
        throw SilKit::ConfigurationError("DataPublishers do not support history > 1.");
    }

    std::string network = to_string(Util::Uuid::GenerateRandom());

    // Merge config and parameters, sort labels
    SilKit::Config::DataPublisher controllerConfig =
        GetConfigByControllerName(_participantConfig.dataPublishers, canonicalName);
    UpdateOptionalConfigValue(canonicalName, controllerConfig.topic, dataSpec.Topic());
    SilKit::Services::PubSub::PubSubSpec configuredDataNodeSpec{controllerConfig.topic.value(), dataSpec.MediaType()};
    auto labels = dataSpec.Labels();
    std::sort(labels.begin(), labels.end(),
              [](const MatchingLabel& v1, const MatchingLabel& v2) { return v1.key < v2.key; });
    for (auto label : labels)
    {
        configuredDataNodeSpec.AddLabel(label);
    }

    SilKit::Core::SupplementalData supplementalData;
    supplementalData[SilKit::Core::Discovery::controllerType] = SilKit::Core::Discovery::controllerTypeDataPublisher;
    supplementalData[SilKit::Core::Discovery::supplKeyDataPublisherTopic] = configuredDataNodeSpec.Topic();
    supplementalData[SilKit::Core::Discovery::supplKeyDataPublisherPubUUID] = network;
    supplementalData[SilKit::Core::Discovery::supplKeyDataPublisherMediaType] = configuredDataNodeSpec.MediaType();
    auto labelStr = SilKit::Config::Serialize<std::decay_t<decltype(labels)>>(labels);
    supplementalData[SilKit::Core::Discovery::supplKeyDataPublisherPubLabels] = labelStr;

    auto controller = CreateController<Services::PubSub::DataPublisher>(
        controllerConfig, network, std::move(supplementalData), true, true, &_timeProvider, configuredDataNodeSpec,
        network, controllerConfig);

    _connection.SetHistoryLengthForLink(history, controller);

    if (GetLogger()->GetLogLevel() <= Logging::Level::Trace)
    {
        Logging::Trace(
            GetLogger(),
            "Created DataPublisher '{}' with topic '{}' and media type '{}' for network '{}' with service name "
            "'{}' and labels: {}",
            controllerConfig.name, controllerConfig.topic.value(), dataSpec.MediaType(), network,
            controller->GetServiceDescriptor().to_string(), FormatLabelsForLogging(dataSpec.Labels()));
    }

    auto* traceSource = dynamic_cast<ITraceMessageSource*>(controller);
    if (traceSource)
    {
        AddTraceSinksToSourceInternal(traceSource, controllerConfig);
    }

    if (_replayScheduler)
    {
        _replayScheduler->ConfigureController(controllerConfig.name, controller, controllerConfig.replay,
                                              controllerConfig.topic.value(), controllerConfig.GetNetworkType());
    }

    return controller;
}


template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::CreateDataSubscriber(
    const std::string& canonicalName, const SilKit::Services::PubSub::PubSubSpec& dataSpec,
    SilKit::Services::PubSub::DataMessageHandler defaultDataHandler) -> Services::PubSub::IDataSubscriber*
{
    // DataSubscriber has no registered messages (discovers DataPublishers and creates DataSubscriberInternal),
    // so the network name is irrelevant.
    const auto network = "default";

    // Merge config and parameters, sort labels
    SilKit::Config::DataSubscriber controllerConfig =
        GetConfigByControllerName(_participantConfig.dataSubscribers, canonicalName);
    UpdateOptionalConfigValue(canonicalName, controllerConfig.topic, dataSpec.Topic());

    SilKit::Services::PubSub::PubSubSpec configuredDataNodeSpec{controllerConfig.topic.value(), dataSpec.MediaType()};
    auto labels = dataSpec.Labels();
    std::sort(labels.begin(), labels.end(),
              [](const MatchingLabel& v1, const MatchingLabel& v2) { return v1.key < v2.key; });
    for (auto label : labels)
    {
        configuredDataNodeSpec.AddLabel(label);
    }

    Core::SupplementalData supplementalData;
    supplementalData[SilKit::Core::Discovery::controllerType] = SilKit::Core::Discovery::controllerTypeDataSubscriber;
    supplementalData[SilKit::Core::Discovery::supplKeyDataSubscriberTopic] = configuredDataNodeSpec.Topic();
    supplementalData[SilKit::Core::Discovery::supplKeyDataSubscriberMediaType] = configuredDataNodeSpec.MediaType();
    auto labelStr = SilKit::Config::Serialize<std::decay_t<decltype(labels)>>(labels);
    supplementalData[SilKit::Core::Discovery::supplKeyDataSubscriberSubLabels] = labelStr;

    auto controller = CreateController<Services::PubSub::DataSubscriber>(
        controllerConfig, network, std::move(supplementalData), true, true, controllerConfig, &_timeProvider,
        configuredDataNodeSpec, defaultDataHandler);

    controller->RegisterServiceDiscovery();

    if (GetLogger()->GetLogLevel() <= Logging::Level::Trace)
    {
        Logging::Trace(
            GetLogger(),
            "Created DataSubscriber '{}' with topic '{}' and media type '{}' for network '{}' with service name "
            "'{}' and labels: {}",
            controllerConfig.name, controllerConfig.topic.value(), dataSpec.MediaType(), network,
            controller->GetServiceDescriptor().to_string(), FormatLabelsForLogging(dataSpec.Labels()));
    }

    auto* traceSource = dynamic_cast<ITraceMessageSource*>(controller);
    if (traceSource)
    {
        AddTraceSinksToSourceInternal(traceSource, controllerConfig);
    }

    return controller;
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::CreateRpcServerInternal(
    const std::string& functionName, const std::string& clientUUID, const std::string& mediaType,
    const std::vector<SilKit::Services::MatchingLabel>& clientLabels, Services::Rpc::RpcCallHandler handler,
    Services::Rpc::IRpcServer* parent) -> Services::Rpc::RpcServerInternal*
{
    Logging::Trace(GetLogger(), "Creating internal server for functionName={}, clientUUID={}", functionName,
                   clientUUID);

    SilKit::Config::RpcServer controllerConfig;
    // Use a unique name to avoid collisions of several RpcSevers on same functionName on one participant
    controllerConfig.name = to_string(Util::Uuid::GenerateRandom());
    std::string network = clientUUID;

    // RpcServerInternal gets discovered by RpcClient which is then ready to detach calls
    SilKit::Core::SupplementalData supplementalData;
    supplementalData[SilKit::Core::Discovery::controllerType] =
        SilKit::Core::Discovery::controllerTypeRpcServerInternal;
    supplementalData[SilKit::Core::Discovery::supplKeyRpcServerInternalClientUUID] = clientUUID;
    auto parentRpcServer = dynamic_cast<Services::Rpc::RpcServer*>(parent);
    if (parentRpcServer)
    {
        supplementalData[SilKit::Core::Discovery::supplKeyRpcServerInternalParentServiceID] =
            std::to_string(parentRpcServer->GetServiceDescriptor().GetServiceId());
    }
    return CreateController<Services::Rpc::RpcServerInternal>(controllerConfig, network, std::move(supplementalData),
                                                              true, true, &_timeProvider, functionName, mediaType,
                                                              clientLabels, clientUUID, handler, parent);
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::CreateRpcClient(
    const std::string& canonicalName, const SilKit::Services::Rpc::RpcSpec& dataSpec,
    Services::Rpc::RpcCallResultHandler handler) -> Services::Rpc::IRpcClient*
{
    // RpcClient communicates on a unique network
    auto network = to_string(Util::Uuid::GenerateRandom());

    SilKit::Config::RpcClient controllerConfig =
        GetConfigByControllerName(_participantConfig.rpcClients, canonicalName);
    UpdateOptionalConfigValue(canonicalName, controllerConfig.functionName, dataSpec.FunctionName());

    // RpcClient gets discovered by RpcServer which creates RpcServerInternal on a matching connection
    Core::SupplementalData supplementalData;
    supplementalData[SilKit::Core::Discovery::controllerType] = SilKit::Core::Discovery::controllerTypeRpcClient;
    supplementalData[SilKit::Core::Discovery::supplKeyRpcClientFunctionName] = controllerConfig.functionName.value();
    supplementalData[SilKit::Core::Discovery::supplKeyRpcClientMediaType] = dataSpec.MediaType();
    const auto& labels = dataSpec.Labels();
    auto labelStr = SilKit::Config::Serialize<std::decay_t<decltype(labels)>>(labels);
    supplementalData[SilKit::Core::Discovery::supplKeyRpcClientLabels] = labelStr;
    supplementalData[SilKit::Core::Discovery::supplKeyRpcClientUUID] = network;

    SilKit::Services::Rpc::RpcSpec configuredDataSpec{controllerConfig.functionName.value(), dataSpec.MediaType()};
    for (auto label : dataSpec.Labels())
    {
        configuredDataSpec.AddLabel(label);
    }

    auto controller =
        CreateController<Services::Rpc::RpcClient>(controllerConfig, network, std::move(supplementalData), true, true,
                                                   &_timeProvider, configuredDataSpec, network, handler);

    // RpcClient discovers RpcServerInternal and is ready to dispatch calls
    controller->RegisterServiceDiscovery();

    if (GetLogger()->GetLogLevel() <= Logging::Level::Trace)
    {
        Logging::Trace(
            GetLogger(),
            "Created RPC Client '{}' with function name '{}' and media type '{}' for network '{}' with service name "
            "'{}' and labels: {}",
            controllerConfig.name, controllerConfig.functionName.value(), dataSpec.MediaType(), network,
            controller->GetServiceDescriptor().to_string(), FormatLabelsForLogging(dataSpec.Labels()));
    }

    return controller;
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::CreateRpcServer(
    const std::string& canonicalName, const SilKit::Services::Rpc::RpcSpec& dataSpec,
    Services::Rpc::RpcCallHandler handler) -> Services::Rpc::IRpcServer*
{
    // RpcServer has no registered messages (discovers RpcClients and creates RpcServerInternal),
    // so the network name is irrelevant.
    auto network = "default";

    SilKit::Config::RpcServer controllerConfig =
        GetConfigByControllerName(_participantConfig.rpcServers, canonicalName);
    UpdateOptionalConfigValue(canonicalName, controllerConfig.functionName, dataSpec.FunctionName());

    Core::SupplementalData supplementalData;
    supplementalData[SilKit::Core::Discovery::controllerType] = SilKit::Core::Discovery::controllerTypeRpcServer;
    // Needed for RpcServer discovery in tests
    supplementalData[SilKit::Core::Discovery::supplKeyRpcServerFunctionName] = controllerConfig.functionName.value();
    supplementalData[SilKit::Core::Discovery::supplKeyRpcServerMediaType] = dataSpec.MediaType();
    const auto& labels = dataSpec.Labels();
    auto labelStr = SilKit::Config::Serialize<std::decay_t<decltype(labels)>>(labels);
    supplementalData[SilKit::Core::Discovery::supplKeyRpcServerLabels] = labelStr;

    SilKit::Services::Rpc::RpcSpec configuredDataSpec{controllerConfig.functionName.value(), dataSpec.MediaType()};
    for (auto label : dataSpec.Labels())
    {
        configuredDataSpec.AddLabel(label);
    }

    auto controller = CreateController<Services::Rpc::RpcServer>(controllerConfig, network, supplementalData, true,
                                                                 true, &_timeProvider, configuredDataSpec, handler);

    // RpcServer discovers RpcClient and creates RpcServerInternal on a matching connection
    controller->RegisterServiceDiscovery();

    if (GetLogger()->GetLogLevel() <= Logging::Level::Trace)
    {
        Logging::Trace(
            GetLogger(),
            "Created RPC Server '{}' with function name '{}' and media type '{}' for network '{}' with service name "
            "'{}' and labels: {}",
            controllerConfig.name, controllerConfig.functionName.value(), dataSpec.MediaType(), network,
            controller->GetServiceDescriptor().to_string(), FormatLabelsForLogging(dataSpec.Labels()));
    }

    return controller;
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::CreateTimeSyncService(Orchestration::LifecycleService* lifecycleService)
    -> Services::Orchestration::TimeSyncService*
{
    auto* timeSyncService =
        GetController<Orchestration::TimeSyncService>(SilKit::Core::Discovery::controllerTypeTimeSyncService);

    if (timeSyncService)
    {
        throw SilKitError("Tried to instantiate TimeSyncService multiple times!");
    }


    Core::SupplementalData timeSyncSupplementalData;
    timeSyncSupplementalData[SilKit::Core::Discovery::controllerType] =
        SilKit::Core::Discovery::controllerTypeTimeSyncService;

    Config::InternalController config;
    config.name = Discovery::controllerTypeTimeSyncService;
    config.network = "default";
    timeSyncService = CreateController<Orchestration::TimeSyncService>(
        config, std::move(timeSyncSupplementalData), false, false, &_timeProvider, _participantConfig.healthCheck,
        lifecycleService, _participantConfig.experimental.timeSynchronization.animationFactor);

    return timeSyncService;
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::GetLifecycleService() -> Services::Orchestration::ILifecycleService*
{
    auto* lifecycleService =
        GetController<Orchestration::LifecycleService>(SilKit::Core::Discovery::controllerTypeLifecycleService);

    if (!lifecycleService)
    {
        Core::SupplementalData lifecycleSupplementalData;
        lifecycleSupplementalData[SilKit::Core::Discovery::controllerType] =
            SilKit::Core::Discovery::controllerTypeLifecycleService;

        Config::InternalController config;
        config.name = Discovery::controllerTypeLifecycleService;
        config.network = "default";
        lifecycleService = CreateController<Orchestration::LifecycleService>(
            config, std::move(lifecycleSupplementalData), false, true);
    }
    return lifecycleService;
}


static inline auto FormatLifecycleConfigurationForLogging(
    const Services::Orchestration::LifecycleConfiguration& lifecycleConfiguration) -> std::string
{
    std::ostringstream os;

    using Services::Orchestration::OperationMode;

    os << "LifecycleConfiguration{operationMode=";
    switch (lifecycleConfiguration.operationMode)
    {
    case OperationMode::Invalid:
        os << "Invalid";
        break;
    case OperationMode::Coordinated:
        os << "Coordinated";
        break;
    case OperationMode::Autonomous:
        os << "Autonomous";
        break;
    default:
        os << "OperationMode("
           << static_cast<std::underlying_type_t<OperationMode>>(lifecycleConfiguration.operationMode) << ")";
    }

    os << "}";

    return os.str();
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::CreateLifecycleService(
    Services::Orchestration::LifecycleConfiguration startConfiguration) -> Services::Orchestration::ILifecycleService*
{
    if (startConfiguration.operationMode == SilKit::Services::Orchestration::OperationMode::Invalid)
    {
        throw ConfigurationError("Cannot create lifecycle service with OperationMode::Invalid");
    }

    if (_isLifecycleServiceCreated)
    {
        throw SilKitError("You may not create the lifecycle service more than once.");
    }
    _isLifecycleServiceCreated = true;

    auto* lifecycleService = GetLifecycleService();
    dynamic_cast<SilKit::Services::Orchestration::LifecycleService*>(lifecycleService)
        ->SetLifecycleConfiguration(startConfiguration);

    Logging::Trace(GetLogger(), "Created Lifecycle with operating mode {}",
                   FormatLifecycleConfigurationForLogging(startConfiguration));

    return lifecycleService;
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::GetSystemMonitor() -> Services::Orchestration::ISystemMonitor*
{
    auto* controller =
        GetController<Orchestration::SystemMonitor>(SilKit::Core::Discovery::controllerTypeSystemMonitor);
    if (!controller)
    {
        Core::SupplementalData supplementalData;
        supplementalData[SilKit::Core::Discovery::controllerType] =
            SilKit::Core::Discovery::controllerTypeSystemMonitor;

        Config::InternalController config;
        config.name = Discovery::controllerTypeSystemMonitor;
        config.network = "default";
        controller = CreateController<Orchestration::SystemMonitor>(config, std::move(supplementalData), true, true);

        _connection.RegisterMessageReceiver([controller](IVAsioPeer* peer, const ParticipantAnnouncement&) {
            controller->OnParticipantConnected(
                Services::Orchestration::ParticipantConnectionInformation{peer->GetInfo().participantName});
        });

        _connection.RegisterPeerShutdownCallback([controller](IVAsioPeer* peer) {
            controller->OnParticipantDisconnected(
                Services::Orchestration::ParticipantConnectionInformation{peer->GetInfo().participantName});
        });
    }
    return controller;
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::CreateSystemMonitor() -> Services::Orchestration::ISystemMonitor*
{
    if (_isSystemMonitorCreated)
    {
        throw SilKitError("You may not create the system monitor more than once.");
    }
    _isSystemMonitorCreated = true;
    return GetSystemMonitor();
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::GetServiceDiscovery() -> Discovery::IServiceDiscovery*
{
    auto* controller = GetController<SilKit::Core::Discovery::ServiceDiscovery>(
        SilKit::Core::Discovery::controllerTypeServiceDiscovery);
    if (!controller)
    {
        Core::SupplementalData supplementalData;
        supplementalData[SilKit::Core::Discovery::controllerType] =
            SilKit::Core::Discovery::controllerTypeServiceDiscovery;

        Config::InternalController config;
        config.name = Discovery::controllerTypeServiceDiscovery;
        config.network = "default";
        controller = CreateController<SilKit::Core::Discovery::ServiceDiscovery>(config, std::move(supplementalData),
                                                                                 true, true, GetParticipantName());

        _connection.RegisterPeerShutdownCallback(
            [controller](IVAsioPeer* peer) { controller->OnParticpantRemoval(peer->GetInfo().participantName); });
    }
    return controller;
}


template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::GetRequestReplyService() -> RequestReply::IRequestReplyService*
{
    auto* controller =
        GetController<RequestReply::RequestReplyService>(SilKit::Core::Discovery::controllerTypeRequestReplyService);
    if (!controller)
    {
        Core::SupplementalData supplementalData;
        supplementalData[SilKit::Core::Discovery::controllerType] =
            SilKit::Core::Discovery::controllerTypeRequestReplyService;

        _participantReplies = std::make_unique<RequestReply::ParticipantReplies>(this, controller);

        RequestReply::ProcedureMap procedures{
            {RequestReply::FunctionType::ParticipantReplies, _participantReplies.get()}};

        Config::InternalController config;
        config.name = "RequestReplyService";
        config.network = "default";
        controller = CreateController<RequestReply::RequestReplyService>(config, std::move(supplementalData), true,
                                                                         true, GetParticipantName(), procedures);

        _connection.RegisterPeerShutdownCallback(
            [controller](IVAsioPeer* peer) { controller->OnParticpantRemoval(peer->GetInfo().participantName); });
    }
    return controller;
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::GetParticipantRepliesProcedure() -> RequestReply::IParticipantReplies*
{
    return _participantReplies.get();
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::GetMetricsManager() -> IMetricsManager*
{
    return _metricsManager.get();
}

template <class SilKitConnectionT>
bool Participant<SilKitConnectionT>::GetIsSystemControllerCreated()
{
    return _isSystemControllerCreated;
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SetIsSystemControllerCreated(bool isCreated)
{
    _isSystemControllerCreated = isCreated;
}

template <class SilKitConnectionT>
bool Participant<SilKitConnectionT>::GetIsNetworkSimulatorCreated()
{
    return _isNetworkSimulatorCreated;
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SetIsNetworkSimulatorCreated(bool isCreated)
{
    _isNetworkSimulatorCreated = isCreated;
}


template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::GetSystemController() -> Experimental::Services::Orchestration::ISystemController*
{
    auto* controller =
        GetController<Orchestration::SystemController>(SilKit::Core::Discovery::controllerTypeSystemController);
    if (!controller)
    {
        Core::SupplementalData supplementalData;
        supplementalData[SilKit::Core::Discovery::controllerType] =
            SilKit::Core::Discovery::controllerTypeSystemController;

        Config::InternalController config;
        config.name = SilKit::Core::Discovery::controllerTypeSystemController;
        config.network = "default";

        return CreateController<Orchestration::SystemController>(config, std::move(supplementalData), true, true);
    }
    return controller;
}


template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::CreateNetworkSimulator() -> Experimental::NetworkSimulation::INetworkSimulator*
{
    if (_networkSimulatorInternal != nullptr)
    {
        throw SilKitError("You may not create the network simulator more than once.");
    }

    _networkSimulatorInternal = std::make_unique<Experimental::NetworkSimulation::NetworkSimulatorInternal>(this);
    return _networkSimulatorInternal.get();
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::GetLogger() -> Services::Logging::ILogger*
{
    return _logger.get();
}

template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::GetLoggerInternal() -> Services::Logging::ILoggerInternal*
{
    return _logger.get();
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const Can::WireCanFrameEvent& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const Can::CanFrameTransmitEvent& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const Can::CanControllerStatus& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const Can::CanConfigureBaudrate& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const Can::CanSetControllerMode& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const Ethernet::WireEthernetFrameEvent& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Ethernet::EthernetFrameTransmitEvent& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const Ethernet::EthernetStatus& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const Ethernet::EthernetSetMode& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Flexray::WireFlexrayFrameEvent& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Flexray::WireFlexrayFrameTransmitEvent& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Flexray::FlexraySymbolEvent& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Flexray::FlexraySymbolTransmitEvent& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Flexray::FlexrayCycleStartEvent& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Flexray::FlexrayHostCommand& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Flexray::FlexrayControllerConfig& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Flexray::FlexrayTxBufferConfigUpdate& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Flexray::WireFlexrayTxBufferUpdate& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Flexray::FlexrayPocStatusEvent& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Lin::LinSendFrameRequest& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Lin::LinSendFrameHeaderRequest& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const Services::Lin::LinTransmission& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const Services::Lin::LinWakeupPulse& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Lin::WireLinControllerConfig& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Lin::LinControllerStatusUpdate& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Lin::LinFrameResponseUpdate& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::PubSub::WireDataMessageEvent& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const Services::Rpc::FunctionCall& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, Services::Rpc::FunctionCall&& msg)
{
    SendMsgImpl(from, std::move(msg));
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Rpc::FunctionCallResponse& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, Services::Rpc::FunctionCallResponse&& msg)
{
    SendMsgImpl(from, std::move(msg));
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Orchestration::NextSimTask& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Orchestration::ParticipantStatus& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Orchestration::SystemCommand& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Services::Orchestration::WorkflowConfiguration& msg)
{
    SendMsgImpl(from, msg);
}


template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const Services::Logging::LogMsg& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, Services::Logging::LogMsg&& msg)
{
    SendMsgImpl(from, std::move(msg));
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const VSilKit::MetricsUpdate& msg)
{
    SendMsgImpl(from, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const Discovery::ParticipantDiscoveryEvent& msg)
{
    SendMsgImpl(from, std::move(msg));
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const Discovery::ServiceDiscoveryEvent& msg)
{
    SendMsgImpl(from, std::move(msg));
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const RequestReply::RequestReplyCall& msg)
{
    SendMsgImpl(from, std::move(msg));
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from,
                                             const RequestReply::RequestReplyCallReturn& msg)
{
    SendMsgImpl(from, std::move(msg));
}

template <class SilKitConnectionT>
template <typename SilKitMessageT>
void Participant<SilKitConnectionT>::SendMsgImpl(const IServiceEndpoint* from, SilKitMessageT&& msg)
{
    TraceTx(GetLogger(), from, msg);
    _connection.SendMsg(from, std::forward<SilKitMessageT>(msg));
}

// Targeted messaging
template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Can::WireCanFrameEvent& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Can::CanFrameTransmitEvent& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Can::CanControllerStatus& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Can::CanConfigureBaudrate& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Can::CanSetControllerMode& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Ethernet::WireEthernetFrameEvent& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Ethernet::EthernetFrameTransmitEvent& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Ethernet::EthernetStatus& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Ethernet::EthernetSetMode& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Flexray::WireFlexrayFrameEvent& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Flexray::WireFlexrayFrameTransmitEvent& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Flexray::FlexraySymbolEvent& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Flexray::FlexraySymbolTransmitEvent& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Flexray::FlexrayCycleStartEvent& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Flexray::FlexrayHostCommand& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Flexray::FlexrayControllerConfig& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Flexray::FlexrayTxBufferConfigUpdate& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Flexray::WireFlexrayTxBufferUpdate& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Flexray::FlexrayPocStatusEvent& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Lin::LinSendFrameRequest& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Lin::LinSendFrameHeaderRequest& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Lin::LinTransmission& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Lin::LinWakeupPulse& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Lin::WireLinControllerConfig& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Lin::LinControllerStatusUpdate& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Lin::LinFrameResponseUpdate& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::PubSub::WireDataMessageEvent& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Rpc::FunctionCall& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             Services::Rpc::FunctionCall&& msg)
{
    SendMsgImpl(from, targetParticipantName, std::move(msg));
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Rpc::FunctionCallResponse& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             Services::Rpc::FunctionCallResponse&& msg)
{
    SendMsgImpl(from, targetParticipantName, std::move(msg));
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Orchestration::NextSimTask& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Orchestration::ParticipantStatus& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Orchestration::SystemCommand& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Orchestration::WorkflowConfiguration& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Services::Logging::LogMsg& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             Services::Logging::LogMsg&& msg)
{
    SendMsgImpl(from, targetParticipantName, std::move(msg));
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const VSilKit::MetricsUpdate& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Discovery::ParticipantDiscoveryEvent& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const Discovery::ServiceDiscoveryEvent& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const RequestReply::RequestReplyCall& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                             const RequestReply::RequestReplyCallReturn& msg)
{
    SendMsgImpl(from, targetParticipantName, msg);
}

template <class SilKitConnectionT>
template <typename SilKitMessageT>
void Participant<SilKitConnectionT>::SendMsgImpl(const IServiceEndpoint* from, const std::string& targetParticipantName,
                                                 SilKitMessageT&& msg)
{
    TraceTx(GetLogger(), from, msg);
    _connection.SendMsg(from, targetParticipantName, std::forward<SilKitMessageT>(msg));
}


template <class SilKitConnectionT>
template <class ControllerT>
auto Participant<SilKitConnectionT>::GetController(const std::string& serviceName) -> ControllerT*
{
    auto&& controllerMap = tt::predicative_get<tt::rbind<IsControllerMap, ControllerT>::template type>(_controllers);
    const auto qualifiedName = serviceName;
    if (controllerMap.count(qualifiedName))
    {
        return static_cast<ControllerT*>(controllerMap.at(qualifiedName).get());
    }
    else
    {
        return nullptr;
    }
}

template <class SilKitConnectionT>
template <class ControllerT, typename... Arg>
auto Participant<SilKitConnectionT>::CreateController(const SilKitServiceTraitConfigType_t<ControllerT>& config,
                                                      const SilKit::Core::SupplementalData& supplementalData,
                                                      const bool publishServiceDiscovery,
                                                      const bool registerSilKitService, Arg&&... arg) -> ControllerT*
{
    SILKIT_ASSERT(config.network.has_value());
    return CreateController<ControllerT>(config, *config.network, supplementalData, publishServiceDiscovery,
                                         registerSilKitService, std::forward<Arg>(arg)...);
}

template <class SilKitConnectionT>
template <class ControllerT, typename... Arg>
auto Participant<SilKitConnectionT>::CreateController(const SilKitServiceTraitConfigType_t<ControllerT>& config,
                                                      const std::string& network,
                                                      const SilKit::Core::SupplementalData& supplementalData,
                                                      const bool publishServiceDiscovery,
                                                      const bool registerSilKitService, Arg&&... arg) -> ControllerT*
{
    const auto serviceType = SilKitServiceTraitServiceType<ControllerT>::GetServiceType();
    if (config.name == "")
    {
        throw SilKit::ConfigurationError("Services must have a non-empty name.");
    }

    // If possible, load controller from cache
    auto* controllerPtr = GetController<ControllerT>(config.name);
    if (controllerPtr != nullptr)
    {
        throw SilKit::ConfigurationError(fmt::format("Service {} in network {} already exists.", config.name, network));
    }

    auto&& controllerMap = tt::predicative_get<tt::rbind<IsControllerMap, ControllerT>::template type>(_controllers);
    auto controller = std::make_unique<ControllerT>(this, std::forward<Arg>(arg)...);
    controllerPtr = controller.get();

    auto localEndpoint = _localEndpointId++;

    auto descriptor = ServiceDescriptor{};
    descriptor.SetNetworkName(network);
    descriptor.SetParticipantNameAndComputeId(GetParticipantName());
    descriptor.SetServiceName(config.name);
    descriptor.SetNetworkType(config.GetNetworkType());
    descriptor.SetServiceId(localEndpoint);
    descriptor.SetServiceType(serviceType);
    descriptor.SetSupplementalData(supplementalData);

    controller->SetServiceDescriptor(std::move(descriptor));

    if (registerSilKitService)
    {
        _connection.RegisterSilKitService(controllerPtr);
    }

    const auto qualifiedName = config.name;
    controllerMap[qualifiedName] = std::move(controller);

    if (publishServiceDiscovery)
    {
        GetServiceDiscovery()->NotifyServiceCreated(controllerPtr->GetServiceDescriptor());
    }
    return controllerPtr;
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::RegisterTimeSyncService(
    SilKit::Services::Orchestration::TimeSyncService* controllerPtr)
{
    if (controllerPtr)
    {
        _connection.RegisterSilKitService(controllerPtr);
    }
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::AddTraceSinksToSource(ITraceMessageSource* traceSource,
                                                           SilKit::Config::SimulatedNetwork config)
{
    AddTraceSinksToSourceInternal(traceSource, config);
}

template <class SilKitConnectionT>
template <class ConfigT>
void Participant<SilKitConnectionT>::AddTraceSinksToSourceInternal(ITraceMessageSource* traceSource, ConfigT config)
{
    if (config.useTraceSinks.empty())
    {
        Logging::Debug(GetLogger(), "Tracer on {}/{} not enabled, skipping", GetParticipantName(), config.name);
        return;
    }
    auto findSinkByName = [this](const auto& name) {
        return std::find_if(_traceSinks.begin(), _traceSinks.end(),
                            [&name](const auto& sinkPtr) { return sinkPtr->Name() == name; });
    };

    for (const auto& sinkName : config.useTraceSinks)
    {
        auto sinkIter = findSinkByName(sinkName);
        if (sinkIter == _traceSinks.end())
        {
            Logging::Warn(GetLogger(), "Tracing: the service '{}' refers to non-existing trace sink '{}'", config.name,
                          sinkName);
            continue;
        }
        traceSource->AddSink((*sinkIter).get(), config.GetNetworkType());
    }
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::RegisterSimulator(
    ISimulator* busSim, std::string networkName, Experimental::NetworkSimulation::SimulatedNetworkType networkType)
{
    auto serviceDescriptor = ServiceDescriptor{};
    serviceDescriptor.SetNetworkName(networkName);
    serviceDescriptor.SetServiceName(networkName);
    serviceDescriptor.SetNetworkType(ConvertNetworkTypeToConfig(networkType));
    serviceDescriptor.SetParticipantNameAndComputeId(GetParticipantName());
    busSim->SetServiceDescriptor(serviceDescriptor);

    // Tell the middleware we are interested in this named network of the given type
    switch (networkType)
    {
    case Experimental::NetworkSimulation::SimulatedNetworkType::CAN:
        _connection.RegisterSilKitService(dynamic_cast<Services::Can::IMsgForCanSimulator*>(busSim));
        break;
    case Experimental::NetworkSimulation::SimulatedNetworkType::FlexRay:
        _connection.RegisterSilKitService(dynamic_cast<Services::Flexray::IMsgForFlexraySimulator*>(busSim));
        break;
    case Experimental::NetworkSimulation::SimulatedNetworkType::LIN:
        _connection.RegisterSilKitService(dynamic_cast<Services::Lin::IMsgForLinSimulator*>(busSim));
        break;
    case Experimental::NetworkSimulation::SimulatedNetworkType::Ethernet:
        _connection.RegisterSilKitService(dynamic_cast<Services::Ethernet::IMsgForEthSimulator*>(busSim));
        break;
    default:
        throw SilKitError{"RegisterSimulator: simulator does not support given network type: "
                          + to_string(networkType)};
    }
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::OnAllMessagesDelivered(std::function<void()> callback)
{
    _connection.OnAllMessagesDelivered(std::move(callback));
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::FlushSendBuffers()
{
    _connection.FlushSendBuffers();
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::ExecuteDeferred(std::function<void()> callback)
{
    _connection.ExecuteDeferred(std::move(callback));
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::AddAsyncSubscriptionsCompletionHandler(std::function<void()> handler)
{
    _connection.AddAsyncSubscriptionsCompletionHandler(std::move(handler));
}

template <class SilKitConnectionT>
template <typename ValueT>
void Participant<SilKitConnectionT>::LogMismatchBetweenConfigAndPassedValue(const std::string& canonicalName,
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

template <class SilKitConnectionT>
size_t Participant<SilKitConnectionT>::GetNumberOfConnectedParticipants()
{
    return _connection.GetNumberOfConnectedParticipants();
}

template <class SilKitConnectionT>
size_t Participant<SilKitConnectionT>::GetNumberOfRemoteReceivers(const IServiceEndpoint* service,
                                                                  const std::string& msgTypeName)
{
    return _connection.GetNumberOfRemoteReceivers(service, msgTypeName);
}

template <class SilKitConnectionT>
std::vector<std::string> Participant<SilKitConnectionT>::GetParticipantNamesOfRemoteReceivers(
    const IServiceEndpoint* service, const std::string& msgTypeName)
{
    return _connection.GetParticipantNamesOfRemoteReceivers(service, msgTypeName);
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::NotifyShutdown()
{
    _connection.NotifyShutdown();
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::EvaluateAggregationInfo(bool isSyncSimStepHandler)
{
    // tell connection, if aggregation should be done or not
    // determine by information from TimeSyncService AND information from ParticipantConfig

    switch (_participantConfig.experimental.timeSynchronization.enableMessageAggregation)
    {
    case SilKit::Config::v1::Aggregation::Off:
        // nothing to do
        break;
    case SilKit::Config::v1::Aggregation::On:
        // aggregate in both blocking and non-blocking case
        _connection.EnableAggregation();
        break;
    case SilKit::Config::v1::Aggregation::Auto:
        // aggregate in blocking case only
        if (isSyncSimStepHandler)
            _connection.EnableAggregation();
        break;
    default:
        throw SilKitError{"Unknown aggregation type."};
    }
}

template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::RegisterReplayController(SilKit::Tracing::IReplayDataController* replayController,
                                                              const std::string& controllerName,
                                                              const SilKit::Config::SimulatedNetwork& simulatedNetwork)
{
    if (!_replayScheduler)
    {
        return;
    }
    if (simulatedNetwork.replay.direction != SilKit::Config::Replay::Direction::Undefined)
    {
        _replayScheduler->ConfigureController(controllerName, replayController, simulatedNetwork.replay,
                                              simulatedNetwork.name, simulatedNetwork.type);
    }
}

template <class SilKitConnectionT>
bool Participant<SilKitConnectionT>::ParticipantHasCapability(const std::string& participantName,
                                                              const std::string& capability) const
{
    return _connection.ParticipantHasCapability(participantName, capability);
}

template <class SilKitConnectionT>
std::string Participant<SilKitConnectionT>::GetServiceDescriptorString(
    SilKit::Experimental::NetworkSimulation::ControllerDescriptor controllerDescriptor)
{
    if (!_networkSimulatorInternal)
    {
        Logging::Warn(GetLogger(), "GetServiceDescriptorString was queried, but no network simulator exists.");
        return "";
    }
    return _networkSimulatorInternal->GetServiceDescriptorString(controllerDescriptor);
}


template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::GetMetricsProcessor() -> IMetricsProcessor*
{
    return _metricsProcessor.get();
}


template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::GetMetricsSender() -> VSilKit::IMetricsSender*
{
    return _metricsSender;
}


template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::GetOrCreateMetricsSender() -> VSilKit::IMetricsSender*
{
    if (_metricsSender == nullptr)
    {
        bool hasRemoteSinks{false};
        for (const auto& config : _participantConfig.experimental.metrics.sinks)
        {
            hasRemoteSinks = hasRemoteSinks || (config.type == Config::MetricsSink::Type::Remote);
        }

        if (hasRemoteSinks)
        {
            auto* sender = GetController<IMsgForMetricsSender>(SilKit::Core::Discovery::controllerTypeMetricsSender);

            if (sender == nullptr)
            {
                Core::SupplementalData supplementalData;
                supplementalData[SilKit::Core::Discovery::controllerType] =
                    SilKit::Core::Discovery::controllerTypeMetricsSender;

                Config::InternalController config;
                config.name = SilKit::Core::Discovery::controllerTypeMetricsSender;
                config.network = "default";

                sender = CreateController<VSilKit::MetricsSender>(config, std::move(supplementalData), true, true);
            }

            _metricsSender = static_cast<IMetricsSender*>(static_cast<VSilKit::MetricsSender*>(sender));
        }
        else
        {
            _logger->Debug("Refusing to create MetricsSender because no remote sinks are configured");
        }
    }

    return _metricsSender;
}


template <class SilKitConnectionT>
void Participant<SilKitConnectionT>::CreateSystemInformationMetrics()
{
    auto ee = VSilKit::GetExecutionEnvironment();

    GetMetricsManager()->GetStringList("SilKit/System/OperatingSystem")->Add(ee.operatingSystem);
    GetMetricsManager()->GetStringList("SilKit/System/Hostname")->Add(ee.hostname);
    GetMetricsManager()->GetStringList("SilKit/System/PageSize")->Add(ee.pageSize);
    GetMetricsManager()->GetStringList("SilKit/System/ProcessorCount")->Add(ee.processorCount);
    GetMetricsManager()->GetStringList("SilKit/System/ProcessorArchitecture")->Add(ee.processorArchitecture);
    GetMetricsManager()->GetStringList("SilKit/System/PhysicalMemory")->Add(ee.physicalMemoryMiB + " MiB");
    GetMetricsManager()->GetStringList("SilKit/Process/Executable")->Add(ee.executable);
    GetMetricsManager()->GetStringList("SilKit/Process/Username")->Add(ee.username);
}


template <class SilKitConnectionT>
auto Participant<SilKitConnectionT>::MakeTimerThread() -> std::unique_ptr<IMetricsTimerThread>
{
    if (_participantConfig.experimental.metrics.sinks.empty())
    {
        return nullptr;
    }

    return std::make_unique<VSilKit::MetricsTimerThread>(
        [this] { ExecuteDeferred([this] { GetMetricsManager()->SubmitUpdates(); }); });
}


} // namespace Core
} // namespace SilKit
