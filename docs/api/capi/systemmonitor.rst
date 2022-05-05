System Monitor C API
---------------------

.. contents::
   :local:
   :depth: 3

The features of the SystemMonitor in the Cpp API are not provided through a SystemMonitor abstraction 
in the C API but directly provided through the ib_Participant.

.. doxygenfunction:: ib_Participant_RegisterSystemStateHandler
.. doxygenfunction:: ib_Participant_RegisterParticipantStatusHandler
.. doxygenfunction:: ib_Participant_GetParticipantState
.. doxygenfunction:: ib_Participant_GetSystemState