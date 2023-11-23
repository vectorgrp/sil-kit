.. _sec:cfg-participant-middleware:

.. |NormalOperationNotice| replace:: It is *not* required to set this field under normal circumstances.

===================================================
Middleware Configuration
===================================================

.. contents:: :local:
   :depth: 3

Overview
--------------------

The Vector SIL Kit uses an internal middleware for message exchange.
This middleware is open source and included in the sources of the Vector SIL Kit.

The middleware can be configured as follows.

Configuration
--------------------

For the initial discovery of the SIL Kit participants, the SIL Kit uses a central registry
process at a preconfigured hostname and port. By default, the registry is expected to be
running on 'localhost' listening on port 8500. These values can be changed via the
`RegistryUri`, for example:


.. code-block:: yaml

    Middleware:
      RegistryUri: silkit://localhost:8500
      ConnectAttempts: 1
      TcpNoDelay: false
      TcpQuickAck: false
      TcpSendBufferSize: 1024
      TcpReceiveBufferSize: 1024
      RegistryAsFallbackProxy: false
      ConnectTimeoutSeconds: 5.0

.. list-table:: Middleware Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description

   * - RegistryUri
     - The URI used by participants when connecting to the SIL Kit Registry.
       By default, the registry is expected to be running on 'localhost' with port 8500.
       The URI uses a scheme of 'silkit', i.e., ``silkit://localhost:8500``.

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

   * - AcceptorUris
     - Overwrite the default acceptor URIs of the participant. The configuration
       field exists to support more complicated network setups, where the
       listening ports of the participant must have a known, fixed port number
       and address.
       |NormalOperationNotice|

   * - RegistryAsFallbackProxy
     - Disable using the registry as a proxy for participant-to-participant
       communication as a fallback, if the direct connection attempts fail.
       The feature is enabled by default and can be disabled explicitly via this
       field.
       |NormalOperationNotice|

   * - ConnectTimeoutSeconds
     - The timeout (in seconds) until a connection attempt is aborted or a handshake is considered failed.
       This timeout applies to each attempt (TCP, Local-Domain) individually.
       |NormalOperationNotice|
