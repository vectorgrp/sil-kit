.. _sec:mwcfg:

===================================================
Middleware Configuration
===================================================

.. contents:: :local:
   :depth: 3

Overview
--------------------

The Vector Integration Bus is powered by the Vector VAsio middleware. 
This middleware is open source and included in the sources of the Vector Integration Bus.
The currently supported middleware is *Boost.Asio* based and was specifically designed for the Vector
Integration Bus as a transport layer.

The middleware can be configured as follows.

.. _sec:mwcfg-vasio:

Configuration
--------------------

For the initial discovery of the VIB participants, the VIB uses a central registry
process at a preconfigured hostname and port. By default, the registry is expected to be
running on localhost listening on Port 8500. These values can be changed via the
`RegistryUri`, e.g.:


.. code-block:: yaml

    Middleware:
      RegistryUri: vib://localhost:8500
      ConnectAttempts: 1
      TcpNoDelay: false
      TcpQuickAck: false
      TcpSendBufferSize: 1024
      TcpReceiveBufferSize: 1024


.. list-table:: VAsio Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description

   * - RegistryUri
     - The URI used by participants when connecting to the IbRegistry.
       By default, the registry is expected to be running on "localhost" with port 8500.
       The URI uses a scheme of 'vib', i.e. ``vib://localhost:8500``.

   * - ConnectAttempts
     - Number of connects to the registry a participant should attempt before giving up and signaling an error.
       By default, only a single connect is attempted.

   * - TcpNoDelay
     - Enable the TCP_NODELAY flag on TCP sockets. This disables Nagle's algorithm.

   * - TcpQuickAck
     - Enable the TCP_QUICKACK flag on TCP sockets (Linux only). Disables delayed
       acknowledges, at the cost of increased syscall overhead.

   * - TcpSendBufferSize
     - Sets the TCP send buffer size. Be careful when changing the OS defaults!

   * - TcpReceiveBufferSize
     - Sets the TCP receive buffer size. Be careful when changing the OS defaults!

