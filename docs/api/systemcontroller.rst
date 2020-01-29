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

Initiate state transitions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first transition performed by the system controller, usually when
:cpp:enumerator:`SystemState::Idle<ib::mw::sync::Idle>` is reached,
is accomplished by initializing all participants::

  // Initialize a participant by providing its id.
  auto* systemController = comAdapter->GetSystemController();
  systemController->Initialize(participant.id);

After all participants are successfully initialized and the system is in
:cpp:enumerator:`SystemState::Initialized<ib::mw::sync::Initialized>`, the next transition can be initiated
by calling the :cpp:func:`Run()<ib::mw::sync::ISystemController::Run()>` command::

  // Initiate state transition from Idle to Running for all participants.
  auto* systemController = comAdapter->GetSystemController();
  systemController->Run();

After all participants are successfully running and the system is in
:cpp:enumerator:`SystemState::Running<ib::mw::sync::Running>`, the simulation can be stopped by calling
the :cpp:func:`Stop()<ib::mw::sync::ISystemController::Stop()>` command::

  // Initiate state transition from Running to Stopped for all participants.
  auto* systemController = comAdapter->GetSystemController();
  systemController->Stop();

If the system is in :cpp:enumerator:`SystemState::Stopped<ib::mw::sync::Stopped>`, participants can either
be reinitialize or the system can be shut down::

  // Reinitialize a participant by providing its id.
  auto* systemController = comAdapter->GetSystemController();
  systemController->ReInitialize(participant.id);

  // Shut down all participants.
  auto* systemController = comAdapter->GetSystemController();
  systemController->Shutdown();

After a participant is in :cpp:enumerator:`ParticipantState::Shutdown<ib::mw::sync::Shutdown>`,
the simulation is considered to be completed and no more state transitions are possible.


API and Data Type Reference
--------------------------------------------------

System Controller API
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygenclass:: ib::mw::sync::ISystemController
    :members:


Data Structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenstruct:: ib::mw::sync::ParticipantCommand
    :members:

.. doxygenstruct:: ib::mw::sync::SystemCommand
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