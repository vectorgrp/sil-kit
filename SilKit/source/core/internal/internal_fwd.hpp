// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


namespace VSilKit {
class SystemStateTracker;
class ConnectPeer;
class MetricsProcessor;
class AsioGenericRawByteStream;
} // namespace VSilKit
namespace SilKit {
namespace Tracing {
class PcapSink;
class ReplayScheduler;
} // namespace Tracing
 namespace Dashboard {
class DashboardRestClient;
class DashboardSystemServiceClient;
class DashboardInstance;
} // namespace Dashboard
namespace Experimental {
namespace NetworkSimulation {
class NetworkSimulatorInternal;
class SimulatedNetworkInternal;
class SimulatedNetworkRouter;
} // namespace NetworkSimulation
} // namespace Experimental
namespace Services {
namespace Can {
class SimBehaviorTrivial;
class CanController;
class IMsgForCanSimulator;
} // namespace Can
namespace Ethernet {
class EthController;
class IMsgForEthSimulator;
} // namespace Ethernet
namespace Flexray {
class FlexrayController;
class IMsgForFlexrayBusSimulator;
} // namespace Flexray
namespace Lin {
class SimBehaviorTrivial;
class LinController;
class IMsgForLinSimulator;
} // namespace Lin
namespace PubSub {
class IMsgForDataPublisher;
class IMsgForDataSubscriber;
class IMsgForDataSubscriberInternal;
class DataSubscriberInternal;
} // namespace PubSub
namespace Rpc {
class IMsgForRpcClient;
class IMsgForRpcServer;
class IMsgForRpcServerInternal;
class RpcServerInternal;
class RpcDiscoverer;
} // namespace Rpc
namespace Orchestration {
class TimeConfiguration;
class SystemMonitor;
class LifecycleManagement;
class LifecycleService;
class TimeSyncService;
} // namespace Orchestration
} // namespace Services
namespace Core {
template <class MsgT>
class SilKitLink; 
class IParticipantInternal;
class ConnectKnownParticipants;
class RemoteConnectionManager;
class VAsioConnection;
class VAsioRegistry;
class VAsioPeer;
template <class SilKitConnectionT>
class Participant;
namespace Discovery {
class IServiceDiscovery;
} // namespace Discovery
namespace RequestReply {
class ParticipantReplies;
class IRequestReplyService;
class IParticipantReplies;
} // namespace RequestReply
} // namespace Core
namespace Experimental {
namespace NetworkSimulation {
class INetworkSimulator;
} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit
