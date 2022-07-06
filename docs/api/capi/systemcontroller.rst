System Controller C API
-----------------------

.. contents::
   :local:
   :depth: 3

The features of the SystemController in the Cpp API are not provided through a SystemController abstraction 
in the C API but directly provided through the SilKit_Participant.

.. doxygenfunction:: SilKit_Participant_Restart
.. doxygenfunction:: SilKit_Participant_RunSimulation
.. doxygenfunction:: SilKit_Participant_StopSimulation
.. doxygenfunction:: SilKit_Participant_Shutdown
.. doxygenfunction:: SilKit_Participant_SetWorkflowConfiguration