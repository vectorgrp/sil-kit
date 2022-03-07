===========================
!!! Remote procedure call (Rpc)
===========================

.. Macros for docs use
.. |IComAdapter| replace:: :cpp:class:`IComAdapter<ib::mw::IComAdapter>`
.. |CreateRpcClient| replace:: :cpp:func:`CreateRpcClient<ib::mw::IComAdapter::CreateRpcClient()>`
.. |CreateRpcServer| replace:: :cpp:func:`CreateRpcServer<ib::mw::IComAdapter::CreateRpcServer()>`
.. |Call| replace:: :cpp:func:`Call()<ib::sim::rpc::IRpcClient::Call()>`
.. |SubmitResult| replace:: :cpp:func:`SubmitResult()<ib::sim::rpc::IRpcServer::SubmitResult()>`
.. |SetRpcHandler| replace:: :cpp:func:`SetRpcHandler()<ib::sim::rpc::IRpcServer::SetRpcHandler()>`
.. |SetCallReturnHandler| replace:: :cpp:func:`SetCallReturnHandler()<ib::sim::rpc::IRpcClient::SetCallReturnHandler()>`
.. |DiscoverRpcServers| replace:: :cpp:func:`DiscoverRpcServers()<ib::mw::IComAdapter::DiscoverRpcServers()>`
.. |IRpcClient| replace:: :cpp:class:`IRpcClient<ib::sim::rpc::IRpcClient>`
.. |IRpcServer| replace:: :cpp:class:`IRpcClient<ib::sim::rpc::IRpcServer>`
.. contents::
   :local:
   :depth: 3

!!! Using the Rpc API
-----------------

This API provides a client-server model for remote calls with arbitrary argument- and return data. 
The RpcClient dispatches the call (1) with given argument data. The call arrives remotely and is processed by 
the handler (2) of the RpcServer, submitting (3) the result back to the RpcClient who gets informed 
about the incoming return data in his call return handler (4). These steps constitute the core Rpc API, where the 
handlers (2,4) are provided on instantiation and call / submit (1,3) are commands of the RpcClient / RpcServer 
instances. Further, a query can be run providing a list of available RpcServers and their properties.

!!! Function name
~~~~~~~~~~~~~

RpcClients and RpcServers are linked by a string-based function name. For each link, the endpoints must be unique. 
That is, on one participant, there can only be one RpcClient / RpcServer on a given function name. However, it is 
possible to use multiple RpcClients / RpcServers on the same function name distributed among different participants.

!!! RpcExchangeFormat
~~~~~~~~~~~~~~~~~

Both RpcClients and RpcServers define a RpcExchangeFormat, a meta description of the transmitted data. It can
be used to provide infomation about the de- / serialization of the underlying user data. Just like the function 
name, the RpcExchangeFormat has to match between RpcClients / RpcServers for communicaiton to take place. 
An empty character on a RpcClient will match any other string of that given field of the RpcExchangeFormat. 
Currently, the RpcExchangeFormat only consists of the field "mediaType".

!!! Labels
~~~~~~

RpcClients and RpcServers can be annotated with string-based key-value pairs (labels). Additional to the matching 
requirements regarding functionName and RpcExchangeFormat, RpcServers will only receive calls by RpcClients if their 
labels apply the following matching rules:

* A RpcClient without labels matches any other RpcServer.
* If labels are specified on a RpcClients, all of the labels must be found on a RpcServer.
* An empty value string on a RpcClients's label is a wildcard.

!!! Server Discovery
~~~~~~~~~~~~~~~~

The simulation can be queried about available RpcServers with |DiscoverRpcServers|. The method takes filter arguments
for functionName, RpcExchangeFormat and labels. To obtain the results of the query, a handler is given to the method 
which carries a vector of RpcDiscoveryResult providing the properties of each discovered RpcServer.

!!! Usage
~~~~~

The RpcClient and RpcServer interfaces are instantiated from an |IComAdapter| interface by calling 
|CreateRpcClient| and |CreateRpcServer|, respectively. The controller name corresponds to the function name and
is used in the configuration and instantiation of the interfaces.

The RpcClient can detach a call using the |Call| method providing argument data as a vector of bytes. The method is
non-blocking and returns a call handle which can be used later for identification. The call arrives at the 
RpcServer and is delivered via a callback, which has to be specified on creation of the RpcServer and can be 
overwritten using the |SetRpcHandler| method. There, the argument data and call handle arrive and can be processed.
The RpcServer can submit the answer to the call at a later point in time with the call handle obtained in the 
RpcHandler by using the |SubmitResult| method providing the return data for the calling RpcClient. 
The RpcClient receives the call return in a callback which is also specified on creation and can be overwritten with
|SetCallReturnHandler|. The callback provides the original call handle, the return data and a call status 
indicating success or an error during the procedure.

!!! Error handling
~~~~~~~~~~~~~~

* If using |Call| with no corresponding server available, the CallReturnHandler is triggered immediately with a nullptr
  call handle and CallStatus::ServerNotReachable. In this case, the call handle returned by |Call| is also nullptr.
* |SubmitResult| must only be used with a valid call handle received in the RpcHandler.

!!! Usage Example
~~~~~~~~~~~~~

The interfaces for the Rpc mechanism can be instantiated from an IComAdapter:

.. code-block:: cpp

    // ------------------
    // Client participant
    // ------------------

    auto comAdapter = ib::CreateComAdapter(std::move(config), participant_name, domainId);
    auto* client = comAdapter->CreateRpcClient("TestFunc", RpcExchangeFormat{"application/octet-stream"}, 
        [](IRpcClient* client, const CallHandle callHandle, const CallStatus callStatus, const std::vector<uint8_t>& returnData) {
            // handle returnData
        });
    );

    // define argumentData
    auto callHandle = client->Call(argumentData)

    // ------------------
    // Server participant
    // ------------------

    auto comAdapter = ib::CreateComAdapter(std::move(config), participant_name, domainId);
    auto* server = comAdapter->CreateRpcServer("TestFunc", RpcExchangeFormat{"application/octet-stream"},
        [](IRpcServer* server, const CallHandle callHandle, const std::vector<uint8_t>& argumentData) {
            // handle argumentData
            // define resultData
            server->SubmitResult(callHandle, resultData)
        });

!!! RpcClient API
~~~~~~~~~~~~~~~~~~

    .. doxygenclass:: ib::sim::rpc::IRpcClient
       :members:

!!! RpcServers API
~~~~~~~~~~~~~~~~~~~

    .. doxygenclass:: ib::sim::rpc::IRpcServer
       :members:

!!! Data Structures
~~~~~~~~~~~~~~~

    .. doxygenstruct:: ib::cfg::RpcPort
       :members:
