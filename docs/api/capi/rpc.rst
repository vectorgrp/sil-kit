RPC C API
-----------

.. contents::
   :local:
   :depth: 3

The Rpc API provides client/server based Rpc functionality. 
It consists of RpcClients and RpcServers and a method to discover remote RpcServers.

RpcClients
~~~~~~~~~~
.. doxygenfunction:: ib_Rpc_Client_Create
.. doxygenfunction:: ib_Rpc_Client_Call

A RpcClient is created with a handler for the call return by RpcServers:
.. doxygentypedef:: ib_Rpc_CallResultHandler_t

RpcServers
~~~~~~~~~~
.. doxygenfunction:: ib_Rpc_Server_Create
.. doxygenfunction:: ib_Rpc_Server_SubmitResult

A RpcServers is created with a handler to process incoming calls by RpcClients:
.. doxygentypedef:: ib_Rpc_CallHandler_t

RpcServer Discovery
~~~~~~~~~~~~~~~~~~~

A participant can poll for already known RpcServers:

.. doxygenfunction:: ib_Rpc_DiscoverServers

The method takes a handler with the discovery results:

.. doxygentypedef:: ib_Rpc_DiscoveryResultHandler_t

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: ib_Rpc_DiscoveryResult
   :members:
.. doxygenstruct:: ib_Rpc_DiscoveryResultList
   :members:
.. doxygentypedef:: ib_Rpc_CallHandle
.. doxygentypedef:: ib_Rpc_CallStatus