.. _sec:api-lifecycle-service:

==============================
Life Cycle Service
==============================
.. |ILifecycleService| replace:: :cpp:class:`ILifecycleService<ib::mw::sync::ILifecycleService>`
.. |Participant| replace:: :doc:`Participant<participant>`

.. contents::
    :local:
    :depth: 2

.. highlight:: cpp

The lifecycle service is the main interface to model a member of a simulation.
It provides access to the  :cpp:class:`time synchronization service <ib::mw::sync::ITimeSyncService>`, register callbacks for state changes, query the participant's state and issue commands to change the state.
For an overview of a participant's state and its relation to the simulation
refer to the :ref:`participant life cycle section<sec:sim-lifecycle-syncParticipants>`.

Using the Life Cycle Service
----------------------------

.. admonition:: Note
  
   The following describes the current behavior and will change in the foreseeable future.
   
An |ILifecycleService| instance can be retrieved from a connected |Participant|::
   
    auto* lifecycleService = participant->GetLifecycleService();

Participants can synchronize their time with other participants (see :ref:`synchronized simulation run<sec:sim-synchronization>` for details)::

    auto* timeSyncService = lifecycleService->GetTimeSyncService();
    (...)
    lifecycleService->ExecuteLifecycleWithTimeSync(...);

Alternatively, participants can run asynchronously (regarding the time synchronization)::
    lifecycleService->ExecuteLifecycleNoTimeSync(...);

The other callbacks, which are triggered on state transitions, are always executed
in the middleware's worker thread::

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

    lifecycleService->SetCommunicationReadyHandler(
       []() {
            std::cout << "Communication ready" << std::endl;
       }
    );

..
    If a participant does not use the virtual time synchronization, a separate callback informs about the transition to the Running state.
    This can be used to start local timers.

    lifecycleService->SetStartingHandler(
       []() {
            std::cout << "Starting" << std::endl;
       }
    );

The ``CommunicationReady`` handler should be used to intialize and configure :doc:`services and controllers<api>`. 

.. 
    TODO Further, it allows to exchange data via DataPublisher and DataSubscriber with other participants.


Controlling the Participant
"""""""""""""""""""""""""""
After a successful startup, the participant will enter the :cpp:enumerator:`Running<ib::mw::sync::Running>` state.
:cpp:func:`State()<ib::mw::sync::ILifecycleService::State()>` returns the
current state as a plain enumeration, whereas :cpp:func:`Status()<ib::mw::sync::ILifecycleService::Status()>`
returns additional information such as the participant's name, the human readable 
reason for entering the state, and the wall clock time when the state was entered.

To temporarily pause a simulation task, the :cpp:func:`Pause()<ib::mw::sync::ILifecycleService::Pause()>`
method can be invoked with a human readable explanation as a string argument.
Execution can be resumed using the :cpp:func:`Continue()<ib::mw::sync::ILifecycleService::Continue()>`
method.

To abort the simulation and report an error message use the 
:cpp:func:`ReportError()<ib::mw::sync::ILifecycleService::ReportError()>` method.
This will change the current participant state to :cpp:enumerator:`Error<ib::mw::sync::Error>` and report the error message
to the VIB runtime system.
ReportError is also called when the invocation of a registered handler throws an exception.

To stop a particular participant, use the :cpp:func:`Stop()<ib::mw::sync::ILifecycleService::Stop()>`
method.
This will exit the :cpp:func:`ExecuteLifecycleNoTimeSync<ib::mw::sync::ILifecycleService::ExecuteLifecycleNoTimeSync()>` (or :cpp:func:`ExecuteLifecycleWithTimeSync<ib::mw::sync::ILifecycleService::ExecuteLifecycleWithTimeSync()>`) loop,
call a registered StopHandler and switch to
the :cpp:enumerator:`Stopped<ib::mw::sync::Stopped>` state.

.. admonition:: Note

    Only use Stop() when the current participant's simulation cannot continue.
    To stop the simulation always use a system ``Stop`` command.
    **Calling ILifecycleService::Stop() will only change the current participant's
    state.**
    In general, other participants are not affected!

    The :ref:`sec:util-system-controller` utility monitors the states of all participants and
    sends an appropriate system-wide ``Stop`` command -- this is an implementation
    detail that should not be relied on.


API and Data Type Reference
-------------------------------

.. doxygenclass:: ib::mw::sync::ILifecycleService
   :members:

Usage Example
--------------
The following example is based on the ``IbCanDemo`` source code which is
distributed with the VIB, and slightly adapted for clarity.
It demonstrates how to setup a lifecycle service and register callbacks
to monitor participant state changes.

To demonstrate how to properly initialize other services, a can controller is 
initialized within the ``CommunicationReady`` callback of the lifecycle service.
This is the recommended way to set up controllers before first use.

.. literalinclude:: 
	examples/sync/LifecycleNoSyncSample.cpp
    :language: cpp
