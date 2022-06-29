// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

namespace ib {
namespace sim {
namespace can {
class IIbToCanSimulator;
} // namespace can
namespace eth {
class IIbToEthSimulator;
} // namespace eth
namespace fr {
class IIbToFlexrayBusSimulator;
} // namespace fr
namespace lin {
class IIbToLinSimulator;
} // namespace lin
namespace data {
class IIbToDataPublisher;
class IIbToDataSubscriber;
class IIbToDataSubscriberInternal;
class DataSubscriberInternal;
} // namespace data
namespace rpc {
class IIbToRpcClient;
class IIbToRpcServer;
class IIbToRpcServerInternal;
class RpcServerInternal;
class RpcDiscoverer;
} // namespace rpc
} // namespace sim
namespace mw {
namespace service {
class IServiceDiscovery;
} // namespace service
namespace sync {
class LifecycleService;
class TimeSyncService;
} // namespace sync
} // namespace mw
} // namespace ib

