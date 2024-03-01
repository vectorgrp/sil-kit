CAN C API
----------

.. contents::
   :local:
   :depth: 3

CAN Controller
~~~~~~~~~~~~~~~

**A CAN controller is created and configured with the following functions:**

.. doxygenfunction:: SilKit_CanController_Create
.. doxygenfunction:: SilKit_CanController_SetBaudRate

**Its status can be controlled with the functions:**

.. doxygenfunction:: SilKit_CanController_Start
.. doxygenfunction:: SilKit_CanController_Stop
.. doxygenfunction:: SilKit_CanController_Reset
.. doxygenfunction:: SilKit_CanController_Sleep

**The controller can send frames with:**

.. doxygenfunction:: SilKit_CanController_SendFrame

**The following set of functions can be used to add and remove event handlers on the controller:**

.. doxygenfunction:: SilKit_CanController_AddFrameTransmitHandler
.. doxygenfunction:: SilKit_CanController_AddFrameHandler
.. doxygenfunction:: SilKit_CanController_AddStateChangeHandler
.. doxygenfunction:: SilKit_CanController_AddErrorStateChangeHandler
.. doxygenfunction:: SilKit_CanController_RemoveFrameTransmitHandler
.. doxygenfunction:: SilKit_CanController_RemoveFrameHandler
.. doxygenfunction:: SilKit_CanController_RemoveStateChangeHandler
.. doxygenfunction:: SilKit_CanController_RemoveErrorStateChangeHandler

Data Structures
~~~~~~~~~~~~~~~

.. doxygenstruct:: SilKit_CanFrame
   :members:

.. doxygenstruct:: SilKit_CanFrameEvent
   :members:

.. doxygenstruct:: SilKit_CanFrameTransmitEvent
   :members:

.. doxygenstruct:: SilKit_CanStateChangeEvent
   :members:

.. doxygenstruct:: SilKit_CanErrorStateChangeEvent
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygentypedef:: SilKit_CanFrameFlag
.. doxygendefine:: SilKit_CanFrameFlag_ide
.. doxygendefine:: SilKit_CanFrameFlag_rtr
.. doxygendefine:: SilKit_CanFrameFlag_fdf
.. doxygendefine:: SilKit_CanFrameFlag_brs
.. doxygendefine:: SilKit_CanFrameFlag_esi
.. doxygendefine:: SilKit_CanFrameFlag_xlf
.. doxygendefine:: SilKit_CanFrameFlag_sec
