// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

namespace SilKit {
//forwards
namespace Config {
inline namespace V1 {
struct CanController;
struct EthernetController;
struct LinController;
struct FlexrayController;
struct RpcClient;
struct RpcServer;
struct DataPublisher;
struct DataSubscriber;
} // namespace V1
} // namespace Config

namespace Core {
namespace Discovery {
class ServiceDiscovery;
} // namespace Discovery
namespace RequestReply {
class RequestReplyService;
} // namespace RequestReply
} // namespace Core

namespace Services {
namespace Orchestration {
class TimeSyncService;
class LifecycleService;
class SystemMonitor;
class SystemController;
} // namespace Orchestration
namespace Logging {
class LogMsgReceiver;
class LogMsgSender;
} // namespace Logging
namespace Ethernet {
class EthController;
} // namespace Ethernet
namespace Can {
class CanController;
} // namespace Can
namespace Lin {
class LinController;
} // namespace Lin
namespace Flexray {
class FlexrayController;
} // namespace Flexray

namespace PubSub {
class DataPublisher;
class DataSubscriber;
class DataSubscriberInternal;
} // namespace PubSub
namespace Rpc {
class RpcServer;
class RpcClient;
class RpcServerInternal;
} // namespace Rpc

} // namespace Services
} // namespace SilKit

namespace VSilKit {
class MetricsManager;
class MetricsSender;
} // namespace VSilKit
