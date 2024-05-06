.. _chap:lifecycle-service-api:

==================
Lifecycle Service
==================

.. Macros for docs use

.. |Participant| replace:: :doc:`Participant<participant>`
.. |IParticipant| replace:: :cpp:class:`IParticipant<SilKit::IParticipant>`

.. |ITimeSyncService| replace:: :cpp:class:`ITimeSyncService<SilKit::Services::Orchestration::ITimeSyncService>`

.. |ILifecycleService| replace:: :cpp:class:`ILifecycleService<SilKit::Services::Orchestration::ILifecycleService>`
.. |StartLifecycle| replace:: :cpp:func:`StartLifecycle<SilKit::Services::Orchestration::ILifecycleService::StartLifecycle()>`
.. |CreateTimeSyncService| replace:: :cpp:func:`CreateTimeSyncService<SilKit::Services::Orchestration::ILifecycleService::CreateTimeSyncService()>`

.. |SetWorkflowConfiguration| replace:: :cpp:func:`ISystemController::SetWorkflowConfiguration()<SilKit::Experimental::Services::Orchestration::ISystemController::SetWorkflowConfiguration()>`
.. |AbortSimulation| replace:: :cpp:func:`ISystemController::AbortSimulation()<SilKit::Experimental::Services::Orchestration::ISystemController::AbortSimulation()>`

.. |Pause| replace:: :cpp:func:`Pause()<SilKit::Services::Orchestration::ILifecycleService::Pause()>`
.. |Continue| replace:: :cpp:func:`Continue()<SilKit::Services::Orchestration::ILifecycleService::Continue()>`
.. |Stop| replace:: :cpp:func:`Stop()<SilKit::Services::Orchestration::ILifecycleService::Stop()>`

.. |ReportError| replace:: :cpp:func:`ReportError()<SilKit::Services::Orchestration::ILifecycleService::ReportError()>`
.. |State| replace:: :cpp:func:`State()<SilKit::Services::Orchestration::ILifecycleService::State()>`
.. |Status| replace:: :cpp:func:`Status()<SilKit::Services::Orchestration::ILifecycleService::Status()>`

.. |CommunicationReadyHandler| replace:: :cpp:type:`CommunicationReadyHandler<SilKit::Services::Orchestration::CommunicationReadyHandler>`

.. |OperationMode| replace:: :cpp:enum:`OperationMode<SilKit::Services::Orchestration::OperationMode>`
.. |OperationMode_Coordinated| replace:: :cpp:enumerator:`OperationMode::Coordinated<SilKit::Services::Orchestration::OperationMode::Coordinated>`
.. |OperationMode_Autonomous| replace:: :cpp:enumerator:`OperationMode::Autonomous<SilKit::Services::Orchestration::OperationMode::Autonomous>`

.. |InvalidState| replace:: :cpp:enumerator:`Invalid<SilKit::Services::Orchestration::ParticipantState::Invalid>`
.. |ServicesCreatedState| replace:: :cpp:enumerator:`ServicesCreated<SilKit::Services::Orchestration::ParticipantState::ServicesCreated>`
.. |CommunicationInitializingState| replace:: :cpp:enumerator:`CommunicationInitializing<SilKit::Services::Orchestration::ParticipantState::CommunicationInitializing>`
.. |CommunicationInitializedState| replace:: :cpp:enumerator:`CommunicationInitialized<SilKit::Services::Orchestration::ParticipantState::CommunicationInitialized>`
.. |ReadyToRunState| replace:: :cpp:enumerator:`ReadyToRun<SilKit::Services::Orchestration::ParticipantState::ReadyToRun>`
.. |RunningState| replace:: :cpp:enumerator:`Running<SilKit::Services::Orchestration::ParticipantState::Running>`
.. |PausedState| replace:: :cpp:enumerator:`Paused<SilKit::Services::Orchestration::ParticipantState::Paused>`
.. |StoppingState| replace:: :cpp:enumerator:`Stopping<SilKit::Services::Orchestration::ParticipantState::Stopping>`
.. |StoppedState| replace:: :cpp:enumerator:`Stopped<SilKit::Services::Orchestration::ParticipantState::Stopped>`
.. |ErrorState| replace:: :cpp:enumerator:`Error<SilKit::Services::Orchestration::ParticipantState::Error>`
.. |ShuttingDownState| replace:: :cpp:enumerator:`ShuttingDown<SilKit::Services::Orchestration::ParticipantState::ShuttingDown>`
.. |ShutdownState| replace:: :cpp:enumerator:`Shutdown<SilKit::Services::Orchestration::ParticipantState::Shutdown>`
.. |AbortingState| replace:: :cpp:enumerator:`Aborting<SilKit::Services::Orchestration::ParticipantState::Aborting>`

