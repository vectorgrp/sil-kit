VIB Simulation
================================
.. macros for internal use
.. |ComAdapter| replace:: :doc:`ComAdapter<api/comadapter>`
.. |Middleware| replace:: :doc:`Middleware<configuration/middleware-configuration>`
.. |Fastrtps| replace:: :ref:`FastRTPS<sec:mwcfg-fastrtps>`
.. |Participant| replace:: :doc:`Participant<api/participantcontroller>`
.. |System| replace:: :doc:`System<api/synchronisation>`
.. |SystemController| replace:: :cpp:class:`ISystemController<ib::mw::sim::ISystemController>`
.. |SystemMonitor| replace:: :cpp:class:`ISystemMonitor<ib::mw::sim::ISystemMonitor>`
.. |SystemControllerExe| replace:: :ref:`VIB SystemController Utility<sec:util-registry>`

.. |Run| replace:: :cpp:func:`Run()<ib::mw::sync::IParticipantController::Run()>`


.. contents::
    :local:
    :depth: 2

This document discusses the distributed algorithm of the integration bus
and the simulation time synchronization.
The state machines which build the foundation for the distributed execution
are shared among all middleware implementations.

However, when the simulation is in a running state, the progress of the
simulation time is governed by the active middleware, and the selected
time synchronization algorithm and policy.


Distributed State Machine
-------------------------
The Integration Bus simulation relies on a distributed state machine.
The distributed algorithm takes each individual participant's state into
account and allows for computing a global system state and controlling
the simulation in its entirety.

The underlying |ComAdapter| and |Middleware| implement the low-level
protocol required for the exchange and synchronization of
state updates.
The protocol itself is stateless.
A user accesible API allows introspection of |Participant| and |System| states, and
also sending commands to transition the system or participants into new states.


The participant lifecycle
~~~~~~~~~~~~~~~~~~~~~~~~~~~
The participant's lifecycle consists mainly of three stages.
First, it needs to be initialized, then it will execute a simulation task, and
at last it will shutdown.
Failures will always lead to entering the ``Error`` state.

State changes are triggered by external commands, which are either issued
to particular participants (blue commands in the :ref:`figure below<fig-participant-states>`),
or to all participants of the simulation (green commands).

At creation time the participant's state will be ``Invalid``, and after a call
to |Run| it will transition into ``Idle``, at which point the participant
instance can be initialized.
The ``Initializing`` state should be used to properly set up
:doc:`services and controllers<api/api>` or do other initialization tasks
required for the operation of the simulation.
Reinitializing from an error or stopped state can also be done on a per participant
basis.

.. _fig-participant-states:

.. figure:: _static/ParticipantLifeCycle_1.png
   :alt: The participant controller life cycle
   :align: center
   :width: 80%
   
   The Participant Controller Life Cycle.

After successful initialization, the participants can all be set to
``Running`` using a system command.
Sending a ``Stopped`` or ``Shutdown`` command is also a system wide command that affects
more than one participant.

From a ``Stopped`` state the participant can be reinitialized, in contrast to the
``Shutdown`` state.
This allows for restarting a simulation and, e.g. saving simulation results in the
user's code after reaching the stopped state.

Allthough the |SystemController| interface can be used by every participant,  the
control of all participants of a simulation is usually delegated to a single
dedicated system controller.
The |SystemControllerExe| implements such a dedicated participant.


The system lifecycle
~~~~~~~~~~~~~~~~~~~~
The system states are a superset of the participant states.
That is, the system lifecycle is basically the same as that of an individual
participant.
With the exception that system states are only changed when all participants
are in an appropriate state.
For example, when the system ``Stop`` command is sent, the system state
``Stopping`` is entered.
The whole system remains in this state until all participants have finished
their simulation tasks and entered their ``Stopped`` states.
Only then the whole system transitions in to the ``Stopped`` state.
Thus, the additional intermediate states (marked green in the
:ref:`figure below<fig-system-states>`  ) have been added to model the period of
time until stable states are reached for all participants in the distributed
simulation.

.. _fig-system-states:

.. figure:: _static/SystemMonitor_1.png
   :alt: The SystemMonitor
   :align: center
   :width: 80%
   
   The System State Machine
    
The |SystemMonitor| API can be used to register callbacks to monitor for state
transitions of the system and individual participants.
There is also a :ref:`sec:util-system-monitor` utility, which prints participant
and system state updates, and is a handy tool to debug simulations.

Time Synchronization
--------------------
The Integration Bus supports several different simulation time synchronization
policies and algorithms.
Note that the simulation time only advances when system state is ``running``.
The :ref:`time synchronization<sec:cfg-time-sync>` and the
:ref:`synchronization policy<sec:cfg-participant-controller>` can be configured
using the :doc:`simulation configuration<../configuration/simulation-setup>`.

The available synchronization types are listed in the :ref:`table<sync-types>`
below.

.. _sync-types:

.. list-table:: Synchronization Types
    :widths: 30 70
    :header-rows: 1
    
    * - SyncType
      - Description

    * - DiscreteTime
      - Simulation advances according to clock "Ticks" generated by a time master,
        Participant sends a "TickDone" on completion.
            
    * - DiscreteTimePassive
      - Same as DiscreteTime, but the participant only listens to Ticks and does not
        send a "TickDone".

    * - DistributedTimeQuantum
      - TimeQuantum synchronization using a distributed algorithm. When using VAsio
        middleware, this SyncType provides inherent strict message delivery.

    * - TimeQuantum
      - Dynamic length time quanta requested by the participants.



Synchronization Policies
~~~~~~~~~~~~~~~~~~~~~~~~
.. admonition:: Note

    Note that the synchronization policies mainly affect the |FastRTPS| middleware and are
    considered a legacy setting.

The synchronization policies, listed in the table below, define the guarantees
about data delivery.


.. list-table:: Synchronization Policies
    :widths: 30 70
    :header-rows: 1

    * - Policy
      - Description
    * - Loose
      - There is no guarantee that data has been received before the next
        simulation cycle
    * - Strict
      - Enforce that all sent data has been received before the next
        simulation cycle

Choosing a ``loose`` policy allows running the simulation as fast as possible,
while minimizing the synchronization overhead.
The ``strict`` policy, on the other hand, guarantees that data is received
orderly before a new simulation cycles starts.
This comes at the cost of a considerable slowdown, when using the FastRTPS middleware.
