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

#pragma once

namespace SilKit {
//forwards
namespace Config {
inline namespace v1 {
struct CanController;
struct EthernetController;
struct LinController;
struct FlexrayController;
struct RpcClient;
struct RpcServer;
struct DataPublisher;
struct DataSubscriber;
} //v1
}//Config

namespace Core {
namespace Discovery {
class ServiceDiscovery;
}//Discovery
namespace RequestReply {
class RequestReplyService;
} //RequestReply
}//Core

namespace Services {
namespace Orchestration {
class TimeSyncService;
class LifecycleService;
class SystemMonitor;
class SystemController;
}//Orchestration
namespace Logging {
class LogMsgReceiver;
class LogMsgSender;
} //Logging
namespace Ethernet {
class EthController;
} //Can
namespace Can {
class CanController;
} //Can
namespace Lin {
class LinController;
} //Lin
namespace Flexray {
class FlexrayController;
} //Flexray

namespace PubSub {
class DataPublisher;
class DataSubscriber  ;
class DataSubscriberInternal;
} //PubSub
namespace Rpc {
class RpcServer;
class RpcClient  ;
class RpcServerInternal;
} //Rpc

} //Services
} // namespace SilKit
