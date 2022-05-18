.. _sec:api-participant-controller:

==============================
Participant Controller
==============================
.. |IParticipantController| replace:: :cpp:class:`IParticipantController<ib::mw::sync::IParticipantController>`
.. |Participant| replace:: :doc:`Participant<participant>`

.. contents::
    :local:
    :depth: 2

.. highlight:: cpp

The Participant Controller is the main interface to model a member of a simulation.
It allows to set a simulation task, query the state, issuing commands, and to register callbacks
for state changes.
For an overview of a participants state and its relation to the simulation
refer to the :ref:`participant lifecycle section<sec:sim-lifecycle-syncParticipants>`.

Using the Participant Controller
-------------------------------------
An |IParticipantController| instance can be retrieved from a connected |Participant|::
   
    auto* participantController = participant->GetParticipantController();

Setting a simulation task, even an empty one, is mandatory for participants using
the participant controller interface.
The simulation task will be executed in the context of the thread that invokes the
:cpp:func:`Run()<ib::mw::sync::IParticipantController::Run()>` or in the context of the
middleware thread if :cpp:func:`RunAsync()<ib::mw::sync::IParticipantController::RunAsync()>`
is used::

    //set simulation task to a lambda
    participantController->SetSimulationTask(
        [](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
            // do simulation computation at timepoint 'now'
        }
    );
    participantController->Run();

The other callbacks, which are triggered on state transitions, are always executed
in the middleware's worker thread::

    participantController->SetStopHandler(
        []() {
            std::cout << "Stopping" << std::endl;
        }
    );


    participantController->SetShutdownHandler(
        []() {
            std::cout << "Shutting down" << std::endl;
        }
    );

    participantController->SetInitHandler(
       [](auto initCommand) {
           switch(initCommand.kind) {
           case ParticipantCommand::Kind::Initialize:
               std::cout << "Initialize" << std::endl; break;
           case ParticipantCommand::Kind::Reinitialize:
               std::cout << "Re-intializing" << std::endl; break;
           }
       }
    );

The ``Init`` handler should be used to intialize and configure other
:doc:`services and controllers<api>` before first use in the running state.


Controlling the Participant
"""""""""""""""""""""""""""
After successful initialization, the participant will enter the :cpp:enumerator:`Running<ib::mw::sync::Running>` state.
The participant can now access the current simulation time using the
:cpp:func:`Now()<ib::mw::sync::IParticipantController::Now()>` method.
:cpp:func:`State()<ib::mw::sync::IParticipantController::State()>` returns the
current state as a plain enumeration, whereas :cpp:func:`Status()<ib::mw::sync::IParticipantController::Status()>`
returns additional information such as the participant's name, the human readable 
reason for entering the state, and the wall clock time when the state was entered.
To update the status' time stamp, use :cpp:func:`RefreshStatus()<ib::mw::sync::IParticipantController::RefreshStatus()>`.
This will signal other participants that the current participant is still alive and operational.


To temporarily pause a simulation task, the :cpp:func:`Pause()<ib::mw::sync::IParticipantController::Pause()>`
method can be invoked with a human readable explanation as a string argument.
Execution can be resumed using the :cpp:func:`Continue()<ib::mw::sync::IParticipantController::Continue()>`
method.

To abort the simulation and report an error message use the 
:cpp:func:`ReportError()<ib::mw::sync::IParticipantController::ReportError()>` method.
This will change the current participant state to :cpp:enumerator:`Error<ib::mw::sync::Error>` and report the error message
to the VIB runtime system.
ReportError is also called when the invocation of a registered handler throws an exception.

To stop a particular participant, use the :cpp:func:`Stop()<ib::mw::sync::IParticipantController::Stop()>`
method.
This will exit the :cpp:func:`RunAsync<ib::mw::sync::IParticipantController::RunAsync()>` loop,
call a registered StopHandler and switch to
the :cpp:enumerator:`Stopped<ib::mw::sync::Stopped>` state.

.. admonition:: Note

    Only use Stop() when the current participant's simulation cannot continue.
    To stop the simulation always use a system ``Stop`` command.
    **Calling IParticipantController::Stop() will only change the current participant's
    state.**
    In general, other participants are not affected!

    The :ref:`sec:util-system-controller` utility monitors the states of all participants and
    sends an appropriate system-wide ``Stop`` command -- this is an implementation
    detail that should not be relied on.

Synchronizing an application thread with the simulation task
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

In special cases, it may be required to synchronize some application thread with the 
execution of the simulation task. That is, the application wants to execute some code
between time steps (given by invocations of the simulation task), *but on a different thread*
than where the simulation task is executing.
To achieve this, use :cpp:func:`SetSimulationTaskAsync()<ib::mw::sync::IParticipantController::SetSimulationTaskAsync()>` to assign
the simulation task function, and :cpp:func:`CompleteSimulationTask()<ib::mw::sync::IParticipantController::CompleteSimulationTask()>` to let
the VIB continue the simulation. If the simulation task has been assigned using that function, execution will stop after the simulation task has finished executing.
By invoking :cpp:func:`CompleteSimulationTask()<ib::mw::sync::IParticipantController::CompleteSimulationTask()>` VIB's simulation loop 
(implemented in :cpp:func:`RunAsync<ib::mw::sync::IParticipantController::RunAsync()>`) will continue to the next time step.


Changing simulation task duration
""""""""""""""""""""""""""""""""""
Within the VIB, :cpp:func:`SetPeriod()<ib::mw::sync::IParticipantController::SetPeriod()>` specifies the simulation 
time duration of each simulation task invocation. It corresponds to the simulation time difference between each 
task execution.
The simulation task duration can be changed at any time by calling
:cpp:func:`SetPeriod()<ib::mw::sync::IParticipantController::SetPeriod()>`. When calling it within a simulation task, 
the duration of the current simulation task will already be affected by the call.

Coldswap Feature
""""""""""""""""""""""
It is possible to disconnect and reconnect individual participants between
consecutive simulation runs. This is referred to as a cold swap. If a
participant is
:cpp:func:`enabled<ib::mw::sync::IParticipantController::EnableColdswap()>` for
cold swap, the participant can shutdown when executing a cold swap / restart
routine. By default, this feature is disabled. The actual cold swapping is
initiated by the
:cpp:func:`SystemController<ib::mw::sync::ISystemController::PrepareColdswap()>`.

API and Data Type Reference
-------------------------------

.. doxygenclass:: ib::mw::sync::IParticipantController
   :members:

Usage Example
--------------
The following example is based on the ``IbCanDemo`` source code which is
distributed with the VIB, and slightly abridged for clarity.
It demonstrates how to setup a participant controller and register callbacks
to monitor participant state changes.

To demonstrate how to properly initialize other services, a can controller is 
initialized within the ``Init`` callback of the participant controller.
This is the recommended way to set up controllers before first use.

Inside the simulation task callback, the controller's
:cpp:func:`SendIbMessage()<ib::sim::can::ICanController::SendIbMessage()>`
is used to transmit data frames to the simulated CAN bus.

.. literalinclude:: 
	examples/sync/ParticipantControllerSample.cpp
    :language: cpp


