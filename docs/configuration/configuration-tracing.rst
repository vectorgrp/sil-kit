================================
Tracing and Replay Configuration
================================

.. admonition:: Note

    At the moment, the exact behavior of tracing and replay,
    and the available configuration settings are under development and should be considered **experimental**.


.. contents:: :local:
   :depth: 3


Overview
========

When a controller is configured for tracing, it references a trace sink by name.
Similarly, when a controller is configured for replay, it references a trace source by name in its :ref:`Replay<sec:cfg-participant-replay>` configuration.

The following section describes the definitions for the named trace sinks and trace sources, and the definitions for replay attached to each controller.

.. admonition:: Note

    At the moment only ``PCAP`` tracing and replay is available in the SIL Kit library.


.. _sec:cfg-participant-tracing:

Tracing
=======

Trace Sinks
-----------

.. code-block:: yaml
    
    Tracing:
      TraceSinks:
        - Type: ...
          Name: ...
          OutputPath: ...

.. list-table:: Trace Sink Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Type
     - The type of trace sink to create.  Can be ``PcapFile``, or ``PcapPipe``.
       See :ref:`Trace Sink Types<sec:cfg-participant-trace-sink-source-types>` for more information on the individual types.
   * - Name
     - The name of the trace sink. This name is used in the controller configuration (``UseTraceSinks``) to reference the sink.
   * - OutputPath
     - The path used to create the trace sink. How the path is used, depends on the ``Type`` property.

Trace Sources
-------------

.. code-block:: yaml
    
    Tracing:
      TraceSources:
        - Type: ...
          Name: ...
          InputPath: ...

.. list-table:: Trace Source Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Type
     - The type of trace source to create.  Can be ``PcapFile``, or ``PcapPipe``.
       See :ref:`Trace Source Types<sec:cfg-participant-trace-sink-source-types>` for more information on the individual types.
   * - Name
     - The name of the trace source. This name is used in the controller configuration (``Replay/UseTraceSource``) to reference the source.
   * - InputPath
     - The path used to create the trace source. How the path is used, depends on the ``Type`` property.


.. _sec:cfg-participant-replay:

Replay
======

Any controller may contain a ``Replay`` configuration,
which references a trace source and may specify additional properties influencing how messages are replayed.

Messages from the replay file are injected as if they were sent or received by the controller.
The timestamps present in the trace source are used to schedule when the messages are injected.

.. admonition:: Note

    Messages from a ``PCAP`` trace source are always sent by the controller with the ``userContext`` being ``nullptr``.

.. code-block:: yaml
    
    ...Controllers:
      - Name: ...
        ...: ...
        Replay:
          UseTraceSource: ...
          Direction: ...

.. list-table:: Per-Controller Replay Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - UseTraceSource
     - The name of a single trace source, as defined in the :ref:`Tracing<sec:cfg-participant-tracing>` configuration.
   * - Direction
     - Only replay messages from the trace source that match this direction filter. Can be ``Send``, ``Receive``, or ``Both``.


.. _sec:cfg-participant-trace-sink-source-types:

Trace Sink and Source Types
===========================

SIL Kit supports tracing and replay in different formats.

.. admonition:: Note

    At the moment only ``PCAP`` tracing and replay is available in the SIL Kit library.

PCAP
----

The ``PCAP`` file format is a common format for tracing Ethernet packets.
It is widely supported by tools, e.g., by Wireshark and the ``tcpdump`` family of command line tools.

PcapFile
~~~~~~~~

If ``PcapFile`` is used for the ``Type`` property in the trace sink or source definition,
SIL Kit will write/read the trace to/from a normal file,
identified by the ``OutputPath`` or ``InputPath`` properties respectively.

PcapPipe
~~~~~~~~

If ``PcapPipe`` is used for the ``Type`` property in the trace sink or source definition,
SIL Kit will write/read the trace to/from a named pipe,
identified by the ``OutputPath`` or ``InputPath`` properties respectively.

SIL Kit will wait until the other end of the named pipe has been opened, e.g., opening Wireshark on the named pipe.

.. admonition:: Note

    * On Windows, SIL Kit places (or looks for) the named pipe under the path ``\\.\pipe\<ConfigValue>`` where ``<ConfigValue>`` is the value of the ``OutputPath`` (or ``InputPath``).

.. admonition:: Note

    * The PCAP format can only be used with Ethernet controllers.
    * When used as a trace source, all messages will be replayed as transmissions by the replaying controller.