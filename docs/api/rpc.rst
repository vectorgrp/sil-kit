.. Macros for docs use

.. |IParticipant| replace:: :cpp:class:`IParticipant<SilKit::IParticipant>`
.. |CreateRpcClient| replace:: :cpp:func:`CreateRpcClient()<SilKit::IParticipant::CreateRpcClient()>`
.. |CreateRpcServer| replace:: :cpp:func:`CreateRpcServer()<SilKit::IParticipant::CreateRpcServer()>`

.. |SetCallResultHandler| replace:: :cpp:func:`SetCallReturnHandler()<SilKit::Services::Rpc::IRpcClient::SetCallResultHandler()>`
.. |Call| replace:: :cpp:func:`Call()<SilKit::Services::Rpc::IRpcClient::Call()>`
.. |CallWithTimeout| replace:: :cpp:func:`CallWithTimeout()<SilKit::Services::Rpc::IRpcClient::CallWithTimeout()>`

.. |SetCallHandler| replace:: :cpp:func:`SetCallHandler()<SilKit::Services::Rpc::IRpcServer::SetCallHandler()>`
.. |SubmitResult| replace:: :cpp:func:`SubmitResult()<SilKit::Services::Rpc::IRpcServer::SubmitResult()>`

.. |RpcSpec| replace:: :cpp:class:`RpcSpec<SilKit::Services::Rpc::RpcSpec>`
.. |AddLabel| replace:: :cpp:func:`AddLabel()<SilKit::Services::Rpc::RpcSpec::AddLabel>`
.. |MatchingLabel| replace:: :cpp:class:`MatchingLabel<SilKit::Services::MatchingLabel>`

.. |IRpcClient| replace:: :cpp:class:`IRpcClient<SilKit::Services::Rpc::IRpcClient>`
.. |IRpcServer| replace:: :cpp:class:`IRpcServer<SilKit::Services::Rpc::IRpcServer>`

.. |RpcCallHandler| replace:: :cpp:type:`RpcCallHandler<SilKit::Services::Rpc::RpcCallHandler>`
.. |RpcCallResultHandler| replace:: :cpp:type:`RpcCallResultHandler<SilKit::Services::Rpc::RpcCallResultHandler>`

.. |RpcCallResultEvent| replace:: :cpp:class:`RpcCallResultEvent<SilKit::Services::Rpc::RpcCallResultEvent>`

.. |MediaTypeRpc| replace:: :cpp:func:`MediaTypeRpc()<SilKit::Util::SerDes::MediaTypeRpc()>`

.. |CoordinatedLifecycle| replace:: :ref:`coordinated lifecycle<subsubsec:sim-lifecycle-starting-a-simulation>`

.. _chap:rpc-service-api:

===============================
RPC (Remote Procedure Call) API
===============================

.. contents::
   :local:
   :depth: 3


Using the RPC API
=================

This API provides a client-server model for remote calls with serialized argument- and return data.

Calling a Remote Procedure
--------------------------

The |IRpcClient| is instantiated from an |IParticipant| instance by calling the |CreateRpcClient| method.

.. code-block:: cpp

    auto rpcCallResultHandler = [] (IRpcClient*, const RpcCallResultEvent& event) {
        if (event.callStatus == SilKit::Services::Rpc::RpcCallStatus::Success)
        {
            return;
        }

        SilKit::Util::SerDes::Deserializer deserializer{SilKit::Util::ToStdVector(event.resultData)};
        std::cout << "sum is " << deserializer.Deserialize<uint32_t>(32) << " with user context " << event.userContext << std::endl;
    };

    SilKit::Services::Rpc::RpcSpec rpcSpec{"Add", SilKit::Util::SerDes::MediaTypeRpc()};
    auto* client = participant->CreateRpcClient("AddClient", rpcSpec, rpcCallResultHandler);

Remote procedures are invoked through the |Call| method of an |IRpcClient| instance.
The |Call| method is non-blocking and allows for later identification of the call through an additional user context pointer (of type ``void *``) which is passed as an optional, second argument and provided in the call return handler.

Additionally, |CallWithTimeout| can be used to trigger calls that have to be replied to within a specified timeout duration.
Otherwise the call will lead to a timeout |RpcCallResultEvent|.

