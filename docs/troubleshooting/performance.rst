.. include:: /substitutions.rst

Performance
===========

The following sections provide support for improving the performance of |ProductName|.

TCP Socket Configuration
~~~~~~~~~~~~~~~~~~~~~~~~

The TCP sockets used by SIL Kit can be configured via the participant configuration file, more precisely the :doc:`middleware<../configuration/middleware-configuration>` options.
This configuration needs to be done carefully. In particular, using Nagle's algorithm (``TcpNoDelay: false``) and delayed acknowledgements (``TcpQuickAck: false``) are known to interact badly.
Please note that version 4.0.56 and following will use the ``TCP_NODELAY`` flag (which disables Nagle's algorithm) by default (``TcpNoDelay: true``).

Using ``TcpNoDelay: true`` and ``TcpQuickAck: true`` together may cause severe performance losses as well as non-deterministic behaviour, e.g., resulting in 40ms latency peaks on Linux platforms.

Message Aggregation
~~~~~~~~~~~~~~~~~~~

For the time-synchronized case, |ProductName| offers the possibility of aggregating messages prior to sending. 
This feature can be enabled via the :doc:`EnableMessageAggregation<../configuration/experimental-configuration>` parameter in the participant configuration file.
We mention that the message aggregation feature has a significant impact on the throughput, in particular if several small messages (<1kB) are sent within one simulation step.