.. |IDataPublisher| replace:: :cpp:class:`IDataPublisher<SilKit::Services::PubSub::IDataPublisher>`
.. |IDataSubscriber| replace:: :cpp:class:`IDataPublisher<SilKit::Services::PubSub::IDataSubscriber>`
.. |Publish| replace:: :cpp:func:`Publish()<SilKit::Services::PubSub::IDataPublisher::Publish()>`

.. contents::
    :local:
    :depth: 2

.. highlight:: cpp

The lifecycle service is used to orchestrate the workflow and states of a simulation participant.
It provides access to the |ITimeSyncService|, registers callbacks for state changes, queries the participant's state and issues commands to change the state.
For an overview of a participant's state and its relation to the simulation refer to the :ref:`participant lifecycle section<sec:sim-lifecycle-syncParticipants>`.

.. _subsec:sim-configuring-lifecycle:

Configuring the Lifecycle Service
---------------------------------

To create the lifecycle service, a valid lifecycle configuration must be provided.
Currently, only the mode that the lifecycle operates in can be configured.
There are two possible operation modes:

* Coordinated: 
  
  * The lifecycle service will coordinate its state with other participants.
    It will do so by reacting to changes of the system state, which is based on the list of required participants.
  * A coordinated participant will only work, if any participant set a list of required participants as part of |SetWorkflowConfiguration|.
  * Coordinated participants will stop, as soon as the system state changes to the stopping state.
  * Coordinated participants can also be terminated externally using the |AbortSimulation| call.

* Autonomous: 
  
  * An autonomous participant will not align its state to the system state or any participant state of other participants.
  * The lifecycle service will run through all participant states until it is running, and it will trigger all callbacks.
  * Autonomous participants must stop themselves by explicitly calling |Stop|.
  * The only way to stop an autonomous participant externally is the |AbortSimulation| call.

.. _subsec:sim-using-lifecycle:

Using the Lifecycle Service
----------------------------

An |ILifecycleService| instance can be retrieved from a |Participant|.
To do so, a lifecycle configuration must be provided::

    using SilKit::Services::Orchestration
    auto lifecycleConfig = LifecycleConfiguration{OperationMode::Coordinated};
    auto* lifecycleService = participant->CreateLifecycleService(lifecycleConfig);

.. admonition:: Note

    Please note that the lifecycle service and the time synchronization service can only be created once.
    Calling these methods more than once throws an exception.

Participants can synchronize their time with other participants (see :ref:`synchronized simulation run<sec:sim-synchronization>` for details)::

    auto* timeSyncService = lifecycleService->CreateTimeSyncService();
    (...)
    lifecycleService->StartLifecycle();

Alternatively, participants can run asynchronously (regarding the time synchronization).
This will happen if |CreateTimeSyncService| was not called before |StartLifecycle|.

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


If a participant does not use the virtual time synchronization, a separate callback informs about the transition to the ``Running`` state.
This can be used to start local timers::

    lifecycleService->SetStartingHandler(
        []() {
            std::cout << "Starting" << std::endl;
        }
    );

The |CommunicationReadyHandler| should be used to initialize and configure :doc:`services and controllers<api>`.

Communication guarantees
""""""""""""""""""""""""

The following communication guarantees apply to participants that utilize a Lifecycle Service and are limited to RPC and Publish/Subscribe services.
Bus systems have definitions on the protocol level for message acknowledgement.
For RPC or Publish/Subscribe however, communication guarantees depend on the implementation.
In the SIL Kit, the |CommunicationReadyHandler| handler is the first point at which guarantees can be given.
This implies that |StartLifecycle| has already been called.
Remote services that have been created at the point a participant calls |StartLifecycle| will be available for communication as soon as the |CommunicationReadyHandler| is triggered.
As an example, a call to |Publish| on a |IDataPublisher| on participant A in the |CommunicationReadyHandler| is guaranteed to arrive at 
participant B if its |IDataSubscriber| has been created before participant A has called |StartLifecycle|.