The |IRpcClient| receives the call result in a callback specified during creation of the |IRpcClient|, and can be overwritten with |SetCallResultHandler|.
The callback provides the user context pointer passed to |Call| or |CallWithTimeout|, the return data, and a call status indicating success or an error during the procedure.

.. code-block:: cpp

    SilKit::Util::SerDes::Serializer serializer;
    serializer.BeginStruct();
    serializer.Serialize(uint32_t{31}, 32);
    serializer.Serialize(uint32_t{11}, 32);
    serializer.EndStruct();

    client->Call(serializer.ReleaseBuffer());


Serving a Remote Procedure
--------------------------

The |IRpcServer| is instantiated from an |IParticipant| instance by calling the |CreateRpcServer| method.

Any call that arrives at the |IRpcServer| is delivered via a callback specified during creation of the |IRpcServer|, which can be overwritten using the |SetCallHandler| method.
There, the argument data and call handle are provided and can be processed.

The |IRpcServer| must submit the answer to the call at a later point in time with the call handle obtained in the |RpcCallHandler| by using the |SubmitResult| method providing the return data for the calling |IRpcClient|.

.. code-block:: cpp

    auto rpcCallHandler = [](IRpcServer* server, const RpcCallEvent& event) {
        SilKit::Util::SerDes::Deserializer deserializer{SilKit::Util::ToStdVector(event.argumentData)};
        deserializer.BeginStruct();
        const auto lhs = deserializer.Deserialize<uint32_t>(32);
        const auto rhs = deserializer.Deserialize<uint32_t>(32);
        deserializer.EndStruct();

        SilKit::Util::SerDes::Serializer serializer;
        serializer.Serialize(lhs + rhs, 32);

        server->SubmitResult(event.callHandle, serializer.ReleaseBuffer());
    };

    SilKit::Services::Rpc::RpcSpec rpcSpec{"Add", SilKit::Util::SerDes::MediaTypeRpc()};
    auto* server = participant->CreateRpcServer("AddServer", rpcSpec, rpcCallHandler);

Argument and return data is represented as a byte vector, so the serialization schema can be chosen by the user.
Nonetheless, it is highly recommended to use SIL Kit's :doc:`Data Serialization/Deserialization API</api/serdes>` to ensure compatibility among all SIL Kit participants.


Usage Examples
==============

Example: Simple Calculator
--------------------------

In this example, the RPC Server offers a simple function for adding two numbers. 
The example shows the usage of the RPC Server / Client and data (de-)serialization. 
Note that the availability of the RPC Server is not guaranteed and will depend on the starting order of the two participants.
The next example shows how a coordinated lifecycle can be set up to guarantee the reception of the RPC client call.

Server - Addition
~~~~~~~~~~~~~~~~~

.. code-block:: cpp

    auto rpcCallHandler = [](IRpcServer* server, const RpcCallEvent& event) {
        SilKit::Util::SerDes::Deserializer deserializer{SilKit::Util::ToStdVector(event.argumentData)};
        deserializer.BeginStruct();
        const auto lhs = deserializer.Deserialize<uint32_t>(32);
        const auto rhs = deserializer.Deserialize<uint32_t>(32);
        deserializer.EndStruct();

        SilKit::Util::SerDes::Serializer serializer;
        serializer.Serialize(lhs + rhs, 32);

        server->SubmitResult(event.callHandle, serializer.ReleaseBuffer());
    };

    SilKit::Services::Rpc::RpcSpec rpcSpec{"Add", SilKit::Util::SerDes::MediaTypeRpc()};
    auto* server = participant->CreateRpcServer("AddServer", rpcSpec, rpcCallHandler);

Client - Addition
~~~~~~~~~~~~~~~~~

.. code-block:: cpp

    auto rpcCallResultHandler = [] (IRpcClient*, const RpcCallResultEvent& event) {
        if (event.callStatus != SilKit::Services::Rpc::RpcCallStatus::Success)
        {
            return;
        }

        SilKit::Util::SerDes::Deserializer deserializer{SilKit::Util::ToStdVector(event.resultData)};
        std::cout << "sum is " << deserializer.Deserialize<uint32_t>(32) << std::endl;
    };

    SilKit::Services::Rpc::RpcSpec rpcSpec{"Add", SilKit::Util::SerDes::MediaTypeRpc()};
    auto* client = participant->CreateRpcClient("AddClient", rpcSpec, rpcCallResultHandler);

    std::this_thread::sleep_for(1s);
    
    SilKit::Util::SerDes::Serializer serializer;
    serializer.BeginStruct();
    serializer.Serialize(uint32_t{31}, 32);
    serializer.Serialize(uint32_t{11}, 32);
    serializer.EndStruct();

    client->Call(serializer.ReleaseBuffer());


