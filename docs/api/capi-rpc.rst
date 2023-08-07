RPC C API
-----------

.. contents::
   :local:
   :depth: 3

The RPC API provides client/server based RPC functionality.
It consists of RPC clients and RPC servers and a method to discover remote RPC servers.

RPC Clients
~~~~~~~~~~~
.. doxygenfunction:: SilKit_RpcClient_Create
.. doxygenfunction:: SilKit_RpcClient_Call
.. doxygenfunction:: SilKit_RpcClient_CallWithTimeout

An ``RpcClient`` is created with a handler for the call return by RPC servers:
.. doxygentypedef:: SilKit_CallResultHandler_t

RPC Servers
~~~~~~~~~~~
.. doxygenfunction:: SilKit_RpcServer_Create
.. doxygenfunction:: SilKit_RpcServer_SubmitResult

An ``RpcServer`` is created with a handler to process incoming calls by RPC clients:

.. doxygentypedef:: SilKit_RpcCallHandler_t

Data Structures
~~~~~~~~~~~~~~~
.. doxygentypedef:: SilKit_RpcCallHandle
.. doxygentypedef:: SilKit_RpcCallStatus