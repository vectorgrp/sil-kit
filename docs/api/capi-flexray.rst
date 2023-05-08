FlexRay C API
-------------

.. contents::
   :local:
   :depth: 3

The FlexRay API consists of the following parts:

* The FlexRay controller
* The FlexRay controller and TX buffer configuration
* A set of FlexRay specific messages, each with its own function to register callbacks: 
  ``Message``, ``MessageAck``, ``Wakeup``, ``ControllerStatus``, ``PocStatus``, ``Symbol``, ``SymbolAck``, ``CycleStart``
  
 
FlexRay Controller
~~~~~~~~~~~~~~~~~~
A FlexRay controller interacts with the configured FlexRay bus and sends FlexRay frames and other events on its own behalf.
Note that it is not possible to explicitly send frames or other events, nor exist any API functions to construct these events.
This is because the FlexRay bus works completely time triggered, sending preconfigured frames.
To configure these frames, the API provides functions to manipulate Tx buffers.

**The following functions can be used to create a FlexRay controller and manipulate its configuration:**

.. doxygenfunction:: SilKit_FlexrayController_Create
.. doxygenfunction:: SilKit_FlexrayController_Configure
.. doxygenfunction:: SilKit_FlexrayController_ReconfigureTxBuffer
.. doxygenfunction:: SilKit_FlexrayController_UpdateTxBuffer

**The following function can be used to manipulate the controller's state by triggering Controller Host Interface (CHI) commands:**

.. doxygenfunction:: SilKit_FlexrayController_ExecuteCmd

**The following set of functions can be used to add and remove event handlers on the controller:**

.. doxygenfunction:: SilKit_FlexrayController_AddFrameHandler
.. doxygenfunction:: SilKit_FlexrayController_AddFrameTransmitHandler
.. doxygenfunction:: SilKit_FlexrayController_AddWakeupHandler
.. doxygenfunction:: SilKit_FlexrayController_AddPocStatusHandler
.. doxygenfunction:: SilKit_FlexrayController_AddSymbolHandler
.. doxygenfunction:: SilKit_FlexrayController_AddSymbolTransmitHandler
.. doxygenfunction:: SilKit_FlexrayController_AddCycleStartHandler
.. doxygenfunction:: SilKit_FlexrayController_RemoveFrameHandler
.. doxygenfunction:: SilKit_FlexrayController_RemoveFrameTransmitHandler
.. doxygenfunction:: SilKit_FlexrayController_RemoveWakeupHandler
.. doxygenfunction:: SilKit_FlexrayController_RemovePocStatusHandler
.. doxygenfunction:: SilKit_FlexrayController_RemoveSymbolHandler
.. doxygenfunction:: SilKit_FlexrayController_RemoveSymbolTransmitHandler
.. doxygenfunction:: SilKit_FlexrayController_RemoveCycleStartHandler

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: SilKit_FlexrayClusterParameters
   :members:
.. doxygenstruct:: SilKit_FlexrayNodeParameters
   :members:
.. doxygenstruct:: SilKit_FlexrayTxBufferConfig
   :members:
.. doxygenstruct:: SilKit_FlexrayControllerConfig
   :members:
.. doxygenstruct:: SilKit_FlexrayTxBufferUpdate
   :members:
.. doxygenstruct:: SilKit_FlexrayHeader
   :members:
.. doxygenstruct:: SilKit_FlexrayFrame
   :members:

.. doxygenstruct:: SilKit_FlexrayFrameEvent
   :members:
.. doxygenstruct:: SilKit_FlexrayFrameTransmitEvent
   :members:
.. doxygenstruct:: SilKit_FlexraySymbolEvent
   :members:
.. doxygenstruct:: SilKit_FlexrayCycleStartEvent
   :members:
.. doxygenstruct:: SilKit_FlexrayPocStatusEvent
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: SilKit_FlexrayMacroTick
.. doxygentypedef:: SilKit_FlexrayMicroTick
.. doxygentypedef:: SilKit_FlexrayClockPeriod
.. doxygentypedef:: SilKit_FlexrayChannel
.. doxygentypedef:: SilKit_FlexraySymbolPattern
.. doxygentypedef:: SilKit_FlexrayChiCommand
.. doxygentypedef:: SilKit_FlexrayTransmissionMode
.. doxygentypedef:: SilKit_FlexrayPocState
.. doxygentypedef:: SilKit_FlexraySlotModeType
.. doxygentypedef:: SilKit_FlexrayErrorModeType
.. doxygentypedef:: SilKit_FlexrayStartupStateType
.. doxygentypedef:: SilKit_FlexrayWakeupStatusType
.. doxygentypedef:: SilKit_FlexrayHeader_Flag

.. doxygentypedef:: SilKit_FlexraySymbolTransmitEvent
