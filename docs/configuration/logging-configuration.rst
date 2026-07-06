.. include:: /substitutions.rst

.. _sec:cfg-participant-logging:

===================================================
Logging Configuration
===================================================

.. contents:: :local:
   :depth: 3

Overview
========================================

Within the SIL Kit, the Logger provides features for local and distributed logging on different log 
levels.

Configuration
========================================
The Logging configuration allows to configure the logging behavior of the simulation participant.
Within the SIL Kit, the Logger uses so-called sinks to store log messages.
Multiple sinks can be configured at the same time. For example, to send log
messages with log level ``Debug`` or higher to a remote logger and write ``Trace`` level
logs to a file, the following configuration could be used:

.. code-block:: yaml

    Logging:
      Sinks:
      - Type: Stdout
        Level: Info
      - Type: Remote
        Level: Debug
      - Type: File
        Level: Trace
        LogName: ParticipantLog


.. _sec:cfg-participant-logger:

.. list-table:: Logger Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Sinks
     - A list of logger :ref:`sink configurations<sec:cfg-participant-logsinks>`
   * - FlushLevel
     - The log level at which flushes are triggered.  Valid options are *Critical*,
       *Error*, *Warn*, *Info*, *Debug*, *Trace*, and *Off*.
   * - LogFromRemotes
     - A boolean flag whether to log messages from other participants with
       remote sinks. Log messages received from other participants are only 
       sent to local sinks, i.e., *Stdout* and *File*



.. _sec:cfg-participant-logsinks:

.. list-table:: Sink Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Type
     - The sink type determines where the log messages are stored or sent
       to. Valid options are *Stdout*, *File*, and *Remote*. Sinks of type
       *Remote* send the log messages over the underlying middleware. Note that
       this can result in a significant amount of traffic, which can impact the
       simulation performance, in particular when using a low log level.
   * - Level
     - The minimum log level of a message to be logged by the sink. All messages
       with a lower log level are ignored. Valid options are *Critical*,
       *Error*, *Warn*, *Info*, *Debug*, *Trace*, and *Off*.
   * - LogName
     - The filename used by sinks of type *File*. The
       resulting filename is ``<LogName>_<Sanitized-Participant-Name>_<ISO-TimeStamp>.txt``.
   * - EnabledTopics
     - Optional allow-list of logging topics for this sink. If this list is non-empty,
       only messages with topics from this list are written to the sink.
   * - DisabledTopics
     - Optional block-list of logging topics for this sink. Disabled topics are always
       filtered out, even if they are also listed in *EnabledTopics*.

Topic-based sink filtering
========================================

Topic filters can be configured per sink by using *EnabledTopics* and/or *DisabledTopics*.

- If *EnabledTopics* is empty or not set, all topics are allowed.
- If *EnabledTopics* is set, only listed topics are allowed.
- *DisabledTopics* always has precedence and filters matching topics out.
- Topic names are parsed case-insensitively.
- User-level logging uses the ``User`` topic.

Example:

.. code-block:: yaml

    Logging:
      Sinks:
      - Type: Stdout
        Level: Trace
        EnabledTopics: [User, Ethernet]

In this example, only the user-level and ``Ethernet`` log messages are enabled.




The following topic names are currently assigned by SIL Kit components:

A topic classifies the origin of a log message (for example middleware internals, service controllers, or user-level logging) and can be used by sink filters to include or exclude related messages.

- ``Asio``: Logging from middleware communication and connection handling.
- ``Can``: Logging related to CAN services and controllers.
- ``Dashboard``: Logging from dashboard integration and dashboard clients.
- ``Ethernet``: Logging related to Ethernet services and controllers.
- ``Extension``: Logging from SIL Kit extension components.
- ``Flexray``: Logging related to FlexRay services and controllers.
- ``LifeCycle``: Logging from lifecycle state transitions and lifecycle management.
- ``Lin``: Logging related to LIN services and controllers.
- ``MessageTracing``: Logging from message tracing internals.
- ``Metrics``: Logging from metrics collection and processing.
- ``NetSim``: Logging from experimental network simulation components.
- ``Participant``: Logging from participant-level internal functionality.
- ``Pubsub``: Logging related to publish/subscribe data services.
- ``RequestReply``: Logging related to internal request/reply coordination.
- ``Rpc``: Logging related to RPC clients, servers, and discovery.
- ``ServiceDiscovery``: Logging related to service discovery.
- ``SystemMonitor``: Logging related to system monitor internals.
- ``SystemState``: Logging related to system state tracking and monitoring.
- ``TimeConfig``: Logging related to time synchronization configuration.
- ``TimeSync``: Logging from runtime time synchronization.
- ``Tracing``: Logging related to tracing, replay, and trace sinks.
- ``User``: Logging emitted by user application code.
