=======================
System Controller
=======================

.. contents::
   :local:
   :depth: 3


.. highlight:: cpp

Using the System Controller
-------------------------------

The system controller API can be used to control the simulation flow such as starting and stoping a simulation.
Each participant has access to the system controller API and is able to initiate a new system state.
But the simpler alternative is rather that one participant is solely responsible for using
the System Controller, so that no erroneous state changes occur and the transitions remain clear.

.. admonition:: Note

  The SIL Kit provides a utility called :ref:`sil-kit-system-controller<sec:util-system-controller>`, that provides a basic
  implementation that can be used to start a simulation. In most cases this utility can be used and no own 
  implementation is needed.

Before the system controller can be used to initiate state transisitions, 
:cpp:func:`SetWorkflowConfiguration()<SilKit::Services::Orchestration::ISystemController::SetWorkflowConfiguration()>` must be called
with a :cpp:class:`WorkflowConfiguration<SilKit::Services::Orchestration::WorkflowConfiguration>` containing the set of 
required participants within the simulation. This set of required participants must contain all participants that take part in the simulation.

Initiate state transitions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

After the required participants are set and the participants called 
:cpp:func:`StartLifecycleNoTimeSync<SilKit::Services::Orchestration::ILifecycleService::StartLifecycleNoTimeSync()>` or 
:cpp:func:`StartLifecycleWithTimeSync<SilKit::Services::Orchestration::ILifecycleService::StartLifecycleWithTimeSync()>`, 
the participant states will progress automatically either to :cpp:enumerator:`ParticipantState::ReadyToRun<SilKit::Services::Orchestration::ReadyToRun>` 
(for coordinated participants) or :cpp:enumerator:`ParticipantState::Running<SilKit::Services::Orchestration::Running>`  (for non-coordinated participants).

Once all required participants reached at least :cpp:enumerator:`ParticipantState::ReadyToRun<SilKit::Services::Orchestration::ReadyToRun>` 
(and therefore the system is in :cpp:enumerator:`SystemState::ReadyToRun<SilKit::Services::Orchestration::ReadyToRun>`), the next transition can be initiated
by calling the :cpp:func:`Run()<SilKit::Services::Orchestration::ISystemController::Run()>` command::

  // Initiate state transition from ReadyToRun to Running for all coordinated participants.
  auto* systemController = participant->GetSystemController();
  systemController->Run();

After all participants are successfully running and the system is in
:cpp:enumerator:`SystemState::Running<SilKit::Services::Orchestration::Running>`, the simulation can be stopped by calling
the :cpp:func:`Stop()<SilKit::Services::Orchestration::ISystemController::Stop()>` command::

  // Initiate state transition from Running to Stopped for all coordinated participants.
  auto* systemController = participant->GetSystemController();
  systemController->Stop();

If the system is in :cpp:enumerator:`SystemState::Stopped<SilKit::Services::Orchestration::Stopped>`, participants can either
be restart or the system can be shut down::

  // Restart a participant by providing its name.
  auto* systemController = participant->GetSystemController();
  systemController->Restart(participant.name);

  // Shut down all participants.
  auto* systemController = participant->GetSystemController();
  systemController->Shutdown();

After a participant is in :cpp:enumerator:`ParticipantState::Shutdown<SilKit::Services::Orchestration::Shutdown>`,
the simulation is considered to be completed and no more state transitions are possible.


API and Data Type Reference
--------------------------------------------------

System Controller API
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygenclass:: SilKit::Services::Orchestration::ISystemController
    :members:


Data Structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenstruct:: SilKit::Services::Orchestration::ParticipantCommand
    :members:

.. doxygenstruct:: SilKit::Services::Orchestration::SystemCommand
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
