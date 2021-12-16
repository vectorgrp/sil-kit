Synchronization
===================
.. macros for internal use
.. |ComAdapter| replace:: :doc:`ComAdapter<../api/comadapter>`
.. |Middleware| replace:: :doc:`Middleware<../configuration/middleware-configuration>`
.. |Fastrtps| replace:: :ref:`FastRTPS<sec:mwcfg-fastrtps>`
.. |Participant| replace:: :doc:`Participant<../api/participantcontroller>`
.. |SystemController| replace:: :cpp:class:`ISystemController<ib::mw::sim::ISystemController>`
.. |SystemMonitor| replace:: :cpp:class:`ISystemMonitor<ib::mw::sim::ISystemMonitor>`
.. |Running| replace:: :cpp:enumerator:`Running<ib::mw::sync::Running>`


.. contents::
    :local:
    :depth: 2

This document discusses the synchronization of the simulation time and how it is
affected by the underlying middleware.

The simulation time in the VIB only advances if the system is in the state
|Running|, and different synchronization algorithms ensure that simulation
time advances in a coordinated manner. These synchronization algorithms are
either governed by a dedicated ``SyncMaster`` or executed in a distributed way.

Synchronization is enabled on a per-participant base by configuring
a :ref:`sec:cfg-participant-controller`, and by using its :ref:`C++ API<sec:api-participant-controller>`.
In summary, the participant controller's SimTask callback is executed whenever
a new timestep is executed.

.. _sec:sim-time-sync:

Time Synchronization
--------------------

The Integration Bus supports several different algorithms to synchronize
simulation time, which can be configured using the :doc:`simulation
configuration<../configuration/simulation-setup>` and are listed in the
following table.

.. list-table:: Synchronization Types
    :widths: 30 70
    :header-rows: 1
    
    * - SyncType
      - Description

    * - DistributedTimeQuantum
      - Participants request simulation time for individual time periods
        (=quanta), which are negotiated between all simulation participants. 


When a participant is configured without a
:ref:`ParticipantController<sec:cfg-participant-controller>`, it will not take
part in synchronization. This is useful for participants such as the
SystemController or SystemMonitor, which only control the system state machine
or observe the simulation, but do not take part in the actual simulation.

Configuring the :ref:`ParticipantController<sec:cfg-participant-controller>`
instantiates the simulation algorithm internally.
As such, the synchronziation mechanism of the VIB is completely optional.

.. admonition:: Note

    For technical reasons, the `Unsynchronized` SyncType is currently defined in the Config
    API. This indicates the lack of a user-defined ParticipantController and is
    an invalid option for a user-defined `"ParticipantController"` configuration item.

Running participants without a participant controller will thus result in completely
uncoordinated execution.
Without the distributed states of participants and time synchronization,
the bus/service messages are delivered at a best effort base.
For example, if one participant starts sending, while some participants are not ready yet,
the sent messages will be lost.

Synchronization Policies
~~~~~~~~~~~~~~~~~~~~~~~~

.. admonition:: Note

    The synchronization policies only affect the |FastRTPS| middleware and are
    considered a legacy setting. If you require guaranteed message delivery
    before the next SimTask execution, it is recommended to use VAsio.

The synchronization policies, listed in the table below, define the guarantees
about data delivery.

.. list-table:: Synchronization Policies
    :widths: 30 70
    :header-rows: 1

    * - Policy
      - Description
    * - Loose
      - There is no guarantee that data has been received before the next
        simulation cycle (Default).
    * - Strict
      - Enforce that all sent data has been received before the next
        simulation cycle

The synchronization policy allows trading simulation performance off for
simulation accuracy. A ``loose`` policy allows running the simulation as fast as
possible, while minimizing the synchronization overhead.  The ``strict`` policy,
on the other hand, guarantees that data is received orderly before a new
simulation cycles starts.  This comes at the cost of a considerable slowdown,
when using the FastRTPS middleware.


FastRTPS Middleware with loose policy
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
When using the FastRTPS middleware, a ``loose`` synchronization policy can be configured.
FastRTPS does not ensure in-order delivery of messages.
Consider the following figure.
The communication of three participants (Part A, Part B, Part C) and the
progress of real time from left to right is depicted.
The simulation time is represented by the timeline of SyncMaster -- the ``Tick`` and
``TickDone`` points represent start and end of a simulation cycle of a
``DiscreteTime`` synchronization type.
Thick lines represent time that a simulation task (SimTask) is being processed.
The annotated milliseconds refer to the current simulation time cycle.
Thin arrows depict data communication between participants.
Dashed lines depict time synchronization messages:

.. figure:: ../_static/sim-fastrtps-loose.png
   :alt: FastRTPS with a loose policy
   :align: center
   :width: 90%

   FastRTPS with a Loose policy.

At the end of a simulation cycle all participants send a ``TickDone`` message to the
synchronization master.

There is no guarantee that sent messages are received before the next simulation task
(cf. :ref:`sec:sim-participant-lifecycle`) is executed.
For example, the messages ``A2`` and ``B2`` are received during the second SimTask
execution, allthough they have been sent in a previous ``tick`` of the simulation
time (yellow circle in the figure).


VAsio Middleware 
~~~~~~~~~~~~~~~~
The VAsio middleware guarantees message delivery to always be in-order.
This enables the usage of a distributed synchronization algorithm, which
is inherently strict.
The following figure shows the VAsio algorithm:


.. figure:: ../_static/sim-vasio-inorder-strict.png
   :alt: VAsio with a in-order, strict policy
   :align: center
   :width: 90%

   VAsio with strict, in-order delivery of messages.

The algorithm works by reporting the start time of the next due SimTask to all
other participants (``next@`` messages in the figure).
Based on this knowledge a participant knows when it is allowed to execute its next
SimTask. That is, when the earliest "foreign" SimTask is not earlier than its own
next SimTask.

VAsio is inherently strict because messages are delivered *in-order* and the
``next-SimTask`` message is delivered *in-line* with the data.
That is, when the ``next-SimTask`` message is received, it is guaranteed that all previous
data messages were received.

SetPeriod: Variable Simulation Period
*************************************

One advantage of VAsio is that a participant can decide to change its current
simulation period.
This affects the previously discussed algorithm by setting the duration of the ``next``
messages of a single participant.
For example, if a participant has no work to compute for the forseeable
(virtual) next time steps, it can change its simulation period.
This allows other participants to run up to the end of the new period, without
further synchronization.
Let us assume that we have two participants ``A`` and ``B``.
``A`` sets its period to ``1000ms`` and ``B`` sets it to ``200ms``.
After exchanging their ``next`` messages, B is now free to execute five of its
``SimTasks`` (that is, simulation periods) until it has to synchronize with ``A`` again.
Refer to the :cpp:func:`IParticipantController::SetPeriod<ib::mw::sync::IParticipantController::SetPeriod>`
method for details.
