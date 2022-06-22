.. _sec:api-timesync-service:

============================
Time Synchronization Service
============================
.. |ILifecycleService| replace:: :cpp:class:`ILifecycleService<ib::mw::sync::ILifecycleService>`
.. |ITimeSyncService| replace:: :cpp:class:`ITimeSyncService<ib::mw::sync::ITimeSyncService>`
.. |Participant| replace:: :doc:`Participant<participant>`

.. contents::
    :local:
    :depth: 2

.. highlight:: cpp

The time synchronization service is the main interface to model the participant's behavior in case the virtual time synchronization shall be used.
It allows configuring the simulation step length and set the simulation task, that will be executed repeadetly.
For an overview of the time behavior see :ref:`synchronized simulation run<_sec:sim-synchronization>`.

Using the Time Synchronization Service
--------------------------------------

Setting a simulation task, even an empty one, is mandatory for participants using
the time synchronization service interface.
The simulation task will be executed asynchronously::    

    auto* lifecycleService = participant->GetLifecycleService();
    auto* timeSyncService = lifecycleService->GetTimeSyncService();

    //set simulation task to a lambda
    timeSyncService->SetSimulationTask(
        [](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
            // do simulation computation at timepoint 'now'
        }
    );



Controlling the Participant
"""""""""""""""""""""""""""

After a successful startup, the participant will enter the :cpp:enumerator:`Running<ib::mw::sync::Running>` state.
The participant can now access the current simulation time using the
:cpp:func:`Now()<ib::mw::sync::ITimeSyncService::Now()>` method.

Synchronizing an application thread with the simulation task
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

In special cases, it may be required to synchronize an application thread with the execution of the simulation task. 
That is, the application wants to execute some code between time steps (given by invocations of the simulation task), *but on a different thread*
than where the simulation task is executing.
To achieve this, use :cpp:func:`SetSimulationTaskAsync()<ib::mw::sync::ITimeSyncService::SetSimulationTaskAsync()>` to assign
the simulation task function, and :cpp:func:`CompleteSimulationTask()<ib::mw::sync::ITimeSyncService::CompleteSimulationTask()>` to let
the VIB continue the simulation. 
If the simulation task has been assigned using that function, execution will stop after the simulation task has finished executing.
By invoking :cpp:func:`CompleteSimulationTask()<ib::mw::sync::ITimeSyncService::CompleteSimulationTask()>` VIB's simulation loop 
(implemented in :cpp:func:`RunAsync<ib::mw::sync::ITimeSyncService::RunAsync()>`) will continue to the next time step.

Changing simulation task duration
""""""""""""""""""""""""""""""""""
Within the VIB, :cpp:func:`SetPeriod()<ib::mw::sync::ITimeSyncService::SetPeriod()>` specifies the simulation 
time duration of each simulation task invocation. It corresponds to the simulation time difference between each 
task execution.
The simulation task duration can be changed at any time by calling
:cpp:func:`SetPeriod()<ib::mw::sync::ITimeSyncService::SetPeriod()>`. When calling it within a simulation task, 
the duration of the current simulation task will already be affected by the call.

API and Data Type Reference
-------------------------------

.. doxygenclass:: ib::mw::sync::ITimeSyncService
   :members:

Usage Example
--------------
The following example is based on the ``IbCanDemo`` source code which is
distributed with the VIB, and slightly abridged for clarity.
It demonstrates how to setup a life cycle service and register callbacks
to monitor participant state changes.

To demonstrate how to properly initialize other services, a can controller is 
initialized within the ``CommunicationReady`` callback of the life cycle service.
This is the recommended way to set up controllers before first use.

Inside the simulation task callback, the controller's
:cpp:func:`SendFrame()<ib::sim::can::ICanController::SendFrame()>`
is used to transmit data frames to the simulated CAN bus.

.. literalinclude:: 
	examples/sync/LifecycleWithSyncSample.cpp
    :language: cpp
