=======================
System Monitor
=======================

.. contents::
   :local:
   :depth: 3


.. highlight:: cpp

Using the System Monitor
-------------------------

Each participant does have access to a System Monitor and can register callbacks to get informed about
changes of the ParticipantState or the SystemState that occur during the simulation.

Register callbacks for state transitions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To be notified about transitions of the ParticipantState, a ParticipantStateHandler has to be registered::

  // Register ParticipantStateHandler to receive ParticipantState transitions from all participants.
  auto participantStateHandler =
      [](sync::ParticipantState state) {};

  auto* systemMonitor = comAdapter->GetSystemMonitor();
  systemMonitor->RegisterParticipantStateHandler(participantStateHandler);

The ParticipantState changes are always called together with the
:cpp:class:`ParticipantStatus<ib::mw::sync::ParticipantStatus>` changes. The
:cpp:class:`ParticipantStatus<ib::mw::sync::ParticipantStatus>` provides access to some additional
information such as the name of the participant and the reason for the status change. To be notified about
transitions of the :cpp:class:`ParticipantStatus<ib::mw::sync::ParticipantStatus>`, a
ParticipantStatusHandler can be registered as follows::

  // Register ParticipantStatusHandler to receive ParticipantStatus transitions from all participants.
  auto participantStatusHandler =
      [](const sync::ParticipantStatus& participantStatus) {};

  auto* systemMonitor = comAdapter->GetSystemMonitor();
  systemMonitor->RegisterParticipantStatusHandler(participantStatusHandler);

Last but not least a SystemStateHandler can be registered to get informed about system state transitions::

  // Register SystemStateHandler to receive SystemState transitions.
  auto systemStateHandler =
      [](sync::SystemState state) {};

  auto* systemMonitor = comAdapter->GetSystemMonitor();
  systemMonitor->RegisterSystemStateHandler(systemStateHandler);


API and Data Type Reference
--------------------------------------------------

System Monitor API
~~~~~~~~~~~~~~~~~~~~
.. doxygenclass:: ib::mw::sync::ISystemMonitor
   :members:


Data Structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenstruct:: ib::mw::sync::ParticipantStatus
   :members:


Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenenum:: ib::mw::sync::ParticipantState

.. doxygenenum:: ib::mw::sync::SystemState


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
