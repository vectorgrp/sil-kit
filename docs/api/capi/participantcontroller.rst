Participant Controller C API
----------------------------

.. contents::
   :local:
   :depth: 3

The features of the ParticipantController in the Cpp API are not provided through a ParticipantController abstraction 
in the C API but directly provided through the SilKit_Participant.

.. doxygenfunction:: SilKit_LifecycleService_SetCommunicationReadyHandler
.. doxygenfunction:: SilKit_LifecycleService_SetStartingHandler
.. doxygenfunction:: SilKit_LifecycleService_SetStopHandler
.. doxygenfunction:: SilKit_LifecycleService_SetShutdownHandler
.. doxygenfunction:: SilKit_LifecycleService_StartLifecycleNoSyncTime
.. doxygenfunction:: SilKit_LifecycleService_StartLifecycleWithSyncTime
.. doxygenfunction:: SilKit_LifecycleService_WaitForLifecycleToComplete
.. doxygenfunction:: SilKit_LifecycleService_Pause
.. doxygenfunction:: SilKit_LifecycleService_Continue

.. doxygenfunction:: SilKit_TimeSyncService_SetPeriod
.. doxygenfunction:: SilKit_TimeSyncService_SetSimulationTask
.. doxygenfunction:: SilKit_TimeSyncService_SetSimulationTaskAsync
.. doxygenfunction:: SilKit_TimeSyncService_CompleteSimulationTask