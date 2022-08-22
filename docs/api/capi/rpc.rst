RPC C API
-----------

.. contents::
   :local:
   :depth: 3

The Rpc API provides client/server based Rpc functionality. 
It consists of RpcClients and RpcServers and a method to discover remote RpcServers.

RpcClients
~~~~~~~~~~
.. doxygenfunction:: SilKit_RpcClient_Create
.. doxygenfunction:: SilKit_RpcClient_Call

A RpcClient is created with a handler for the call return by RpcServers:
.. doxygentypedef:: SilKit_CallResultHandler_t

RpcServers
~~~~~~~~~~
.. doxygenfunction:: SilKit_RpcServer_Create
.. doxygenfunction:: SilKit_RpcServer_SubmitResult

A RpcServers is created with a handler to process incoming calls by RpcClients:
.. doxygentypedef:: SilKit_RpcCallHandler_t

Data Structures
~~~~~~~~~~~~~~~
.. doxygentypedef:: SilKit_RpcCallHandle
.. doxygentypedef:: SilKit_RpcCallStatus