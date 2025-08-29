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

.. list-table:: TimeSynchronization Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description

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

.. _sec:cfg-registry-experimental:

Metrics for the registry
------------------------

The registry configuration supports collecting metrics from remote participants
and forwarding the collected data to the SIL Kit Dashboard for further
analysis and visualization.
Refer to the documentation of the `Vector SIL Kit Dashboard <https://vector.com/sil-kit-dashboard>`_ for further instructions.

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