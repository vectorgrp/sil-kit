// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <functional>

#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/fwd_decl.hpp"

#include "ib/sim/data/DataMessageDatatypes.hpp"
#include "ib/sim/rpc/RpcDatatypes.hpp"

namespace ib {
namespace mw {

/*! \brief Communication interface to be used by IB participants
 *
 */
class IParticipant
{
public:
    virtual ~IParticipant() = default;

    /* Methods Create*Controller() create controllers at this IB participant.
     *
     * Controllers provide an easy interface to interact with a simulated bus. They
     * act as a proxy to the controller implementation in a Network Simulator connected
     * to the Integration Bus.
     *
     * Each Create*Controller() method creates a proxy instance, sets up all
     * necessary data structures and establishes the connection according to the
     * underlying middleware.
     */

    //! \brief Create a CAN controller at this IB participant.
    virtual auto CreateCanController(const std::string& canonicalName, const std::string& networkName)
        -> sim::can::ICanController* = 0;
    //! \brief Create a CAN controller at this IB participant.
    virtual auto CreateCanController(const std::string& canonicalName) -> sim::can::ICanController* = 0;
    //! \brief Create an Ethernet controller at this IB participant.

    virtual auto CreateEthernetController(const std::string& canonicalName, const std::string& networkName)
        -> sim::eth::IEthernetController* = 0;
    //! \brief Create an Ethernet controller at this IB participant.
    virtual auto CreateEthernetController(const std::string& canonicalName) -> sim::eth::IEthernetController* = 0;
    //! \brief Create an FlexRay controller at this IB participant.

    virtual auto CreateFlexrayController(const std::string& canonicalName, const std::string& networkName)
        -> sim::fr::IFlexrayController* = 0;
    //! \brief Create an FlexRay controller at this IB participant.
    virtual auto CreateFlexrayController(const std::string& canonicalName) -> sim::fr::IFlexrayController* = 0;
    //! \brief Create a LIN controller at this IB participant.

    virtual auto CreateLinController(const std::string& canonicalName, const std::string& networkName)
        -> sim::lin::ILinController* = 0;
    //! \brief Create a LIN controller at this IB participant.
    virtual auto CreateLinController(const std::string& canonicalName) -> sim::lin::ILinController* = 0;

    //! \brief Create a data publisher at this IB participant.
    virtual auto CreateDataPublisher(const std::string& canonicalName, const std::string& topic,
                                     const std::string& mediaType,
                                     const std::map<std::string, std::string>& labels, size_t history = 0)
        -> sim::data::IDataPublisher* = 0;
    //! \brief Create a data publisher at this IB participant.
    virtual auto CreateDataPublisher(const std::string& canonicalName) -> sim::data::IDataPublisher* = 0;

    //! \brief Create a data subscriber at this IB participant.
    virtual auto CreateDataSubscriber(const std::string& canonicalName, const std::string& topic,
                                      const std::string& mediaType,
                                      const std::map<std::string, std::string>& labels,
                                      sim::data::DataMessageHandlerT defaultDataMessageHandler,
                                      sim::data::NewDataPublisherHandlerT newDataPublisherHandler = nullptr)
        -> sim::data::IDataSubscriber* = 0;
    //! \brief Create a data subscriber at this IB participant.
    virtual auto CreateDataSubscriber(const std::string& canonicalName) -> sim::data::IDataSubscriber* = 0;

    //! \brief Create a Rpc client at this IB participant.
    virtual auto CreateRpcClient(const std::string& canonicalName, const std::string& functionName,
                                 const std::string& mediaType, const std::map<std::string, std::string>& labels,
                                 sim::rpc::RpcCallResultHandler handler) -> sim::rpc::IRpcClient* = 0;
    //! \brief Create a Rpc client at this IB participant.
    virtual auto CreateRpcClient(const std::string& canonicalName) -> sim::rpc::IRpcClient* = 0;

    //! \brief Create a Rpc server at this IB participant.
    virtual auto CreateRpcServer(const std::string& canonicalName, const std::string& functionName,
                                 const std::string& mediaType, const std::map<std::string, std::string>& labels,
                                 sim::rpc::RpcCallHandler handler) -> sim::rpc::IRpcServer* = 0;
    //! \brief Create a Rpc server at this IB participant.
    virtual auto CreateRpcServer(const std::string& canonicalName) -> sim::rpc::IRpcServer* = 0;

    //! \brief Discover available Rpc servers and their properties.
    virtual void DiscoverRpcServers(const std::string& functionName, const std::string& mediaType,
                                    const std::map<std::string, std::string>& labels,
                                    sim::rpc::RpcDiscoveryResultHandler handler) = 0;

    //! \brief Return the  ILifecycleService for the current participant.
    virtual auto GetLifecycleService() -> sync::ILifecycleService* = 0;
    virtual auto GetSystemMonitor() -> sync::ISystemMonitor* = 0;
    virtual auto GetSystemController() -> sync::ISystemController* = 0;
    virtual auto GetLogger() -> logging::ILogger* = 0;
};

} // mw
} // namespace ib
