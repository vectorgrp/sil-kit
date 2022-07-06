Participant Controller C API
----------------------------

.. contents::
   :local:
   :depth: 3

The features of the ParticipantController in the Cpp API are not provided through a ParticipantController abstraction 
in the C API but directly provided through the SilKit_Participant.

.. doxygenfunction:: SilKit_Participant_SetCommunicationReadyHandler
.. doxygenfunction:: SilKit_Participant_SetStopHandler
.. doxygenfunction:: SilKit_Participant_SetShutdownHandler
.. doxygenfunction:: SilKit_Participant_Run
.. doxygenfunction:: SilKit_Participant_RunAsync
.. doxygenfunction:: SilKit_Participant_WaitForRunAsyncToComplete
.. doxygenfunction:: SilKit_Participant_SetPeriod
.. doxygenfunction:: SilKit_Participant_SetSimulationTask
.. doxygenfunction:: SilKit_Participant_SetSimulationTaskAsync
.. doxygenfunction:: SilKit_Participant_CompleteSimulationTask
.. doxygenfunction:: SilKit_Participant_Pause
.. doxygenfunction:: SilKit_Participant_Continue