.. _sec:api-lifecycle-service:

==================
Lifecycle Service
==================
.. |ILifecycleService| replace:: :cpp:class:`ILifecycleService<SilKit::Services::Orchestration::ILifecycleService>`
.. |Participant| replace:: :doc:`Participant<participant>`

.. contents::
    :local:
    :depth: 2

.. highlight:: cpp

The lifecycle service is used to orchestrate the workflow and states of a simulation participant.
It provides access to the :cpp:class:`time synchronization service <SilKit::Services::Orchestration::ITimeSyncService>`
, registers callbacks for state changes, queries the participant's state and issues commands to change the state.
For an overview of a participant's state and its relation to the simulation
refer to the :ref:`participant lifecycle section<sec:sim-lifecycle-syncParticipants>`.

.. _subsec:sim-configuring-lifecycle:

Configuring the Lifecycle Service
---------------------------------

To create the lifecycle service, a valid lifecycle configuration must be provided.
Currently, only the mode that the lifecycle operates in can be configured.
There are two possible operation modi:

* Coordinated: 
  
  * The lifecycle service will coordinate its state with other participants. It will do so by reacting to changes of the system state, which is based on the list of required participants.
  * A coordinated participant will only work, if any participant set a list of required participants as part of :cpp:func:`ISystemController::SetWorkflowConfiguration()<SilKit::Experimental::Services::Orchestration::ISystemController::SetWorkflowConfiguration()>`.
  * Coordinated participants will stop, as soon as the system state changes to the stopping state.
  * The `CommunicationReady` callback guarantees that communication via RPC and Publish/Subscribe services is possible.
  * Coordinated participants can also be terminated externally using the :cpp:func:`ISystemController::AbortSimulation()<SilKit::Experimental::Services::Orchestration::ISystemController::AbortSimulation()>` call.
  
* Autonomous: 
  
  * An autonomous participant will not align its state to the system state or any participant state of other participants.
  * The lifecycle service will run through all participant states until it is running and it will trigger all callbacks.
  * The `CommunicationReady` callback will trigger, but the communication guarantee is not given.
  * Autonomous participants must stop themselves by explicitly calling :cpp:func:`Stop()<SilKit::Services::Orchestration::ILifecycleService::Stop()>`.
  * The only way to stop an autonomous participant externally is the :cpp:func:`ISystemController::AbortSimulation()<SilKit::Experimental::Services::Orchestration::ISystemController::AbortSimulation()>` call.

.. _subsec:sim-using-lifecycle:

Using the Lifecycle Service
----------------------------

An |ILifecycleService| instance can be retrieved from a |Participant|. To do so, a lifecycle configuration must be provided::
   
    SilKit::Services::Orchestration::LifecycleConfiguration lc{
        SilKit::Services::Orchestration::OperationMode::Coordinated};
    auto* lifecycleService = participant->CreateLifecycleService(lc);

.. admonition:: Note

    Please note that the lifecycle service and the time synchronization service can only be created once. 
    Calling these methods more than once throws an exception.

Participants can synchronize their time with other participants (see :ref:`synchronized simulation run<sec:sim-synchronization>` for details)::

    auto* timeSyncService = lifecycleService->CreateTimeSyncService();
    (...)
    lifecycleService->StartLifecycle();

Alternatively, participants can run asynchronously (regarding the time synchronization). 
This will happen if ``CreateTimeSyncService()`` was not called before ``StartLifecycleService()``.

There are several callbacks, which are triggered on state transitions. 
They are always executed in the middleware's worker thread::

    lifecycleService->SetCommunicationReadyHandler(
       []() {
            std::cout << "Communication ready" << std::endl;
       }
    );

    lifecycleService->SetStopHandler(
        []() {
            std::cout << "Stopping" << std::endl;
        }
    );

    lifecycleService->SetShutdownHandler(
        []() {
            std::cout << "Shutting down" << std::endl;
        }
    );

    lifecycleService->SetAbortHandler(
        [](auto /*participantState*/) {
            std::cout << "Simulation is aborting" << std::endl;
        }
    );


If a participant does not use the virtual time synchronization, a separate callback informs about the transition to the Running state.
This can be used to start local timers::

    lifecycleService->SetStartingHandler(
        []() {
            std::cout << "Starting" << std::endl;
        }
    );

The ``CommunicationReady`` handler should be used to intialize and configure :doc:`services and controllers<api>`. 


Controlling the Participant
"""""""""""""""""""""""""""
After a successful startup, the participant will enter the :cpp:enumerator:`Running<SilKit::Services::Orchestration::Running>` state.
:cpp:func:`State()<SilKit::Services::Orchestration::ILifecycleService::State()>` returns the
current state as a plain enumeration, whereas :cpp:func:`Status()<SilKit::Services::Orchestration::ILifecycleService::Status()>`
returns additional information such as the participant's name, the human readable 
reason for entering the state, and the wall clock time when the state was entered.

To temporarily pause a simulation task, the :cpp:func:`Pause()<SilKit::Services::Orchestration::ILifecycleService::Pause()>`
method can be invoked with a human readable explanation as a string argument.
Execution can be resumed using the :cpp:func:`Continue()<SilKit::Services::Orchestration::ILifecycleService::Continue()>`
method.

To abort the simulation and report an error message, use the 
:cpp:func:`ReportError()<SilKit::Services::Orchestration::ILifecycleService::ReportError()>` method.
This will change the current participant state to :cpp:enumerator:`Error<SilKit::Services::Orchestration::Error>` and report the error message
to the SIL Kit runtime system. ReportError is also called when the invocation of a registered handler throws an exception.

To stop a particular participant, use the :cpp:func:`Stop()<SilKit::Services::Orchestration::ILifecycleService::Stop()>`
method. This will exit the :cpp:func:`StartLifecycle<SilKit::Services::Orchestration::ILifecycleService::StartLifecycle()>` loop,
call the registered StopHandler (if it was set) and switch to the :cpp:enumerator:`Stopped<SilKit::Services::Orchestration::Stopped>` state.
Afterwards, the participant will shut down by first changing to the :cpp:enumerator:`ShuttingDown<SilKit::Services::Orchestration::ShuttingDown>` state, 
which triggers the shutdown handler (if it was set) and then finishing at the :cpp:enumerator:`Shutdown<SilKit::Services::Orchestration::Shutdown>` state.
At this point, the future provided by :cpp:func:`StartLifecycle()<SilKit::Services::Orchestration::ILifecycleService::StartLifecycle()>` will return.

API and Data Type Reference
-------------------------------

.. doxygenclass:: SilKit::Services::Orchestration::ILifecycleService
   :members:

Usage Example
--------------
The following example is based on the ``SilKitCanDemo`` source code which is
distributed with the SIL Kit, and slightly adapted for clarity.
It demonstrates how to setup a lifecycle service and register callbacks
to monitor participant state changes.

To demonstrate how to properly initialize other services, a can controller is 
initialized within the ``CommunicationReady`` callback of the lifecycle service.
This is the recommended way to set up controllers before first use.

.. literalinclude:: 
	examples/sync/LifecycleNoSyncSample.cpp
    :language: cpp
