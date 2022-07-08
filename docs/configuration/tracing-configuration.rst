===================================================
Tracing and Replay Configuration
===================================================

.. contents:: :local:
   :depth: 3


.. _sec:cfg-tracing-replay-configuration-overview:

Overview
========================================

The Vector SIL Kit contains features for tracing and replaying within the simulation.
:doc:`Trace and Replay<../usage/replay>` describes these features in detail. 
This section of the documentation explains how these features can be configured through the participant configuration
file.

.. _sec:cfg-participant-tracing:

Message Tracing
=============================
To enable message tracing on a participant, two configuration options must be set:
at least one trace sink has to be defined in a *TraceSinks* block of the Tracing configuration, and a
*UseTraceSinks* field has to be defined in a service instance that references the
trace sink by name:

.. code-block:: yaml

    ---
    CanControllers:
    - Name: CanCtrl
      UseTraceSinks:
      - SinkForCan
    Tracing:
      TraceSinks:
      - Name: EthSink
        OutputPath: some/path/EthTraceOputput.pcap
        Type: PcapFile
      - Name: SinkForCan
        OutputPath: other path/CAN1.mdf4
        Type: Mdf4File

Multiple controllers can refer to a sink by name. However, each sink definition
in a TraceSinks block must have a unique name.
Currently, the :ref:`CanController<sec:cfg-participant-can>`,
:ref:`LinController<sec:cfg-participant-lin>`,
:ref:`EthernetController<sec:cfg-participant-ethernet>`,
and
:ref:`FlexrayController<sec:cfg-participant-flexray>`
support trace sinks.

The :ref:`VIBE MDF4Tracing extension<mdf4tracing>` supports tracing messages of
these controllers into an MDF4 file format.
VIBE MDF4Tracing is an extension in shared library form which must be loaded
at runtime. The :ref:`Extension Config<sec:cfg-extension-configuration-overview>`
can be used to adapt the search paths for this shared library.

The PCAP file format is natively supported for Ethernet messages only, please
refer to :ref:`EthernetController API<sec:api-ethernet-tracing>`.

.. _sec:cfg-participant-tracesink:

TraceSink
=============================
The TraceSink configuration is part of the tracing section of the :doc:`participant
configuration<configuration>`.

.. code-block:: yaml
  
    TraceSinks:
    - Name: MyPcapSink
      Type: PcapFile
      OutputPath: Filesystem/Path/MyTrace.pcap


It allows to trace the SIL Kit simulation messages into binary files.

.. list-table:: TraceSink Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the sink. Services may refer to this
       sink by name.
   * - Type
     - The type specifies the format of the output stream. Supported formats
       are: PcapFile, PcapPipe, Mdf4File.
   * - OutputPath
     - A filesystem path where the SIL Kit messages are traced to.


.. _sec:cfg-participant-replaying:

Message Replaying
=============================

Similar to the :ref:`sec:cfg-participant-tracing` configuration, two options
must be set to configure message replaying.
At least one trace source has to be defined in a *TraceSources* block of
the participant, and a *Replay* block has to be defined in a service instance:

.. code-block:: yaml

    ---
    CanControllers:
    - Name: CanCtrl
      Replay:
        Direction: Send
        UseTraceSource: Source1
        MdfChannel: {}
    Tracing:
      TraceSources:
      - Name: EthSource1
        InputPath: some/path/EthTraceOputput.pcap
        Type: PcapFile
      - Name: Source1
        OutputPath: other path/CAN1.mdf4
        Type: Mdf4File




.. _sec:cfg-participant-tracesource:

TraceSource
=============================

The TraceSource configuration is part of the tracing section of the :doc:`participant
configuration<configuration>`.

.. code-block:: yaml
  
    TraceSources:
    - Name: Source1
      Type: PcapFile
      InputPath: Filesystem/Path/MyTrace.pcap


This specifies a trace data source for replaying SIL Kit simulation messages during
live simulations.

.. list-table:: TraceSource Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the source. Services may refer to this
       source by name in a Replay configuration.
   * - Type
     - The type specifies the format of the input stream. Supported formats
       are: PcapFile, Mdf4File.
   * - InputPath
     - A filesystem path where the SIL Kit messages are loaded from.


.. _sec:cfg-participant-replay:

Replay Configuration
=============================
The replay configuration is part of a :doc:`participant's service configuration<configuration-services>`.

.. list-table:: Replay Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - UseTraceSource
     - The name of the trace source to use as a simulation message source.
   * - Direction
     - The message direction of the trace source data. May be "Send", "Receive"
       or "Both".
   * - MdfChannel
     - An (optional) MdfChannel identifier object. May be used to uniquely select
       a MDF channel in an MDF trace file.

Refer to :ref:`sec:replay-foreign` for guidelines on how to use the ``MdfChannel`` to select a replay channel.

.. _table-mdfchannel-json:

.. list-table:: MdfChannel Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description

   * - ChannelName
     - The name of a MDF channel (optional).
   * - ChannelSource
     - The name of a MDF channel's source information object (optional).
   * - ChannelPath
     - The path of a MDF channel's source information object (optional).

   * - GroupName
     - The acquistion name of the MDF channel's channel group (optional).
   * - GroupSource
     - The source of the channel group's source information object (optional).
   * - GroupPath
     - The path of the channel group's source information object (optional).

.. admonition:: Note

    Please note, that all members of ``MdfChannel`` are optional and that the empty string is a valid configuration choice.
    The empty string matches the empty text value, or it indicates the absence of the corresponding MDF meta data.
    To ensure that a ``MdfChannel`` member is not part of MDF channel selection, remove it from the configuration.
    If no ``MdfChannel`` members are specified, the channel selection will use SIL Kit internal criteria.


