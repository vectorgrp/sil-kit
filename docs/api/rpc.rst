=================================
RPC (Remote Procedure Call) API
=================================

.. Macros for docs use
.. |IParticipant| replace:: :cpp:class:`IParticipant<SilKit::IParticipant>`
.. |CreateRpcClient| replace:: :cpp:func:`CreateRpcClient()<SilKit::IParticipant::CreateRpcClient()>`
.. |CreateRpcServer| replace:: :cpp:func:`CreateRpcServer()<SilKit::IParticipant::CreateRpcServer()>`
.. |Call| replace:: :cpp:func:`Call()<SilKit::Services::Rpc::IRpcClient::Call()>`
.. |CallWithTimeout| replace:: :cpp:func:`CallWithTimeout()<SilKit::Services::Rpc::IRpcClient::CallWithTimeout()>`
.. |SubmitResult| replace:: :cpp:func:`SubmitResult()<SilKit::Services::Rpc::IRpcServer::SubmitResult()>`
.. |SetCallHandler| replace:: :cpp:func:`SetRpcHandler()<SilKit::Services::Rpc::IRpcServer::SetCallHandler()>`
.. |SetCallResultHandler| replace:: :cpp:func:`SetCallReturnHandler()<SilKit::Services::Rpc::IRpcClient::SetCallResultHandler()>`

.. |RpcSpec| replace:: :cpp:class:`RpcSpec<SilKit::Services::Rpc::RpcSpec>`
.. |AddLabel| replace:: :cpp:func:`AddLabel()<SilKit::Services::Rpc::RpcSpec::AddLabel>`
.. |MatchingLabel| replace:: :cpp:class:`MatchingLabel<SilKit::Services::MatchingLabel>`

.. |IRpcClient| replace:: :cpp:class:`IRpcClient<SilKit::Services::Rpc::IRpcClient>`
.. |IRpcServer| replace:: :cpp:class:`IRpcClient<SilKit::Services::Rpc::IRpcServer>`

.. |MediaTypeRpc| replace:: :cpp:func:`MediaTypeRpc()<SilKit::Util::SerDes::MediaTypeRpc()>`
.. contents::
   :local:
   :depth: 3

Using the RPC API
-----------------

This API provides a client-server model for remote calls with arbitrary argument- and return data.
The ``RpcClient`` and ``RpcServer`` interfaces are instantiated from an |IParticipant| interface by calling 
|CreateRpcClient| and |CreateRpcServer|, respectively.

The ``RpcClient`` can trigger a call using the |Call| method providing argument data as a vector of bytes.
The |Call| method is non-blocking and allows for later identification of the call through an
additional user context pointer passed as an optional second argument.
The call arrives at the ``RpcServer`` and is delivered via a callback, which has to be specified on
creation of the ``RpcServer`` and can be overwritten using the |SetCallHandler| method.
There, the argument data and call handle arrive and can be processed.
The ``RpcServer`` must submit the answer to the call at a later point in time with the call handle
obtained in the ``RpcCallHandler`` by using the |SubmitResult| method providing the return data for the
calling ``RpcClient``.
The ``RpcClient`` receives the call return in a callback which is also specified on creation and can
be overwritten with |SetCallResultHandler|.
The callback provides the user context pointer passed to |Call|, the return data and a call status indicating
success or an error during the procedure.
Additionally, |CallWithTimeout| can be used to trigger calls that have to be replied to within a specified 
timeout duration.
Otherwise the call will lead to a timeout RpcCallResultEvent.

Argument and Return Data
========================

In theory, users can freely decide on how to de/serialize argument and return data into and out of a byte vector.
However, it is strongly recommended to use SIL Kit's serialization schema implemented by the :doc:`Data Serialization/Deserialization API</api/serdes>` 
to ensure compatibility among all SIL Kit participants.

Function Name
~~~~~~~~~~~~~

RPC clients and RPC servers provide a function name which is part of their |RpcSpec|.

Communication only takes place among RPC clients and RPC servers with the same function name.

Media Type
~~~~~~~~~~

Both RPC clients and RPC servers define a media type as part of their |RpcSpec|. It is a meta description
of the transmitted data in accordance to `RFC2046 <https://datatracker.ietf.org/doc/html/rfc2046>`_ and should be used
to provide information about the de-/serialization of the underlying user data. Just like the topic, the media type 
has to match between RPC clients / RPC servers for communication to take place. An empty string on an RPC client will 
match any other media type on a server.

