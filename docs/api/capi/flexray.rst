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

.. doxygenfunction:: ib_FlexRay_Controller_Create
.. doxygenfunction:: ib_FlexRay_Append_TxBufferConfig
.. doxygenfunction:: ib_FlexRay_Controller_Configure
.. doxygenfunction:: ib_FlexRay_Controller_ReconfigureTxBuffer
.. doxygenfunction:: ib_FlexRay_Controller_UpdateTxBuffer

**The following function can be used to manipulate the controller's state by triggering chi commands:**

.. doxygenfunction:: ib_FlexRay_Controller_ExecuteCmd

**The following set of functions can be used to register event handlers to the controller:**

.. doxygenfunction:: ib_FlexRay_Controller_RegisterMessageHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterMessageAckHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterWakeupHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterPocStatusHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterSymbolHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterSymbolAckHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterCycleStartHandler

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: ib_FlexRay_ClusterParameters
   :members:
.. doxygenstruct:: ib_FlexRay_NodeParameters
   :members:
.. doxygenstruct:: ib_FlexRay_TxBufferConfig
   :members:
.. doxygenstruct:: ib_FlexRay_ControllerConfig
   :members:
.. doxygenstruct:: ib_FlexRay_TxBufferUpdate
   :members:
.. doxygenstruct:: ib_FlexRay_HostCommand
   :members:
.. doxygenstruct:: ib_FlexRay_Header
   :members:
.. doxygenstruct:: ib_FlexRay_Frame
   :members:

.. doxygenstruct:: ib_FlexRay_Message
   :members:
.. doxygenstruct:: ib_FlexRay_MessageAck
   :members:
.. doxygenstruct:: ib_FlexRay_Symbol
   :members:
.. doxygenstruct:: ib_FlexRay_CycleStart
   :members:
.. doxygenstruct:: ib_FlexRay_ControllerStatus
   :members:
.. doxygenstruct:: ib_FlexRay_PocStatus
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: ib_FlexRay_MacroTick
.. doxygentypedef:: ib_FlexRay_MicroTick
.. doxygentypedef:: ib_FlexRay_ClockPeriod
.. doxygentypedef:: ib_FlexRay_Channel
.. doxygentypedef:: ib_FlexRay_SymbolPattern
.. doxygentypedef:: ib_FlexRay_ChiCommand
.. doxygentypedef:: ib_FlexRay_TransmissionMode
.. doxygentypedef:: ib_FlexRay_PocState
.. doxygentypedef:: ib_FlexRay_SlotModeType
.. doxygentypedef:: ib_FlexRay_ErrorModeType
.. doxygentypedef:: ib_FlexRay_StartupStateType
.. doxygentypedef:: ib_FlexRay_WakeupStatusType
.. doxygentypedef:: ib_FlexRay_Header_Flag

.. doxygentypedef:: ib_FlexRay_SymbolAck
