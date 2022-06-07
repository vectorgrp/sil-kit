Participant Controller C API
----------------------------

.. contents::
   :local:
   :depth: 3

The features of the ParticipantController in the Cpp API are not provided through a ParticipantController abstraction 
in the C API but directly provided through the ib_Participant.

.. doxygenfunction:: ib_Participant_SetInitHandler
.. doxygenfunction:: ib_Participant_SetStopHandler
.. doxygenfunction:: ib_Participant_SetShutdownHandler
.. doxygenfunction:: ib_Participant_Run
.. doxygenfunction:: ib_Participant_RunAsync
.. doxygenfunction:: ib_Participant_WaitForRunAsyncToComplete
.. doxygenfunction:: ib_Participant_SetPeriod
.. doxygenfunction:: ib_Participant_SetSimulationTask
.. doxygenfunction:: ib_Participant_SetSimulationTaskAsync
.. doxygenfunction:: ib_Participant_CompleteSimulationTask
.. doxygenfunction:: ib_Participant_Pause
.. doxygenfunction:: ib_Participant_Continue