Example: RPC with guaranteed call reception
-------------------------------------------

This example is based on the previous one and includes participant creation and the setup of a |CoordinatedLifecycle|.
This guarantees that the RPC client and server are validly connected at the time the client makes the call.

Server - Addition
~~~~~~~~~~~~~~~~~

.. code-block:: cpp

    #include <iostream>

    #include "silkit/SilKit.hpp"
    #include "silkit/services/rpc/all.hpp"
    #include "silkit/services/orchestration/all.hpp"
    #include "silkit/util/serdes/Serialization.hpp"

    using namespace SilKit::Services::Orchestration;
    using namespace SilKit::Services::Rpc;

    int main(int argc, char** argv)
    {
        auto config = SilKit::Config::ParticipantConfigurationFromString("");
        auto participant = SilKit::CreateParticipant(config, "Server", "silkit://localhost:8500");
        auto* lifecycleService = participant->CreateLifecycleService({OperationMode::Coordinated});

        auto rpcCallHandler = [](IRpcServer* server, const RpcCallEvent& event) {
            SilKit::Util::SerDes::Deserializer deserializer{SilKit::Util::ToStdVector(event.argumentData)};
            deserializer.BeginStruct();
            const auto lhs = deserializer.Deserialize<uint32_t>(32);
            const auto rhs = deserializer.Deserialize<uint32_t>(32);
            deserializer.EndStruct();

            SilKit::Util::SerDes::Serializer serializer;
            serializer.Serialize(lhs + rhs, 32);

            std::cout << "Server function 'Add' is called with parameters: " << lhs << ", " << rhs << std::endl;
            server->SubmitResult(event.callHandle, serializer.ReleaseBuffer());
        };

        SilKit::Services::Rpc::RpcSpec rpcSpec{"Add", SilKit::Util::SerDes::MediaTypeRpc()};
        auto* server = participant->CreateRpcServer("AddServer", rpcSpec, rpcCallHandler);
    
        auto finalStateFuture = lifecycleService->StartLifecycle();
        finalStateFuture.get();

        return 0;
    }

Client - Addition
~~~~~~~~~~~~~~~~~

.. code-block:: cpp

    #include <iostream>

    #include "silkit/SilKit.hpp"
    #include "silkit/services/rpc/all.hpp"
    #include "silkit/services/orchestration/all.hpp"
    #include "silkit/util/serdes/Serialization.hpp"

    using namespace SilKit::Services::Orchestration;
    using namespace SilKit::Services::Rpc;

    int main(int argc, char** argv)
    {
        auto config = SilKit::Config::ParticipantConfigurationFromString("");
        auto participant = SilKit::CreateParticipant(config, "Client", "silkit://localhost:8500");
        auto* lifecycleService = participant->CreateLifecycleService({OperationMode::Coordinated});

        auto rpcCallResultHandler = [](IRpcClient*, const RpcCallResultEvent& event) {
            if (event.callStatus != SilKit::Services::Rpc::RpcCallStatus::Success)
            {
                return;
            }

            SilKit::Util::SerDes::Deserializer deserializer{SilKit::Util::ToStdVector(event.resultData)};
            std::cout << "Client obtained result: " << deserializer.Deserialize<uint32_t>(32) << std::endl;
        };

        SilKit::Services::Rpc::RpcSpec rpcSpec{"Add", SilKit::Util::SerDes::MediaTypeRpc()};
        auto* client = participant->CreateRpcClient("AddClient", rpcSpec, rpcCallResultHandler);

        lifecycleService->SetCommunicationReadyHandler([client]() {
            SilKit::Util::SerDes::Serializer serializer;
            serializer.BeginStruct();
            serializer.Serialize(uint32_t{31}, 32);
            serializer.Serialize(uint32_t{11}, 32);
            serializer.EndStruct();

            std::cout << "Client calls: 'Add(31, 11)'" << std::endl;
            client->Call(serializer.ReleaseBuffer());
        });

        auto finalStateFuture = lifecycleService->StartLifecycle();
        finalStateFuture.get();

        return 0;
    }


