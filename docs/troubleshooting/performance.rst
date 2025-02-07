.. include:: /substitutions.rst

Performance
=========

The following sections provide support for improving the performance of |ProductName|.

Configuration of TCP Stack (Kernel)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The TCP stack of the kernel can be configured via the participant configuration file, more precisely the :doc:`middleware<../configuration/middleware-configuration>` options. 
This configuration needs to be done carefully. In particular, Nagle's algorithm (``TcpNoDelay``) and delayed acknowledgements (``TcpQuickAck``) are known to interact badly. 
Per default, Nagle's algorithm is disabled and delayed acknowledgements are enabled. Note that enabling both features may cause severe performance losses as well as non-deterministic behaviour.
Latency peaks of 40 ms and a large throughput variance have been observed in this case so far.

Message Aggregation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For the time-synchronized case, |ProductName| offers the possibility of aggregating messages prior to sending. 
This feature can be enabled via the :doc:`EnableMessageAggregation<../configuration/experimental-configuration>` parameter in the participant configuration file.
We mention that the message aggregation feature has a significant impact on the throughput, in particular if several small messages (<1kB) are sent within one simulation step.
