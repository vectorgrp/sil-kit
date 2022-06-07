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

An Ethernet controller can interact with the corresponding Ethernet bus and send Ethernet frames.

.. doxygenfunction:: ib_Ethernet_Controller_Create
.. doxygenfunction:: ib_Ethernet_Controller_Activate
.. doxygenfunction:: ib_Ethernet_Controller_Deactivate
.. doxygenfunction:: ib_Ethernet_Controller_AddFrameHandler
.. doxygenfunction:: ib_Ethernet_Controller_AddFrameTransmitHandler
.. doxygenfunction:: ib_Ethernet_Controller_AddStateChangeHandler
.. doxygenfunction:: ib_Ethernet_Controller_AddBitrateChangeHandler
.. doxygenfunction:: ib_Ethernet_Controller_SendFrame

Ethernet Frame
~~~~~~~~~~~~~~

The ib_Ethernet_Frame is a raw Ethernet frame consisting of the destination mac, the source mac, the ethertype and a 
payload. The union type within the ib_Ethernet_Frame helps when manual construction of a frame is necessary 
(frameHeader, frameHeaderVlanTagged).

.. note:: For an example of manual frame construction one can refer to the C Ethernet demo.

Data Structures
~~~~~~~~~~~~~~~

.. doxygenstruct:: ib_Ethernet_StateChangeEvent
   :members:
.. doxygenstruct:: ib_Ethernet_BitrateChangeEvent
   :members:
.. doxygenstruct:: ib_Ethernet_FrameEvent
   :members:
.. doxygenstruct:: ib_Ethernet_FrameTransmitEvent
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: ib_Ethernet_TransmitStatus
.. doxygentypedef:: ib_Ethernet_State
.. doxygentypedef:: ib_Ethernet_Bitrate
.. doxygenstruct:: ib_Ethernet_Frame

.. doxygentypedef:: ib_Ethernet_Controller

.. doxygentypedef:: ib_Ethernet_FrameHandler_t
.. doxygentypedef:: ib_Ethernet_FrameTransmitHandler_t
.. doxygentypedef:: ib_Ethernet_StateChangeHandler_t
.. doxygentypedef:: ib_Ethernet_BitrateChangeHandler_t