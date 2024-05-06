.. _chap:timesync-service-api:

============================
Time Synchronization Service
============================

.. |ProductName| replace:: SIL Kit

.. |ILifecycleService| replace:: :cpp:class:`ILifecycleService<SilKit::Services::Orchestration::ILifecycleService>`
.. |ITimeSyncService| replace:: :cpp:class:`ITimeSyncService<SilKit::Services::Orchestration::ITimeSyncService>`
.. |Participant| replace:: :doc:`Participant<participant>`

.. |SetSimulationStepHandler| replace:: :cpp:func:`SetSimulationStepHandler()<SilKit::Services::Orchestration::ITimeSyncService::SetSimulationStepHandler()>`
.. |SimulationStepHandler| replace:: :cpp:type:`SimulationStepHandler<SilKit::Services::Orchestration::SimulationStepHandler>`

.. |SetSimulationStepHandlerAsync| replace:: :cpp:func:`SetSimulationStepHandlerAsync()<SilKit::Services::Orchestration::ITimeSyncService::SetSimulationStepHandlerAsync()>`
.. |SimulationStepHandlerAsync| replace:: :cpp:type:`SimulationStepHandlerAsync<SilKit::Services::Orchestration::SimulationStepHandlerAsync>`
.. |CompleteSimulationStep| replace:: :cpp:func:`CompleteSimulationStep()<SilKit::Services::Orchestration::ITimeSyncService::CompleteSimulationStep()>`

.. |Now| replace:: :cpp:func:`Now()<SilKit::Services::Orchestration::ITimeSyncService::Now()>`

.. |RunningState| replace:: :cpp:enumerator:`Running<SilKit::Services::Orchestration::ParticipantState::Running>`

.. contents::
    :local:
    :depth: 2

.. highlight:: cpp

The time synchronization service is the main interface to model a participant's behavior in case virtual time synchronization shall be used.
It allows configuring a simulation step length and set the simulation step that will be executed repeatedly.

.. admonition:: Note

    Here, only a brief overview of the API is given, see :ref:`Virtual Time Synchronization<sec:sim-synchronization>` for more information on time synchronization with the |ProductName|.

Using the Time Synchronization Service
--------------------------------------

Once the time synchronization service was created, the participant will automatically partake in the virtual time synchronization.
Setting a simulation step, even an empty one, is mandatory for participants using the time synchronization service interface.
Please note that the lifecycle service and the time synchronization service can only be created once.
Calling these methods more than once throws an exception.
The following code creates a lifecycle service, a time synchronization service and defines the simulation step handler with the participant's step duration::

    SilKit::Services::Orchestration::LifecycleConfiguration lc
        {SilKit::Services::Orchestration::OperationMode::Coordinated};
    auto* lifecycleService = participant->CreateLifecycleService(lc);
    auto* timeSyncService = lifecycleService->CreateTimeSyncService();

    // Set simulation step to a lambda
    timeSyncService->SetSimulationStepHandler(
        [](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
            // Do simulation computation at timepoint 'now'
        }, 1ms
    );


Retrieving the Current Time
"""""""""""""""""""""""""""

After a successful startup according to the :ref:`Lifecycle State Machine<sec:sim-lifecycle-syncParticipants>`, the participant will enter the |RunningState| state.
The participant can now access the current simulation time using the |Now| method which will be equal to the time transported in the last simulation step handler.

Asynchronous Step Handler
"""""""""""""""""""""""""

In special cases, it may be required to synchronize an application thread with the execution of the simulation step.
That is, the application wants to execute some code during a simulation step, *but on a different thread* than where the simulation step is executing.
To achieve this, use |SetSimulationStepHandlerAsync| to assign the simulation step function that is triggered when the virtual time advances, and |CompleteSimulationStep| to manually trigger the end of the simulation step and continue the simulation.

.. admonition:: Note

    See :ref:`Blocking vs. Asynchronous Step Handler<subsubsec:sim-step-handlers>` for more details and the differences between the handler modes.

API Reference
-------------

.. doxygenclass:: SilKit::Services::Orchestration::ITimeSyncService
   :members:
