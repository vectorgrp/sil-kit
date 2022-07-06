#pragma once
#include <string>

namespace SilKit {
namespace Core {
namespace Discovery {

// controller-specific keys
const std::string controllerType = "controller.type";

// simulatedController-specific keys
// remark: simulated controllers have all attributes of regular controllers in addition to the following
const std::string simulatedControllerOriginalParticipantName =
    "controller.netsim.originalParticipantName";

// controllerType-specific values
// Bus types
const std::string controllerTypeCan = "CAN";
const std::string controllerTypeEthernet = "Ethernet";
const std::string controllerTypeFlexray = "FlexRay";
const std::string controllerTypeLin = "LIN";

// PubSub types and supplementalData keys
const std::string controllerTypeDataPublisher = "DataPublisher";
const std::string supplKeyDataPublisherTopic = "PubSub::topic";
const std::string supplKeyDataPublisherPubUUID = "PubSub::pubUUID";
const std::string supplKeyDataPublisherMediaType = "PubSub::pubMediaType";
const std::string supplKeyDataPublisherPubLabels = "PubSub::pubLabels";

const std::string controllerTypeDataSubscriber = "DataSubscriber";
const std::string controllerTypeDataSubscriberInternal = "DataSubscriberInternal";

// RPC types
const std::string controllerTypeRpcServer = "RpcServer";
const std::string supplKeyRpcServerFunctionName = "Rpc::server::functionName";
const std::string supplKeyRpcServerMediaType = "Rpc::server::mediaType";
const std::string supplKeyRpcServerLabels = "Rpc::server::labels";

const std::string controllerTypeRpcClient = "RpcClient";
const std::string supplKeyRpcClientFunctionName = "Rpc::client::functionName";
const std::string supplKeyRpcClientMediaType = "Rpc::client::mediaType";
const std::string supplKeyRpcClientLabels = "Rpc::client::labels";
const std::string supplKeyRpcClientUUID = "Rpc::client::UUID";

const std::string controllerTypeRpcServerInternal = "RpcServerInternal";
const std::string supplKeyRpcServerInternalClientUUID = "Rpc::serverinternal::clientUUID";

// Internal types
const std::string controllerTypeLoggerSender = "LoggerSender";
const std::string controllerTypeLoggerReceiver = "LoggerReceiver";
const std::string controllerTypeServiceDiscovery = "ServiceDiscovery";
const std::string controllerTypeSystemMonitor = "SystemMonitor";
const std::string controllerTypeSystemController = "SystemController";
const std::string controllerTypeLifecycleService = "LifecycleService";
const std::string controllerTypeTimeSyncService = "TimeSyncService";

// misc / legacy controllers
const std::string controllerTypeOther = "Other";

// Lifecycle & TimeSync
const std::string lifecycleHasCoordinatedStart = "LifecycleHasCoordinatedStart";
const std::string lifecycleHasCoordinatedStop = "LifecycleHasCoordinatedStop";
const std::string timeSyncActive = "TimeSyncActive";

} // namespace Discovery
} // namespace Core
} // namespace SilKit