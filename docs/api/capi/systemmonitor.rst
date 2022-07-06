System Monitor C API
---------------------

.. contents::
   :local:
   :depth: 3

The features of the SystemMonitor in the Cpp API are not provided through a SystemMonitor abstraction 
in the C API but directly provided through the SilKit_Participant.

.. doxygenfunction:: SilKit_Participant_AddSystemStateHandler
.. doxygenfunction:: SilKit_Participant_AddParticipantStatusHandler
.. doxygenfunction:: SilKit_Participant_RemoveSystemStateHandler
.. doxygenfunction:: SilKit_Participant_RemoveParticipantStatusHandler
.. doxygenfunction:: SilKit_Participant_GetParticipantState
.. doxygenfunction:: SilKit_Participant_GetSystemState