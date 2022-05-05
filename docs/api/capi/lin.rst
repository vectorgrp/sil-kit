LIN C API
-------

.. contents::
   :local:
   :depth: 3

The Lin API for the C language provides communication in a Lin-Bus master/slave-architecture. 
The functionality is analogous to the C++ API described in :ref:`sec:lin`.
  
Lin Controller
~~~~~~~~~~~~~~

**A Lin controller is created with the following function:**

.. doxygenfunction:: ib_Lin_Controller_Create

**It's configuration is handed over to the function:**

.. doxygenfunction:: ib_Lin_Controller_Init

**The following set of functions can be used to register event handlers on the controller:**

.. doxygenfunction:: ib_Lin_Controller_AddFrameStatusHandler
.. doxygenfunction:: ib_Lin_Controller_AddGoToSleepHandler
.. doxygenfunction:: ib_Lin_Controller_AddWakeupHandler

**The following functions operate on a configured controller:**

.. doxygenfunction:: ib_Lin_Controller_Status
.. doxygenfunction:: ib_Lin_Controller_SendFrame
.. doxygenfunction:: ib_Lin_Controller_SendFrameHeader
.. doxygenfunction:: ib_Lin_Controller_SetFrameResponse
.. doxygenfunction:: ib_Lin_Controller_SetFrameResponses
.. doxygenfunction:: ib_Lin_Controller_GoToSleep
.. doxygenfunction:: ib_Lin_Controller_GoToSleepInternal
.. doxygenfunction:: ib_Lin_Controller_Wakeup
.. doxygenfunction:: ib_Lin_Controller_WakeupInternal

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: ib_Lin_ControllerConfig
   :members:
.. doxygenstruct:: ib_Lin_Frame
   :members:
.. doxygenstruct:: ib_Lin_FrameResponse
   :members:
.. doxygenstruct:: ib_Lin_FrameStatusEvent
   :members:
.. doxygenstruct:: ib_Lin_WakeupEvent
   :members:
.. doxygenstruct:: ib_Lin_GoToSleepEvent
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: ib_Lin_Controller
.. doxygentypedef:: ib_Lin_ControllerStatus
.. doxygentypedef:: ib_Lin_ControllerMode
.. doxygentypedef:: ib_Lin_BaudRate
.. doxygentypedef:: ib_Lin_FrameResponseMode
.. doxygentypedef:: ib_Lin_Id
.. doxygentypedef:: ib_Lin_ChecksumModel
.. doxygentypedef:: ib_Lin_FrameResponseType
.. doxygentypedef:: ib_Lin_FrameStatus
.. doxygentypedef:: ib_Lin_DataLength

.. doxygentypedef:: ib_Lin_FrameStatusHandler_t
.. doxygentypedef:: ib_Lin_GoToSleepHandler_t
.. doxygentypedef:: ib_Lin_WakeupHandler_t

The Logger API can be used to write log messages.

.. doxygenfunction:: ib_Participant_GetLogger
.. doxygenfunction:: ib_Logger_Log

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: ib_LoggingLevel