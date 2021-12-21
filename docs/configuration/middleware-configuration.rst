.. _sec:mwcfg:

===================================================
Middleware Config
===================================================

.. contents:: :local:
   :depth: 3

Overview
--------------------

Throughout its development, Vector Integration Bus offered multiple middlewares to choose from.
The now discontinued FastRtps based middleware is no longer supported, however it remains
visible in configuration values.
The currently supported middleware is boost ASIO based and was specifically designed for the Vector
Integration Bus as a transport layer.

The active middleware can be configured as follows.

.. admonition:: Note

    FastRTPS is now discontinued.
    All users should use the generic :cpp:func:`CreateComAdapter<ib::CreateComAdapter()>` API
    together with the VAsio middleware (which is the default).


.. _sec:mwcfg-vasio:

Configuring the VAsio Middleware
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For the initial discovery of the VIB participants, the VAsio middleware uses a registry
process at a preconfigured hostname and port. By default, the registry is expected to be
running on localhost listening on Port 8500. These values can be changed via the new
*/MiddlewareConfig/VAsio* section, e.g.:


.. code-block:: javascript

    {
        ...
        "MiddlewareConfig": {
            "ActiveMiddleware": "VAsio",
            "VAsio": {
                "Registry": {
                    "Hostname": "remotehost",
                    "Port": 14014,
                    "Logger": {
                        ...
                    },
                    "ConnectAttempts": 1
                },
                "TcpNoDelay": false,
                "TcpQuickAck": false,
                "TcpSendBufferSize": 1024,
                "TcpReceiveBufferSize": 1024,
            }
        }
    }

.. list-table:: VAsio Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description

   * - Registry
     - The optional :ref:`VAsio Registry configuration<sec:mwcfg-vasio-registry>`.

   * - TcpNoDelay
     - Enable the TCP_NODELAY flag on TCP sockets. This disables Nagle's algorithm.

   * - TcpQuickAck
     - Enable the TCP_QUICKACK flag on TCP sockets (Linux only). Disables delayed
       acknowledges, at the cost of increased syscall overhead.

   * - TcpSendBufferSize
     - Sets the TCP send buffer size. Be careful when changing the OS defaults!

   * - TcpReceiveBufferSize
     - Sets the TCP receive buffer size. Be careful when changing the OS defaults!


.. _sec:mwcfg-vasio-registry:

.. list-table:: VAsio Registry Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description

   * - Hostname
     - The hostname to be used by participants when connecting to the IbRegistry.
       By default, the registry is expected to be running on "localhost".

   * - Port
     - The base port to be used by participants when connecting to the IbRegistry.
       By default, the registry is expected to listen on the port 8500 + *IbDomainId*.

   * - Logger
     - Optional :ref:`Logger configuration<sec:cfg-participant-logger>` for the logger used by the registry.

   * - ConnectAttempts
     - Number of connects to the registry a participant should attempt before giving up and signaling an error.
       By default, only a single connect is attempted.

.. _sec:comadapter-factory:

ComAdapter Factory Method
----------------------------------------

CreateComAdapter
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygenfunction:: ib::CreateComAdapter
