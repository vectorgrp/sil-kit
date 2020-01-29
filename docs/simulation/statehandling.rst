State Handling
==================
.. macros for internal use
.. |ComAdapter| replace:: :doc:`ComAdapter<../api/comadapter>`
.. |Middleware| replace:: :doc:`Middleware<../configuration/middleware-configuration>`
.. |Fastrtps| replace:: :ref:`FastRTPS<sec:mwcfg-fastrtps>`
.. |Participant| replace:: :doc:`Participant<../api/participantcontroller>`
.. |ParticipantController| replace:: :doc:`participant controller<../api/participantcontroller>`
.. |System| replace:: :doc:`System<../api/synchronisation>`
.. |SystemController| replace:: :cpp:class:`ISystemController<ib::mw::sync::ISystemController>`
.. |SystemMonitor| replace:: :cpp:class:`ISystemMonitor<ib::mw::sync::ISystemMonitor>`
.. |SystemControllerExe| replace:: :ref:`VIB SystemController Utility<sec:util-registry>`

.. |Run| replace:: :cpp:func:`Run()<ib::mw::sync::IParticipantController::Run()>`


.. contents::
    :local:
    :depth: 2

This document discusses the state machine of the integration bus. Each
participant executes an individual state machine, and the states of all
participants are combined to a system view. Opposed to :doc:`synchronization`,
which coordinates the simulation time of participants, the state machine
operates at a more coarse grained level. Among other things, It allows starting,
stopping, and shutting down the simulation in an orchestrated manner.

The state machine is implemented by the |ParticipantController| and controlled
by commands sent by the |SystemController|. The |SystemMonitor| allows observing
the state of the system and the individual participants.


Distributed State Machine
-------------------------

The Integration Bus simulation relies on a distributed state machine.  The
distributed algorithm takes each individual participant's state into account to
compute a global system state, which allows controlling the simulation at a
larger scale.

A user accessible API allows introspection of
:cpp:enum:`participant<ib::mw::sync::ParticipantState>` and
:cpp:enum:`system<ib::mw::sync::SystemState>` states, and also sending
commands to transition the system or participants into new states.


.. _sec:sim-participant-lifecycle:

The Participant Lifecycle
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The participant's lifecycle can be broken down into four stages: initialization,
running, stop, and shut down. Transitions between those stages are initiated by
the SystemController.

For all phases, the |ParticipantController| allows setting callbacks that are
executed in that phase. The ``Init-``, ``Stop-``, and ``ShutdownHandlers`` are
executed when entering the new phase, and the configured simulation task
(SimTask) is executed repeatedly while the system is in the ``Running``
state. The following figure shows how the participant state machine operates.

.. _fig-participant-states:

.. figure:: ../_static/ParticipantStateMachine.png
   :alt: The participant controller life cycle
   :align: center
   :width: 80%
   
   The Participant Controller Life Cycle.

In this figure, blue arrows (and labels) indicate commands that are sent by the
SystemController to individual participants. I.e., participants can be
initialized independently. For the future, this is intended to allow individual
parameterization of each participant. Green arrows, on the other hand, are
system commands that are broadcast to all participants. All commands,
participant commands and system commands, are sent by the |SystemController|.

A participant enters the distributed state machine by either calling
:cpp:func:`Run()<ib::mw::sync::IParticipantController::Run()>` or
:cpp:func:`RunAsync()<ib::mw::sync::IParticipantController::RunAsync()>`. This
will cause the |ParticipantController| to anounce its state as ``Idle``,
indicating that it is ready for initialization. Before calling Run() or
RunAsync(), the participant state is unavailable to all other participants.

After all participants have been initialized, the simulation is started by
issuing the system command ``Run``. While the system is in the ``Running``
state, the participants repeatedly execute their registered SimTask while
advancement of time is governed by the configured :doc:`synchronization`
mechanism.

If a participant temporarily cannot advance the simulation, e.g., because a
debugger is attached to investigate its internal state, a participant can be put
into the ``Pause`` state.

The completion of a simulation run is initiated by the ``Stop`` command. A
registered StopHandler can be used to perform collection of simulation
results. Once all participants have successfully executed their StopHandler, and
the system is in state ``Stopped``, the system can either be ``Shutdown`` or
``ReInitialized`` for another simulation run. In the latter case, simulation
time is reset to zero.

Whenever a participant encounters an error from which it cannot recover, it can
switch to the ``Error`` state to indicate this situation to the system. To
provide more information about the cause, the
:cpp:func:`ReportError()<ib::mw::sync::IParticipantController::ReportError()>` allows
specifying a reason for the error. In some situations, a participant controller
automatically enters the error state, e.g., when an uncaught exception is thrown
in a callback. A participant can only recover from the ``Error`` state by two
ways: ReInitialization or Shutdown.

Allthough the |SystemController| interface can be used by every participant, the
control of all participants of a simulation is usually delegated to a single
dedicated system controller. The |SystemControllerExe| implements such a
dedicated participant for the most common usecase, which is executing a single
simulation run without restarting.


.. _sec:sim-system-lifecycle:

The System Lifecycle
~~~~~~~~~~~~~~~~~~~~

The |SystemMonitor| provides an aggregated view of all participant states in the
form of the ``SystemState``. This allows tracking and controlling the entire
simulation in a more convenient way.

By and large, the SystemState is computed as follows. If all participants are in
the same state, the system state will also be in the same state. E.g., if all
Participants are in the state ``Running``, the system state is ``Running``
too. The main exception to this rule are the ``Paused`` and ``Error`` states,
which can be regarded as *dominant* states. I.e., if already *one* participant
enters the ``Paused`` (or ``Error``) state, the system state will be regarded as
``Paused`` or ``Error`` as well.

The system state follows state transitions in a lazy manner. I.e., the system
state will remain the old state, until all participants have achieved the new
state. E.g., the system state will remain ``Initializing`` even if one or more
participants have already achieved the ``Initialized`` state.

In all cases that do not match any of the above, the system state will be
regarded as ``Invalid``. This should typically not occur.
    
The |SystemMonitor| API can be used to register callbacks to monitor for state
transitions of the system and individual participants.
There is also a :ref:`sec:util-system-monitor` utility, which prints participant
and system state updates, and is a handy tool to debug simulations.

