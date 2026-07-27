.. include:: /substitutions.rst

.. _sec:cfg-participant-experimental:

===================================================
Experimental Configuration
===================================================

.. contents:: :local:
   :depth: 3

Overview
--------------------

This section includes experimental configuration fields.

.. warning::
  The features available through the experimental section might be changed or removed in future versions of the |ProductName|.

TimeSynchronization
--------------------

.. code-block:: yaml

    Experimental:
        TimeSynchronization:
            AnimationFactor: 1.0
            EnableMessageAggregation: Off
            DynamicSimulationStep: true

.. list-table:: TimeSynchronization Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description

   * - DynamicSimulationStep
     - Controls *dynamic simulation step sizes*. When enabled, each simulation step of this participant
       is shortened ("aligned") to the minimal step duration among all synchronized participants,
       instead of always advancing by this participant's own configured step size.
       The virtual time therefore stops at the earliest next time point of any participant, which lets
       participants with coarse step sizes still observe events produced by participants with finer steps.

       This is a tri-state option. Its three states have distinct meanings:

       .. list-table::
          :widths: 20 80
          :header-rows: 1

          * - Value
            - Behavior
          * - ``true``
            - Request dynamic stepping for the whole simulation: enable it locally **and** advertise the
              request to all peers, so participants that do not opt out follow. This is the typical
              setting for a driving participant such as a network simulator.
          * - ``false``
            - Hard opt-out: never enable dynamic stepping, even if a peer requests it. This participant
              always advances by its own configured step size.
          * - *(absent, the default)*
            - Follow the network: enable dynamic stepping if and only if at least one peer advertises the
              request; otherwise behave as before and advance by the participant's own step size.

       .. note::
         A typical setup uses ``true`` on a single driving participant (e.g. a network simulator) and
         leaves the key unset on all others, so they follow automatically. When a participant is enabled
         this way by a remote request, it emits an ``Info`` log message stating that dynamic stepping was
         enabled remotely and how to opt out (set ``DynamicSimulationStep`` to ``false``).

   * - AnimationFactor
     - This value affects |ProductName| simulations where a time synchronization service is used. 
       When this value is set, the virtual time is coupled to the local wall clock of the underlying system, scaled down by the given factor.
       E.g., a value of 1.0 means direct coupling to the wall clock. 
       A value of 2.0 means that the virtual time runs **twice as slow** as the local wall clock. 
       Accordingly, a value of 0.5 will cause the virtual time to run **twice as fast**. 
       When omitting the value or setting it to zero, no coupling to the wall clock takes place.

       .. note::
         Depending on the simulation step size, the chosen animation factor and the computational load per simulation step, it might not be possible to couple the virtual time to the wall clock with the desired factor.

   * - EnableMessageAggregation
     - Enable the aggregation of messages in |ProductName| simulations with time synchronization. 
       Valid options are *On*, *Auto* and *Off*. 
       If option *Auto* is chosen, the aggregation is enabled only for the case of synchronous simulation step handlers. 
       If option *On* is chosen, the aggregation is enabled for both synchronous and asynchronous simulation step handlers.
       
       .. note::
         Option *Auto* can be chosen without any concerns. 
         In the case of option *On*, however, it is necessary to verify that the transmission of messages within a time step does not depend on incoming messages from other participants.
         In this case, the time step will not be terminated and the communication will block.

Metrics for participants
------------------------
Each participant supports collecting static attributes of a simulation and
runtime performance metrics.
They are collected and distributed to the configured sinks at the given update interval.

.. code-block:: yaml

    Experimental:
        Metrics:
          Sinks:
            - Name: SomeSink1
              Type: Remote


.. list-table:: Participant Metrics Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description

   * - Sinks
     - A list of named metric sinks. They can be of type ``JsonFile`` or ``Remote``.
   * - updateInterval
     - The time between sending batches of metrics to the registry in seconds.

Available Metrics
~~~~~~~~~~~~~~~~~

Metric names use ``/`` as a hierarchy separator.
Some metrics contain runtime-specific path elements such as the simulation name or the
name of a connected peer participant.

