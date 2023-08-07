Data Publish/Subscribe C API
----------------------------

.. contents::
   :local:
   :depth: 3

The publish/subscribe API provides a topic-based publish/subscribe mechanism to exchange arbitrary user data.
It consists of data publishers and data subscribers.

Data Publishers
~~~~~~~~~~~~~~~
.. doxygenfunction:: SilKit_DataPublisher_Create
.. doxygenfunction:: SilKit_DataPublisher_Publish

Data Subscribers
~~~~~~~~~~~~~~~~
.. doxygenfunction:: SilKit_DataSubscriber_Create
.. doxygenfunction:: SilKit_DataSubscriber_SetDataMessageHandler

Handlers
~~~~~~~~
The ``DataSubscriber`` is created with a handler for data reception:

.. doxygentypedef:: SilKit_DataMessageHandler_t

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: SilKit_DataMessageEvent
   :members:
