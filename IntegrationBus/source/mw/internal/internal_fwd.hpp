// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
namespace ib {
namespace sim {

namespace can {
class IIbToCanSimulator;
} // can
namespace eth {
class IIbToEthSimulator;
} // eth
namespace fr {
class IIbToFlexrayBusSimulator;
} // fr
namespace lin {
class IIbToLinSimulator;
} // lin
namespace data {
class IIbToDataPublisher;
class IIbToDataSubscriber;
class IIbToDataSubscriberInternal;
class DataSubscriberInternal;
} // data
namespace rpc {
class IIbToRpcClient;
class IIbToRpcServer;
class IIbToRpcServerInternal;
class RpcServerInternal;
class RpcDiscoverer;
} // rpc
} // sim
namespace mw {
namespace service {
class IServiceDiscovery;
} //service
namespace sync {
class LifecycleService;
class TimeSyncService;
} //sync
} //mw
} // namespace ib