Communication in the |CommunicationReadyHandler| between participants that use the |OperationMode_Coordinated| is guaranteed.
After |StartLifecycle| is called, these participants will wait for each other until all reached the |ServicesCreatedState| state.
Therefore all services have already been created when the |CommunicationReadyHandler| is triggered.

A common use-case where the guarantee also applies is described as follows: A participant with |OperationMode_Autonomous| wants to join an already running simulation.
In this case, the services of the running participants have been created and the newly joining participant can communicate in the |CommunicationReadyHandler|.

If several participants are defined with |OperationMode_Autonomous| and are executed 'simultaneously' in a distributed setup, their intention is to proceed through the lifecycle states without coordination.
In this case, there is no deterministic execution order of controller creation and calls to |StartLifecycle| across the processes.
Therefore, it is impossible to guarantee that a remote subscriber already exists at the time a participants tries to |Publish| in the |CommunicationReadyHandler|.
Without the useage of a lifecycle, there is no way to synchronize controller creation across different participants.
In other words, controllers can appear at any time and there is no call to the SIL Kit API to build up a communication guarantee on.
In cases were such guarantees are indespensable, it is recommended to use a |ILifecycleService|.

Note that a |IDataPublisher| can be defined with a history (of length 1).
This guarantees the delivery of the last publication but is not coupled to the lifecycle states.

Another guarantee can be given for communication in the SimulationStepHandler.
All participants that take part in the distributed time algorithm are ready to exchange messages if the first SimulationStepHandler is triggered, regardless of their |OperationMode|.


Controlling the Participant
"""""""""""""""""""""""""""


After a successful startup, the participant will enter the |RunningState| state.
|State| returns the current state as a plain enumeration, whereas |Status| returns additional information such as the participant's name, the human-readable reason for entering the state, and the wall clock time when the state was entered.


To temporarily pause a simulation task, the |Pause| method can be invoked with a human-readable explanation as a string argument.
Execution can be resumed using the |Continue| method.


To abort the simulation and report an error message, use the |ReportError| method.
This will change the current participant state to the |ErrorState| state and report the error message to the SIL Kit runtime system.
|ReportError| is also called when the invocation of a registered handler throws an exception.

To stop a particular participant, use the |Stop| method.
This will exit |RunningState| or |PausedState|, call the registered ``StopHandler`` (if it was set) and switch to the |StoppedState| state.
Afterwards, the participant will shut down by first changing to the |ShuttingDownState| state, which triggers the shutdown handler (if it was set) and then finishing at the |ShutdownState| state.
At this point, the future provided by |StartLifecycle| will return.

API Reference
-------------

.. doxygenclass:: SilKit::Services::Orchestration::ILifecycleService
   :members:

Handlers
""""""""

.. doxygentypedef:: SilKit::Services::Orchestration::CommunicationReadyHandler
.. doxygentypedef:: SilKit::Services::Orchestration::StartingHandler
.. doxygentypedef:: SilKit::Services::Orchestration::StopHandler
.. doxygentypedef:: SilKit::Services::Orchestration::ShutdownHandler
.. doxygentypedef:: SilKit::Services::Orchestration::AbortHandler

Data Structures
"""""""""""""""

.. doxygenstruct:: SilKit::Services::Orchestration::LifecycleConfiguration
.. doxygenenum:: SilKit::Services::Orchestration::OperationMode

.. doxygenstruct:: SilKit::Services::Orchestration::WorkflowConfiguration
.. doxygenstruct:: SilKit::Services::Orchestration::ParticipantConnectionInformation

Usage Example
--------------

The following example is based on the ``SilKitCanDemo`` source code which is distributed with the SIL Kit, and slightly adapted for clarity.
It demonstrates how to setup a lifecycle service and register callbacks to monitor participant state changes.

To demonstrate how to properly initialize other services, a CAN controller is initialized within the ``CommunicationReady`` callback of the lifecycle service.
This is the recommended way to set up controllers before first use.

.. literalinclude:: 
	examples/sync/LifecycleNoSyncSample.cpp
    :language: cpp
