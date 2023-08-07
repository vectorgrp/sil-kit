LIN C API
---------

.. contents::
   :local:
   :depth: 3

The LIN API for the C language provides communication in a LIN bus master/slave-architecture.
The functionality is analogous to the C++ API described in :ref:`sec:lin`.
  
LIN Controller
~~~~~~~~~~~~~~

**A LIN controller is created with the following function:**

.. doxygenfunction:: SilKit_LinController_Create

**A LIN controller's configuration is handed over to the function:**

.. doxygenfunction:: SilKit_LinController_Init

**The following set of functions can be used to add and remove event handlers on the controller:**

.. doxygenfunction:: SilKit_LinController_AddFrameStatusHandler
.. doxygenfunction:: SilKit_LinController_AddGoToSleepHandler
.. doxygenfunction:: SilKit_LinController_AddWakeupHandler
.. doxygenfunction:: SilKit_LinController_RemoveFrameStatusHandler
.. doxygenfunction:: SilKit_LinController_RemoveGoToSleepHandler
.. doxygenfunction:: SilKit_LinController_RemoveWakeupHandler

**The following functions operate on a configured controller:**

.. doxygenfunction:: SilKit_LinController_Status
.. doxygenfunction:: SilKit_LinController_SetFrameResponse
.. doxygenfunction:: SilKit_LinController_SendFrame
.. doxygenfunction:: SilKit_LinController_SendFrameHeader
.. doxygenfunction:: SilKit_LinController_GoToSleep
.. doxygenfunction:: SilKit_LinController_GoToSleepInternal
.. doxygenfunction:: SilKit_LinController_Wakeup
.. doxygenfunction:: SilKit_LinController_WakeupInternal

**The following functions are experimental and might be changed or removed in future versions:**

.. doxygenfunction:: SilKit_Experimental_LinController_AddLinSlaveConfigurationHandler
.. doxygenfunction:: SilKit_Experimental_LinController_RemoveLinSlaveConfigurationHandler
.. doxygenfunction:: SilKit_Experimental_LinController_GetSlaveConfiguration

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: SilKit_LinControllerConfig
   :members:
.. doxygenstruct:: SilKit_LinFrame
   :members:
.. doxygenstruct:: SilKit_LinFrameResponse
   :members:
.. doxygenstruct:: SilKit_Experimental_LinSlaveConfiguration
   :members:
.. doxygenstruct:: SilKit_LinFrameStatusEvent
   :members:
.. doxygenstruct:: SilKit_LinWakeupEvent
   :members:
.. doxygenstruct:: SilKit_LinGoToSleepEvent
   :members:

**The following data structures are experimental and might be changed or removed in future versions:**

.. doxygenstruct:: SilKit_Experimental_LinSlaveConfigurationEvent
   :members:

.. doxygentypedef:: SilKit_Experimental_LinSlaveConfigurationHandler_t

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: SilKit_LinController
.. doxygentypedef:: SilKit_LinControllerStatus
.. doxygentypedef:: SilKit_LinControllerMode
.. doxygentypedef:: SilKit_LinBaudRate
.. doxygentypedef:: SilKit_LinFrameResponseMode
.. doxygentypedef:: SilKit_LinId
.. doxygentypedef:: SilKit_LinChecksumModel
.. doxygentypedef:: SilKit_LinFrameResponseType
.. doxygentypedef:: SilKit_LinFrameStatus
.. doxygentypedef:: SilKit_LinDataLength

.. doxygentypedef:: SilKit_LinFrameStatusHandler_t
.. doxygentypedef:: SilKit_LinGoToSleepHandler_t
.. doxygentypedef:: SilKit_LinWakeupHandler_t
