=======================
System Monitor
=======================

.. contents::
   :local:
   :depth: 3


.. highlight:: cpp

Using the System Monitor
------------------------

Each participant has access to a System Monitor and can register callbacks to get informed about
changes of the ``ParticipantState`` or the ``SystemState`` that occur during the simulation.

Register Callbacks for State Transitions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To be notified about transitions of the ``ParticipantState``, a ``ParticipantStatusHandler`` has to be registered. The
:cpp:class:`ParticipantStatus<SilKit::Services::Orchestration::ParticipantStatus>` contains the new ``ParticipantState`` and further details 
about the transition such as the name of the participant, the reason for the status change and timing information:

.. code-block:: c++

  // Register ParticipantStatusHandler to receive ParticipantStatus updates from all participants.
  auto participantStatusHandler =
      [](Orchestration::ParticipantStatus status) {};

  auto* systemMonitor = participant->GetSystemMonitor();
  systemMonitor->AddParticipantStatusHandler(participantStatusHandler);

A ``SystemStateHandler`` can be registered to get informed about system state transitions:

.. code-block:: c++

  // Register SystemStateHandler to receive SystemState transitions.
  auto systemStateHandler =
      [](Orchestration::SystemState state) {};

  auto* systemMonitor = participant->GetSystemMonitor();
  systemMonitor->AddSystemStateHandler(systemStateHandler);

Register Callbacks for Other Participants 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A :cpp:class:`ParticipantConnectedHandler` and :cpp:class:`ParticipantDisconnectedHandler` can be registered.
They report the name of any other participant connecting and disconnecting from the participant:

.. code-block:: c++

  auto* systemMonitor = participant->GetSystemMonitor();

  auto participantConnectedHandler = [](const std::string& participantName) { ... };
  systemMonitor->SetParticipantConnectedHandler(participantConnectedHandler);

  auto participantDisconnectedHandler = [](const std::string& participantName) { ... };
  systemMonitor->SetParticipantDisconnectedHandler(participantConnectedHandler);

Additionally, there is a function which checks if a participant identified by it's name is connected or not:

.. code-block:: c++

  auto* systemMonitor = participant->GetSystemMonitor();
  if (systemMonitor->IsParticipantConnected("SomeParticipant"))
  {
    // things to do only if the other participant is connected
  }

API and Data Type Reference
--------------------------------------------------

System Monitor API
~~~~~~~~~~~~~~~~~~
.. doxygenclass:: SilKit::Services::Orchestration::ISystemMonitor
   :members:


Data Structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenstruct:: SilKit::Services::Orchestration::ParticipantStatus
   :members:


Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenenum:: SilKit::Services::Orchestration::ParticipantState

.. doxygenenum:: SilKit::Services::Orchestration::SystemState


Usage Example
----------------------------------------------------

This section contains a complete example that shows the use of the System Monitor and
the order of callbacks in a complete state transition with two participants.
Although the participants would typically reside in
different processes, their interaction is shown sequentially to demonstrate cause and effect:

.. literalinclude::
   examples/sync/SystemMonitorTwoParticipants.cpp
   :language: cpp
   :lines: 1-29

This might lead to the following function call invocations at runtime:

.. literalinclude::
   examples/sync/SystemMonitorTwoParticipants.cpp
   :language: cpp
   :lines: 29-