.. list-table:: Available Participant Metrics
   :widths: 35 15 50
   :header-rows: 1

   * - Metric name
     - Kind
     - Description
   * - ``SilKit/System/OperatingSystem``
     - ``ATTRIBUTE``
     - Operating system reported by the participant host.
   * - ``SilKit/System/Hostname``
     - ``ATTRIBUTE``
     - Host name of the participant host.
   * - ``SilKit/System/PageSize``
     - ``ATTRIBUTE``
     - Memory page size reported by the participant host.
   * - ``SilKit/System/ProcessorCount``
     - ``ATTRIBUTE``
     - Number of processors reported by the participant host.
   * - ``SilKit/System/ProcessorArchitecture``
     - ``ATTRIBUTE``
     - Processor architecture reported by the participant host.
   * - ``SilKit/System/PhysicalMemory``
     - ``ATTRIBUTE``
     - Total physical memory of the participant host.
   * - ``SilKit/Process/Executable``
     - ``ATTRIBUTE``
     - Path of the running participant executable.
   * - ``SilKit/Process/Username``
     - ``ATTRIBUTE``
     - User name under which the participant process runs.
   * - ``SilKit/Participant/JsonConfig``
     - ``ATTRIBUTE``
     - Participant configuration serialized as JSON.
   * - ``TcpAcceptors``
     - ``STRING_LIST``
     - TCP endpoints on which the participant accepts connections.
   * - ``LocalAcceptors``
     - ``STRING_LIST``
     - Local domain socket endpoints on which the participant accepts connections.
   * - ``Peer/<simulationName>/<participantName>/LocalEndpoint``
     - ``STRING_LIST``
     - Local endpoint addresses used for the connection to the peer participant.
   * - ``Peer/<simulationName>/<participantName>/RemoteEndpoint``
     - ``STRING_LIST``
     - Remote endpoint addresses used for the connection to the peer participant.
   * - ``Peer/<simulationName>/<remoteParticipant>/tx_bytes/[bytes]``
     - ``COUNTER``
     - Total number of transmitted bytes for the peer connection.
   * - ``Peer/<simulationName>/<remoteParticipant>/tx_packets/[count]``
     - ``COUNTER``
     - Total number of transmitted packets for the peer connection.
   * - ``Peer/<simulationName>/<remoteParticipant>/tx_bandwidth/[Bps]``
     - ``STATISTIC``
     - Statistics over transmitted payload sizes, labeled in bytes per second.
   * - ``Peer/<simulationName>/<remoteParticipant>/rx_bytes/[bytes]``
     - ``COUNTER``
     - Total number of received bytes for the peer connection.
   * - ``Peer/<simulationName>/<remoteParticipant>/rx_packets/[count]``
     - ``COUNTER``
     - Total number of received packets for the peer connection.
   * - ``Peer/<simulationName>/<remoteParticipant>/tx_queue_size/[count]``
     - ``STATISTIC``
     - Statistics over the transmit queue size for the peer connection.
   * - ``Peer/<simulationName>/<remoteParticipant>/rx_bandwidth/[Bps]``
     - ``STATISTIC``
     - Statistics over received payload sizes, labeled in bytes per second.
   * - ``SimStepCount``
     - ``COUNTER``
     - Number of completed simulation steps.
   * - ``SimStep/execution_duration/[s]``
     - ``STATISTIC``
     - Statistics over simulation step handler execution time in seconds.
   * - ``SimStep/completion_duration/[s]``
     - ``STATISTIC``
     - Statistics over asynchronous simulation step completion time in seconds.
   * - ``SimStep/waiting_duration/[s]``
     - ``STATISTIC``
     - Statistics over waiting time between simulation steps in seconds.

Metric Value Encoding
~~~~~~~~~~~~~~~~~~~~~

The metric kind determines the format of the metric value:

.. list-table:: Metric Value Encoding
   :widths: 15 85
   :header-rows: 1

   * - Kind
     - Value format
   * - ``COUNTER``
     - A single integer value.
   * - ``STATISTIC``
     - A JSON array ``[mean, stddev, min, max]``.
   * - ``STRING_LIST``
     - A JSON array of strings.
   * - ``ATTRIBUTE``
     - A string value.

Example JSON Output
~~~~~~~~~~~~~~~~~~~

The ``JsonFile`` sink writes one JSON object per line.
Each object contains the timestamp in nanoseconds (``ts``), the participant name (``pn``),
the metric name (``mn``), the metric kind (``mk``), and the metric value (``mv``).

.. code-block:: json

    {"ts":1716200000000000000,"pn":"Participant1","mn":"SimStepCount","mk":"COUNTER","mv":42}
    {"ts":1716200000000100000,"pn":"Participant1","mn":"SimStep/execution_duration/[s]","mk":"STATISTIC","mv":[0.0017,0.0004,0.0012,0.0025]}
    {"ts":1716200000000200000,"pn":"Participant1","mn":"Peer/Sim1/Participant2/tx_bytes/[bytes]","mk":"COUNTER","mv":32768}
    {"ts":1716200000000300000,"pn":"Participant1","mn":"Peer/Sim1/Participant2/RemoteEndpoint","mk":"STRING_LIST","mv":["tcp://10.0.0.2:8500"]}

The exact set of emitted metrics depends on which SIL Kit services are used and which peer
connections are established during the simulation run.

.. _sec:cfg-registry-experimental:

Metrics for the registry
------------------------

The registry configuration supports collecting metrics from remote participants
and forwarding the collected data to the SIL Kit Dashboard for further
analysis and visualization.
Refer to the documentation of the `SIL Kit Dashboard <https://vector.com/sil-kit-dashboard>`_ for further instructions.

.. code-block:: yaml

    Experimental:
        Metrics:
          CollectFromRemote: true


.. list-table:: Registry Metrics Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description

   * - CollectFromRemote
     - Collect metrics from all connected participants. Defaults to ``true``.
