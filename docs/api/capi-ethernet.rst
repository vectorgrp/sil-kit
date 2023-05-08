Ethernet C API
--------------

.. contents::
   :local:
   :depth: 3

The Ethernet API consists of two main parts:

#. The Ethernet controller
#. The Ethernet frame

Ethernet Controller
~~~~~~~~~~~~~~~~~~~

**An Ethernet controller is created with the following function:**

.. doxygenfunction:: SilKit_EthernetController_Create

**It's status can be controlled with the functions:**

.. doxygenfunction:: SilKit_EthernetController_Activate
.. doxygenfunction:: SilKit_EthernetController_Deactivate

**The Ethernet controller can send Ethernet frames with:**

.. doxygenfunction:: SilKit_EthernetController_SendFrame

**The following set of functions can be used to add and remove event handlers on the controller:**

.. doxygenfunction:: SilKit_EthernetController_AddFrameHandler
.. doxygenfunction:: SilKit_EthernetController_AddFrameTransmitHandler
.. doxygenfunction:: SilKit_EthernetController_AddStateChangeHandler
.. doxygenfunction:: SilKit_EthernetController_AddBitrateChangeHandler
.. doxygenfunction:: SilKit_EthernetController_RemoveFrameHandler
.. doxygenfunction:: SilKit_EthernetController_RemoveFrameTransmitHandler
.. doxygenfunction:: SilKit_EthernetController_RemoveStateChangeHandler
.. doxygenfunction:: SilKit_EthernetController_RemoveBitrateChangeHandler

Ethernet Frame
~~~~~~~~~~~~~~

The ``SilKit_EthernetFrame`` is a raw Ethernet frame consisting of the destination MAC, the source MAC, the `EtherType` and a 
payload.

.. note:: For an example of manual frame construction one can refer to the C Ethernet demo.

Data Structures
~~~~~~~~~~~~~~~

.. doxygenstruct:: SilKit_EthernetStateChangeEvent
   :members:
.. doxygenstruct:: SilKit_EthernetBitrateChangeEvent
   :members:
.. doxygenstruct:: SilKit_EthernetFrameEvent
   :members:
.. doxygenstruct:: SilKit_EthernetFrameTransmitEvent
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: SilKit_EthernetTransmitStatus
.. doxygentypedef:: SilKit_EthernetState
.. doxygentypedef:: SilKit_EthernetBitrate
.. doxygenstruct:: SilKit_EthernetFrame

.. doxygentypedef:: SilKit_EthernetController

.. doxygentypedef:: SilKit_EthernetFrameHandler_t
.. doxygentypedef:: SilKit_EthernetFrameTransmitHandler_t
.. doxygentypedef:: SilKit_EthernetStateChangeHandler_t
.. doxygentypedef:: SilKit_EthernetBitrateChangeHandler_t