System Controller C API
-----------------------

.. contents::
   :local:
   :depth: 3

The features of the SystemController in the Cpp API are not provided through a SystemController abstraction 
in the C API but directly provided through the ib_Participant.

.. doxygenfunction:: ib_Participant_Restart
.. doxygenfunction:: ib_Participant_RunSimulation
.. doxygenfunction:: ib_Participant_StopSimulation
.. doxygenfunction:: ib_Participant_Shutdown
.. doxygenfunction:: ib_Participant_SetWorkflowConfiguration