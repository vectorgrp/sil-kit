Can C API
----------

.. contents::
   :local:
   :depth: 3

Can Controller
~~~~~~~~~~~~~~~

**A Can controller is created and configured with the following functions:**

.. doxygenfunction:: ib_Can_Controller_Create
.. doxygenfunction:: ib_Can_Controller_SetBaudRate

**It's status can be controlled with the functions:**

.. doxygenfunction:: ib_Can_Controller_Start
.. doxygenfunction:: ib_Can_Controller_Stop
.. doxygenfunction:: ib_Can_Controller_Reset
.. doxygenfunction:: ib_Can_Controller_Sleep

**The controller can send frames with:**

.. doxygenfunction:: ib_Can_Controller_SendFrame

**The following set of functions can be used to add and remove event handlers on the controller:**

.. doxygenfunction:: ib_Can_Controller_AddFrameTransmitHandler
.. doxygenfunction:: ib_Can_Controller_AddFrameHandler
.. doxygenfunction:: ib_Can_Controller_AddStateChangeHandler
.. doxygenfunction:: ib_Can_Controller_AddErrorStateChangeHandler
.. doxygenfunction:: ib_Can_Controller_RemoveFrameTransmitHandler
.. doxygenfunction:: ib_Can_Controller_RemoveFrameHandler
.. doxygenfunction:: ib_Can_Controller_RemoveStateChangeHandler
.. doxygenfunction:: ib_Can_Controller_RemoveErrorStateChangeHandler

Data Structures
~~~~~~~~~~~~~~~

.. doxygenstruct:: ib_Can_Frame
   :members:

.. doxygenstruct:: ib_Can_FrameEvent
   :members:

.. doxygenstruct:: ib_Can_FrameTransmitEvent
   :members:

.. doxygenstruct:: ib_Can_StateChangeEvent
   :members:

.. doxygenstruct:: ib_Can_ErrorStateChangeEvent
   :members:

