// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <functional>

#include "silkit/core/fwd_decl.hpp"
#include "silkit/services/fwd_decl.hpp"

#include "silkit/services/pubsub/DataMessageDatatypes.hpp"
#include "silkit/services/rpc/RpcDatatypes.hpp"

namespace SilKit {
namespace Core {

/*! \brief Communication interface to be used by SilKit participants
 *
 */
class IParticipant
{
public:
    virtual ~IParticipant() = default;

    /* Methods Create*Controller() create controllers at this SilKit participant.
     *
     * Controllers provide an easy interface to interact with a simulated bus. They
     * act as a proxy to the controller implementation if a network simulator is connected
     * to the SilKit.
     *
     * Each Create*Controller() method creates a proxy instance, sets up all
     * necessary data structures and establishes the connection according to the
     * underlying middleware.
     */

    //! \brief Create a CAN controller at this SilKit participant.
    virtual auto CreateCanController(const std::string& canonicalName, const std::string& networkName)
        -> Services::Can::ICanController* = 0;
    //! \brief Create a CAN controller at this SilKit participant.
    virtual auto CreateCanController(const std::string& canonicalName) -> Services::Can::ICanController* = 0;
    //! \brief Create an Ethernet controller at this SilKit participant.

    virtual auto CreateEthernetController(const std::string& canonicalName, const std::string& networkName)
        -> Services::Ethernet::IEthernetController* = 0;
    //! \brief Create an Ethernet controller at this SilKit participant.
    virtual auto CreateEthernetController(const std::string& canonicalName) -> Services::Ethernet::IEthernetController* = 0;
    //! \brief Create an FlexRay controller at this SilKit participant.

    virtual auto CreateFlexrayController(const std::string& canonicalName, const std::string& networkName)
        -> Services::Flexray::IFlexrayController* = 0;
    //! \brief Create an FlexRay controller at this SilKit participant.
    virtual auto CreateFlexrayController(const std::string& canonicalName) -> Services::Flexray::IFlexrayController* = 0;
    //! \brief Create a LIN controller at this SilKit participant.

    virtual auto CreateLinController(const std::string& canonicalName, const std::string& networkName)
        -> Services::Lin::ILinController* = 0;
    //! \brief Create a LIN controller at this SilKit participant.
    virtual auto CreateLinController(const std::string& canonicalName) -> Services::Lin::ILinController* = 0;

    //! \brief Create a data publisher at this SilKit participant.
    virtual auto CreateDataPublisher(const std::string& canonicalName, const std::string& topic,
                                     const std::string& mediaType,
                                     const std::map<std::string, std::string>& labels, size_t history = 0)
        -> Services::PubSub::IDataPublisher* = 0;
    //! \brief Create a data publisher at this SilKit participant.
    virtual auto CreateDataPublisher(const std::string& canonicalName) -> Services::PubSub::IDataPublisher* = 0;

    //! \brief Create a data subscriber at this SilKit participant.
    virtual auto CreateDataSubscriber(const std::string& canonicalName, const std::string& topic,
                                      const std::string& mediaType,
                                      const std::map<std::string, std::string>& labels,
                                      Services::PubSub::DataMessageHandlerT defaultDataMessageHandler,
                                      Services::PubSub::NewDataPublisherHandlerT newDataPublisherHandler = nullptr)
        -> Services::PubSub::IDataSubscriber* = 0;
    //! \brief Create a data subscriber at this SilKit participant.
    virtual auto CreateDataSubscriber(const std::string& canonicalName) -> Services::PubSub::IDataSubscriber* = 0;

    //! \brief Create a Rpc client at this SilKit participant.
    virtual auto CreateRpcClient(const std::string& canonicalName, const std::string& functionName,
                                 const std::string& mediaType, const std::map<std::string, std::string>& labels,
                                 Services::Rpc::RpcCallResultHandler handler) -> Services::Rpc::IRpcClient* = 0;
    //! \brief Create a Rpc client at this SilKit participant.
    virtual auto CreateRpcClient(const std::string& canonicalName) -> Services::Rpc::IRpcClient* = 0;

    //! \brief Create a Rpc server at this SilKit participant.
    virtual auto CreateRpcServer(const std::string& canonicalName, const std::string& functionName,
                                 const std::string& mediaType, const std::map<std::string, std::string>& labels,
                                 Services::Rpc::RpcCallHandler handler) -> Services::Rpc::IRpcServer* = 0;
    //! \brief Create a Rpc server at this SilKit participant.
    virtual auto CreateRpcServer(const std::string& canonicalName) -> Services::Rpc::IRpcServer* = 0;

    //! \brief Discover available Rpc servers and their properties.
    virtual void DiscoverRpcServers(const std::string& functionName, const std::string& mediaType,
                                    const std::map<std::string, std::string>& labels,
                                    Services::Rpc::RpcDiscoveryResultHandler handler) = 0;

    //! \brief Return the ILifecycleService for the current participant.
    virtual auto GetLifecycleService() -> Orchestration::ILifecycleService* = 0;
    virtual auto GetSystemMonitor() -> Orchestration::ISystemMonitor* = 0;
    virtual auto GetSystemController() -> Orchestration::ISystemController* = 0;
    virtual auto GetLogger() -> Logging::ILogger* = 0;
};

} // namespace Core
} // namespace SilKit
