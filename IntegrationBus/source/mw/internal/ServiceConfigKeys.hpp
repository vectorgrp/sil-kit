#pragma once
#include <string>
#include "ib/cfg/Config.hpp"

namespace ib {
namespace mw {
namespace service {

// controller-specific keys
const std::string controllerType = "controller.type";
const std::string controllerIsSynchronized = "controller.isSynchronized"; // for participantController

// simulatedController-specific keys
// remark: simulated controllers have all attributes of regular controllers in addition to the following
const std::string simulatedControllerOriginalParticipantName =
    "controller.netsim.originalParticipantName";


// controllerType-specific values
// Bus types
const std::string controllerTypeCan = "CAN";
const std::string controllerTypeEthernet = "Ethernet";
const std::string controllerTypeFlexRay = "FlexRay";
const std::string controllerTypeLin = "LIN";

// PubSub types and supplementalData keys
const std::string controllerTypeDataPublisher = "DataPublisher";
const std::string supplKeyDataPublisherTopic = "data::topic";
const std::string supplKeyDataPublisherPubUUID = "data::pubUUID";
const std::string supplKeyDataPublisherPubDxf = "data::pubDxf";
const std::string supplKeyDataPublisherPubLabels = "data::pubLabels";

const std::string controllerTypeDataSubscriber = "DataSubscriber";
const std::string controllerTypeDataSubscriberInternal = "DataSubscriberInternal";

// RPC types
const std::string controllerTypeRpcServer = "RpcServer";
const std::string supplKeyRpcServerFunctionName = "rpc::server::functionName";
const std::string supplKeyRpcServerDxf = "rpc::server::dxf";
const std::string supplKeyRpcServerLabels = "rpc::server::labels";

const std::string controllerTypeRpcClient = "RpcClient";
const std::string supplKeyRpcClientFunctionName = "rpc::client::functionName";
const std::string supplKeyRpcClientDxf = "rpc::client::dxf";
const std::string supplKeyRpcClientLabels = "rpc::client::labels";
const std::string supplKeyRpcClientUUID = "rpc::client::UUID";

const std::string controllerTypeRpcServerInternal = "RpcServerInternal";
const std::string supplKeyRpcServerInternalClientUUID = "rpc::serverinternal::clientUUID";

// Internal types
const std::string controllerTypeParticipantController = "ParticipantController";
const std::string controllerTypeLoggerSender = "LoggerSender";
const std::string controllerTypeLoggerReceiver = "LoggerReceiver";
const std::string controllerTypeServiceDiscovery = "ServiceDiscovery";
const std::string controllerTypeSystemMonitor = "SystemMonitor";
const std::string controllerTypeSystemController = "SystemController";

// misc / legacy controllers
const std::string controllerTypeOther = "Other";

} // namespace service
} // namespace mw
} // namespace ib