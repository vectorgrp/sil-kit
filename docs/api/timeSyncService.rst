.. _sec:api-timesync-service:

============================
Time Synchronization Service
============================
.. |ILifecycleService| replace:: :cpp:class:`ILifecycleService<SilKit::Services::Orchestration::ILifecycleService>`
.. |ITimeSyncService| replace:: :cpp:class:`ITimeSyncService<SilKit::Services::Orchestration::ITimeSyncService>`
.. |Participant| replace:: :doc:`Participant<participant>`

.. contents::
    :local:
    :depth: 2

.. highlight:: cpp

The time synchronization service is the main interface to model a participant's behavior in case virtual time synchronization shall be used.
It allows configuring a simulation step length and set the simulation task that will be executed repeatedly.
For an overview of the time behavior see :ref:`synchronized simulation run<sec:sim-synchronization>`.

Using the Time Synchronization Service
--------------------------------------

Setting a simulation task, even an empty one, is mandatory for participants using
the time synchronization service interface.
The simulation task will be executed asynchronously::

    auto* lifecycleService = participant->GetLifecycleService();
    auto* timeSyncService = lifecycleService->GetTimeSyncService();

    // Set simulation task to a lambda
    timeSyncService->SetSimulationStepHandler(
        [](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
            // Do simulation computation at timepoint 'now'
        }, 1ms
    );

Controlling the Participant
"""""""""""""""""""""""""""

After a successful startup, the participant will enter the :cpp:enumerator:`Running<SilKit::Services::Orchestration::Running>` state.
The participant can now access the current simulation time using the
:cpp:func:`Now()<SilKit::Services::Orchestration::ITimeSyncService::Now()>` method.

Synchronizing an Application Thread with the Simulation Task
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

In special cases, it may be required to synchronize an application thread with the execution of the simulation task. 
That is, the application wants to execute some code between time steps (given by invocations of the simulation task), *but on a different thread*
than where the simulation task is executing.
To achieve this, use :cpp:func:`SetSimulationStepHandlerAsync()<SilKit::Services::Orchestration::ITimeSyncService::SetSimulationStepHandlerAsync()>` to assign
the simulation task function, and :cpp:func:`CompleteSimulationTask()<SilKit::Services::Orchestration::ITimeSyncService::CompleteSimulationTask()>` to let
the SIL Kit continue the simulation. 
If the simulation task has been assigned using that function, execution will stop after the simulation task has finished executing.
By invoking :cpp:func:`CompleteSimulationTask()<SilKit::Services::Orchestration::ITimeSyncService::CompleteSimulationTask()>` SIL Kit's simulation loop 
(implemented in :cpp:func:`RunAsync<SilKit::Services::Orchestration::ITimeSyncService::RunAsync()>`) will continue to the next time step.

Changing Simulation Task Duration
"""""""""""""""""""""""""""""""""

Within the SIL Kit, :cpp:func:`SetPeriod()<SilKit::Services::Orchestration::ITimeSyncService::SetPeriod()>` specifies the simulation 
time duration of each simulation task invocation. It corresponds to the simulation time difference between each 
task execution.
The simulation task duration can be changed at any time by calling
:cpp:func:`SetPeriod()<SilKit::Services::Orchestration::ITimeSyncService::SetPeriod()>`. When calling it within a simulation task, 
the duration of the current simulation task will already be affected by the call.

API and Data Type Reference
---------------------------

.. doxygenclass:: SilKit::Services::Orchestration::ITimeSyncService
   :members:

Usage Example
-------------

The following example is based on the ``SilKitCanDemo`` source code which is
distributed with the SIL Kit, and slightly abridged for clarity.
It demonstrates how to setup a life cycle service and register callbacks
to monitor participant state changes.

To demonstrate how to properly initialize other services, a CAN controller is 
initialized within the ``CommunicationReady`` callback of the life cycle service.
This is the recommended way to set up controllers before first use.

Inside the simulation task callback, the controller's
:cpp:func:`SendFrame()<SilKit::Services::Can::ICanController::SendFrame()>`
is used to transmit data frames to the simulated CAN bus.

.. literalinclude:: 
   examples/sync/LifecycleWithSyncSample.cpp
   :language: cpp
