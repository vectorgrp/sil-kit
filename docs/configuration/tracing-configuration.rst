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

Tracing allows to write received and sent messages of a controller in a trace file (e.g. PCAP). 
Through the replay functionality earlier recorded trace files can be replayed. By this a conifgured controller can behave exactly as within the recorded simulation.

The following section shows the tracing and replay related definitions and gives usage examples.

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


Usage
-------
In this example the EthernetReader and EthernetWriter of the ``EthernetDemo`` are adapted to store the received and transmitted messages in the given PCAP trace files. 
The participant YAML configurations are extended as shown in the code blocks. The ``Tracing`` section is added to describe the used trace sink, output file and tracing format. 
The defined trace sink is assigned to The ``EthernetController``.

.. _label:configuration-tracing-example:

.. figure:: ../_static/ethernet_tracing_example_config.svg
   :alt: : Simple simulation setup with two participants and tracing enabled.
   :align: center
   :width: 800

   : Simple simulation setup with two participants and tracing enabled.

The following YAML code shows the participant configuration of the EthernetReader with tracing enabled.

.. code-block:: yaml
    
    Description: Tracing sample configuration for EthernetReader
    EthernetControllers:
    - Name: Eth1
        UseTraceSinks: [ Sink1 ]
    Tracing:
        TraceSinks:
        - Name: Sink1
            Type: PcapFile
            OutputPath: EthernetReader_TraceSinkFile.pcap

The following YAML code shows the participant configuration of the EthernetWriter with tracing enabled.

.. code-block:: yaml
    
    Description: Tracing sample configuration for EthernetWriter
    EthernetControllers:
    - Name: Eth1
        UseTraceSinks: [ Sink1 ]
    Tracing:
        TraceSinks:
        - Name: Sink1
            Type: PcapFile
            OutputPath: EthernetWriter_TraceSinkFile.pcap


.. admonition:: Note

    In order for the tracing to work, it is important to terminate the simulation properly.

    1. Stop the system controller by pressing enter
    2. Stop the EthernetWriter and EthernetReader by pressing enter


.. _sec:cfg-participant-replay:


Replay
======

Any controller may contain a ``Replay`` configuration,
which references a trace source and may specify additional properties influencing how messages are replayed.

Messages from the replay file are injected as if they were sent or received by the controller.
The timestamps present in the trace source are used to schedule when the messages are injected.

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
     - Messages from the replay file are injected as if they were sent or received by the controller. Can be ``Send``, ``Receive``, or ``Both``. If the replay is active for a direction, normal data transmission is blocked. 

.. admonition:: Note

    Messages from a ``PCAP`` trace source are always sent by the controller with the ``userContext`` being ``nullptr``.


Usage
------

A usage example is the reproduction of a faulty simulation, without having to share all parts of the simulation to debug a single simulation participant.
In this example the EthernetWriter will replay from a pcap file why the EthernetReader will act as in the unchanged ``EthernetDemo``.

The EthernetWrite participant YAML configuration is extended as shown in the code block. The ``Tracing`` section is added to describe the used trace source, input file and tracing format. 
The defined trace source is assigned to The ``EthernetController`` by define it in the Replay section. In addition the ``Direction`` filed can be used to define the rplay behaviour. 

.. _label:configuration-replay-example:

.. figure:: ../_static/ethernet_replay_example_config.svg
   :alt: : Simple simulation setup with two participants and replay enabled.
   :align: center
   :width: 800

   : Simple simulation setup with two participants and replay enabled.


The following YAML code shows the participant configuration of the EthernetWriter with replay enabled.

.. code-block:: yaml
    
    Description: Replay sample configuration for EthernetWriter
    EthernetControllers:
    - Name: Eth1
        Replay: 
          UseTraceSource: Source1
          Direction: Both
    Tracing:
        TraceSources:
        - Name: Source1
          Type: PcapFile
          InputPath: EthernetWriter_ReplaySourceFile.pcap


.. _sec:cfg-participant-trace-sink-source-types:

Trace Sink and Source Types
===========================

SIL Kit supports tracing and replay in different formats.

.. admonition:: Note

    * At the moment only ``PCAP`` tracing and replay is available in the SIL Kit library.
    * The PCAP format can only be used with Ethernet controllers.

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

    