When data is serialized using SIL Kit's :doc:`Data Serialization/Deserialization API</api/serdes>`, media type constant |MediaTypeRpc| 
must be used.

Labels
~~~~~~

Both RPC clients and RPC servers can be annotated with string-based key-value pairs (labels) which can be either
mandatory or optional. In addition to the matching requirements given by topic and media type, RPC clients and 
RPC servers will only communicate if their labels conform to the following matching rules:

* A mandatory label matches, if a label of the same key and value is found on the corresponding counterpart.
* An optional label matches, if the label key does not exist on the counterpart or both its key and value are equal.

The following table shows how RPC clients and RPC servers with matching topics and matching media type would 
match corresponding to their labels. Note that the label matching is symmetric, so clients and servers
are interchangeable here.

.. list-table:: Label combinations
   :header-rows: 1

   * - 
     - Server {"KeyA", "Val1", Optional}
     - Server {"KeyA", "Val1", Mandatory}
   * - Client {}
     - Match
     - No Match
   * - Client {"KeyA", "Val1", Optional}
     - Match
     - Match
   * - Client {"KeyA", "Val2", Optional}
     - No Match
     - No Match
   * - Client {"KeyB", "Val1", Optional}
     - Match
     - No Match
   * - Client {"KeyB", "Val1", Mandatory}
     - No Match
     - No Match

The labels are stored in the |RpcSpec|. A |MatchingLabel| can be added via |AddLabel|,
see the following code snippet:

.. code-block:: cpp

    SilKit::Services::Rpc::RpcSpec clientSpec{"Topic1", "application/json"};
    clientSpec.AddLabel("KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Optional);
    auto* client = participant->CreateRpcClient("ClientCtrl1", clientSpec, callResultHandler);

Error handling
~~~~~~~~~~~~~~

* If using |Call| with no corresponding server available, the ``CallReturnHandler`` is triggered immediately with
  ``RpcCallStatus::ServerNotReachable``.
* |SubmitResult| must only be used with a valid call handle received in the ``RpcHandler``.
* The ``RpcCallResultEvent::resultData`` member is only valid if ``callStatus == RpcCallStatus::Success``.
* If the RPC server receives a call but does not have a valid call handler, the RPC client will receive an
  ``RpcCallResultEvent`` with ``callStatus == RpcCallStatus::InternalServerError``.
* If the RpcServer does not reply within the specified timeout of |CallWithTimeout|, the CallReturnHandler is triggered 
  immediately with ``RpcCallStatus::Timeout``.

Usage Example
~~~~~~~~~~~~~

The interfaces for the RPC mechanism can be instantiated from an |IParticipant|:

.. code-block:: cpp

    // ------------------
    // Client participant
    // ------------------

    auto participant = SilKit::CreateParticipant(std::move(config), participant_name, registryUri);

    SilKit::Services::Rpc::RpcSpec dataSpecClient{"TestFunc", "application/octet-stream"};
    auto client = participant->CreateRpcClient("ClientCtrl1", dataSpecClient, 
        [](IRpcClient* client, RpcCallResultEvent event) {
            // handle event.callStatus and/or event.resultData
        });
    );

    // define argumentData
    client->Call(argumentData);

    // define userContext (void *)
    client->Call(argumentData, userContext);

    // ------------------
    // Server participant
    // ------------------

    auto participant = SilKit::CreateParticipant(std::move(config), participant_name, registryUri);
    SilKit::Services::Rpc::RpcSpec dataSpecServer{"TestFunc", "application/octet-stream"};
            
    auto* server = participant->CreateRpcServer("ServerCtrl1", dataSpecServer, 
        [](IRpcServer* server, RpcCallEvent event) {
            // handle argumentData
            // define resultData
            server->SubmitResult(event.callHandle, resultData)
        });

RpcClient API
~~~~~~~~~~~~~

.. doxygenclass:: SilKit::Services::Rpc::IRpcClient
   :members:

RpcServers API
~~~~~~~~~~~~~~

.. doxygenclass:: SilKit::Services::Rpc::IRpcServer
   :members:


Data Structures
~~~~~~~~~~~~~~~

.. doxygenstruct:: SilKit::Services::Rpc::RpcCallEvent
   :members:

.. doxygenstruct:: SilKit::Services::Rpc::RpcCallResultEvent
   :members:

.. doxygenclass:: SilKit::Services::Rpc::RpcSpec
   :members:

Enumerations
~~~~~~~~~~~~

.. doxygenenum:: SilKit::Services::Rpc::RpcCallStatus