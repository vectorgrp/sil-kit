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
namespace Services {
namespace Can {
class IMsgForCanSimulator;
} // namespace Can
namespace Ethernet {
class IMsgForEthSimulator;
} // namespace Ethernet
namespace Flexray {
class IMsgForFlexrayBusSimulator;
} // namespace Flexray
namespace Lin {
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
class LifecycleService;
class TimeSyncService;
} // namespace Orchestration
} // namespace Services
namespace Core {
namespace Discovery {
class IServiceDiscovery;
} // namespace Discovery
namespace RequestReply {
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

