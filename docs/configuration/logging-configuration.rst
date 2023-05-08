===================================================
Logging Configuration
===================================================

.. contents:: :local:
   :depth: 3


.. _sec:cfg-logging-configuration-overview:

Overview
========================================

.. _sec:cfg-participant-logging:

Within the Vector SIL Kit, the Logger provides features for local and distributed logging on different log 
levels.

Configuration
========================================
The Logging configuration allows to configure the logging behavior of the simulation participant.
Within the Vector SIL Kit, the Logger uses so-called sinks to store log messages.
Multiple sinks can be configured at the same time. For example, to send log
messages with log level ``Debug`` or higher to a remote logger and write ``Trace`` level
logs to a file, the following configuration could be used:

.. code-block:: yaml

    Logging:
      Sinks:
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
       resulting filename is ``<LogName>_<ISO-TimeStamp>.txt``.
