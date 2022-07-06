=======================
System Controller
=======================

.. contents::
   :local:
   :depth: 3


.. highlight:: cpp

Using the System Controller
-------------------------------

Each participant has access to the system controller and is able to initiate a new system state.
But the simpler alternative is rather that one participant is solely responsible for using
the System Controller, so that no erroneous state changes occur and the transitions remain clear.

.. admonition:: Note

  The SILKIT provides a utility called :ref:`SilKitSystemController<sec:util-system-controller>`, that provides a basic
  implementation that can be used to start a simulation. In most cases this utility can be used and no own 
  implementation is needed.

Before the system controller can be used to initiate state transisitions, 
:cpp:func:`SetWorkflowConfiguration()<SilKit::Core::Orchestration::ISystemController::SetWorkflowConfiguration()>` must be called
with a :cpp:class:`WorkflowConfiguration<SilKit::Core::Orchestration::WorkflowConfiguration>` containing the set of 
required participants within the simulation.

Initiate state transitions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

After the required participants are set and the participants called 
:cpp:func:`StartLifecycleNoTimeSync<SilKit::Core::Orchestration::ILifecycleService::StartLifecycleNoTimeSync()>` or 
:cpp:func:`StartLifecycleWithTimeSync<SilKit::Core::Orchestration::ILifecycleService::StartLifecycleWithTimeSync()>`, 
the participant states will progress automatically either to :cpp:enumerator:`ParticipantState::ReadyToRun<SilKit::Core::Orchestration::ReadyToRun>` 
(for coordinated participants) or :cpp:enumerator:`ParticipantState::Running<SilKit::Core::Orchestration::Running>`  (for non-coordinated participants).

Once all required participants reached at least :cpp:enumerator:`ParticipantState::ReadyToRun<SilKit::Core::Orchestration::ReadyToRun>` 
(and therefore the system is in :cpp:enumerator:`SystemState::ReadyToRun<SilKit::Core::Orchestration::ReadyToRun>`, the next transition can be initiated
by calling the :cpp:func:`Run()<SilKit::Core::Orchestration::ISystemController::Run()>` command::

  // Initiate state transition from ReadyToRun to Running for all coordinated participants.
  auto* systemController = participant->GetSystemController();
  systemController->Run();

After all participants are successfully running and the system is in
:cpp:enumerator:`SystemState::Running<SilKit::Core::Orchestration::Running>`, the simulation can be stopped by calling
the :cpp:func:`Stop()<SilKit::Core::Orchestration::ISystemController::Stop()>` command::

  // Initiate state transition from Running to Stopped for all coordinated participants.
  auto* systemController = participant->GetSystemController();
  systemController->Stop();

If the system is in :cpp:enumerator:`SystemState::Stopped<SilKit::Core::Orchestration::Stopped>`, participants can either
be restart or the system can be shut down::

  // Restart a participant by providing its name.
  auto* systemController = participant->GetSystemController();
  systemController->Restart(participant.name);

  // Shut down all participants.
  auto* systemController = participant->GetSystemController();
  systemController->Shutdown();

After a participant is in :cpp:enumerator:`ParticipantState::Shutdown<SilKit::Core::Orchestration::Shutdown>`,
the simulation is considered to be completed and no more state transitions are possible.


API and Data Type Reference
--------------------------------------------------

System Controller API
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygenclass:: SilKit::Core::Orchestration::ISystemController
    :members:


Data Structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenstruct:: SilKit::Core::Orchestration::ParticipantCommand
    :members:

.. doxygenstruct:: SilKit::Core::Orchestration::SystemCommand
    :members:


Usage Example
----------------------------------------------------

This section contains a basic example about the most common use of the System Controller.
It demonstrates a simple simulation from start to end with two participants, where one participant
uses the System Controller to control the system.
Although the participants would typically reside in different processes,
their interaction is shown sequentially to demonstrate cause and effect:

.. literalinclude::
   examples/sync/SystemControllerTwoParticipants.cpp
   :language: cpp