API and Data Type Reference
===========================

RpcClient API
-------------

.. doxygenclass:: SilKit::Services::Rpc::IRpcClient
   :members:

RpcServers API
--------------

.. doxygenclass:: SilKit::Services::Rpc::IRpcServer
   :members:

Callback Types
--------------

.. doxygentypedef:: SilKit::Services::Rpc::RpcCallHandler

.. doxygentypedef:: SilKit::Services::Rpc::RpcCallResultHandler

Data Structures
---------------

.. doxygenstruct:: SilKit::Services::Rpc::RpcCallEvent
   :members:

.. doxygenstruct:: SilKit::Services::Rpc::RpcCallResultEvent
   :members:

.. doxygenclass:: SilKit::Services::Rpc::RpcSpec
   :members:

Enumerations
------------

.. doxygenenum:: SilKit::Services::Rpc::RpcCallStatus


Advanced Usage and Configuration
================================

Function Name
-------------

RPC clients and RPC servers provide a function name which is part of their |RpcSpec|.

Communication only takes place among RPC clients and RPC servers with the same function name.

Media Type
----------

Both RPC clients and RPC servers define a media type as part of their |RpcSpec|.
It is a meta description of the transmitted data in accordance to `RFC2046 <https://datatracker.ietf.org/doc/html/rfc2046>`_ and should be used to provide information about the de-/serialization of the underlying user data.
Just like the function name, the media type has to match between RPC clients / RPC servers for communication to take place.
An empty string on an RPC client will match any other media type on a server.

When data is serialized using SIL Kit's :doc:`Data Serialization/Deserialization API</api/serdes>`, the media type constant |MediaTypeRpc| must be used.

Labels
------

Both RPC clients and RPC servers can be annotated with string-based key-value pairs (labels) which can be either mandatory or optional.
In addition to the matching requirements given by topic and media type, RPC clients and RPC servers will only communicate if their labels match.

The labels are stored in the |RpcSpec|. A |MatchingLabel| can be added via |AddLabel|, see the following code snippet:

.. code-block:: cpp

    SilKit::Services::Rpc::RpcSpec rpcSpec{"OpenMirror", "application/json"};
    rpcSpec.AddLabel("Instance", "FrontLeft", SilKit::Services::MatchingLabel::Kind::Optional);
    auto* client = participant->CreateRpcClient("FrontLeftDoorMirrorPanel", rpcSpec, callResultHandler);

To communicate, RPC clients and RPC Servers must conform to the following matching rules:

* A mandatory label matches, if a label of the same key and value is found on the corresponding counterpart.
* An optional label matches, if the label key does not exist on the counterpart or both its key and value are equal.

The following table shows how RPC clients and RPC servers with matching topics and matching media type would
match corresponding to their labels. Note that the label matching is symmetric, so clients and servers
are interchangeable here.

.. list-table:: Label combinations
   :header-rows: 1

   * -
     - Server {"Instance", "Left", Optional}
     - Server {"Instance", "Left", Mandatory}
   * - Client {}
     - Match
     - No Match
   * - Client {"Instance", "Left", Optional}
     - Match
     - Match
   * - Client {"Instance", "Right", Optional}
     - No Match
     - No Match
   * - Client {"Namespace", "Car", Optional}
     - Match
     - No Match
   * - Client {"Namespace", "Car", Mandatory}
     - No Match
     - No Match

Error handling
--------------

* If using |Call| with no corresponding server available, the ``CallReturnHandler`` is triggered immediately with
  ``RpcCallStatus::ServerNotReachable``.
* |SubmitResult| must only be used with a valid call handle received in the ``RpcHandler``.
* The ``RpcCallResultEvent::resultData`` member is only valid if ``callStatus == RpcCallStatus::Success``.
* If the RPC server receives a call but does not have a valid call handler, the RPC client will receive an
  ``RpcCallResultEvent`` with ``callStatus == RpcCallStatus::InternalServerError``.
* If the RpcServer does not reply within the specified timeout of |CallWithTimeout|, the CallReturnHandler is triggered
  immediately with ``RpcCallStatus::Timeout``.
