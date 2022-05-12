FlexRay C API
-------

.. contents::
   :local:
   :depth: 3

The FlexRay API consists of the following parts:

* The FlexRay controller
* The FlexRay controller and TX buffer configuration
* A set of FlexRay specific messages, each with its own function to register callbacks: 
  Message, MessageAck, Wakeup, ControllerStatus, PocStatus, Symbol, SymbolAck, CycleStart
  
 
FlexRay Controller
~~~~~~~~~~~~~~~~~~
A FlexRay controller interacts with the configured FlexRay bus and sends FlexRay frames and other events on it's own behalf.
Note that it is not possible to explicitly send frames or other events, nor exist any API functions to construct these events.
This is because the FlexRay bus works completely time triggered, sending preconfigured frames.
To configure these frames, the API provides functions to manipulate TX buffers.

**The following functions can be used to create a FlexRay controller and manipulate it's configuration:**

.. doxygenfunction:: ib_Flexray_Controller_Create
.. doxygenfunction:: ib_Flexray_Controller_Configure
.. doxygenfunction:: ib_Flexray_Controller_ReconfigureTxBuffer
.. doxygenfunction:: ib_Flexray_Controller_UpdateTxBuffer

**The following function can be used to manipulate the controller's state by triggering chi commands:**

.. doxygenfunction:: ib_Flexray_Controller_ExecuteCmd

**The following set of functions can be used to register event handlers to the controller:**

.. doxygenfunction:: ib_Flexray_Controller_AddFrameHandler
.. doxygenfunction:: ib_Flexray_Controller_AddFrameTransmitHandler
.. doxygenfunction:: ib_Flexray_Controller_AddWakeupHandler
.. doxygenfunction:: ib_Flexray_Controller_AddPocStatusHandler
.. doxygenfunction:: ib_Flexray_Controller_AddSymbolHandler
.. doxygenfunction:: ib_Flexray_Controller_AddSymbolTransmitHandler
.. doxygenfunction:: ib_Flexray_Controller_AddCycleStartHandler

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: ib_Flexray_ClusterParameters
   :members:
.. doxygenstruct:: ib_Flexray_NodeParameters
   :members:
.. doxygenstruct:: ib_Flexray_TxBufferConfig
   :members:
.. doxygenstruct:: ib_Flexray_ControllerConfig
   :members:
.. doxygenstruct:: ib_Flexray_TxBufferUpdate
   :members:
.. doxygenstruct:: ib_Flexray_HostCommand
   :members:
.. doxygenstruct:: ib_Flexray_Header
   :members:
.. doxygenstruct:: ib_Flexray_Frame
   :members:

.. doxygenstruct:: ib_Flexray_FrameEvent
   :members:
.. doxygenstruct:: ib_Flexray_FrameTransmitEvent
   :members:
.. doxygenstruct:: ib_Flexray_SymbolEvent
   :members:
.. doxygenstruct:: ib_Flexray_CycleStartEvent
   :members:
.. doxygenstruct:: ib_Flexray_ControllerStatus
   :members:
.. doxygenstruct:: ib_Flexray_PocStatusEvent
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: ib_Flexray_MacroTick
.. doxygentypedef:: ib_Flexray_MicroTick
.. doxygentypedef:: ib_Flexray_ClockPeriod
.. doxygentypedef:: ib_Flexray_Channel
.. doxygentypedef:: ib_Flexray_SymbolPattern
.. doxygentypedef:: ib_Flexray_ChiCommand
.. doxygentypedef:: ib_Flexray_TransmissionMode
.. doxygentypedef:: ib_Flexray_PocState
.. doxygentypedef:: ib_Flexray_SlotModeType
.. doxygentypedef:: ib_Flexray_ErrorModeType
.. doxygentypedef:: ib_Flexray_StartupStateType
.. doxygentypedef:: ib_Flexray_WakeupStatusType
.. doxygentypedef:: ib_Flexray_Header_Flag

.. doxygentypedef:: ib_Flexray_SymbolTransmitEvent
