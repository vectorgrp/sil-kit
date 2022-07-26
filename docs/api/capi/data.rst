DataMessage C API
-----------------

.. contents::
   :local:
   :depth: 3

The Data Message API provides a topic-based publish / subscribe mechanism to exchange arbitrary user data. 
It consists of DataPublishers and DataSubscribers.

DataPublishers
~~~~~~~~~~~~~~
.. doxygenfunction:: SilKit_DataPublisher_Create
.. doxygenfunction:: SilKit_DataPublisher_Publish

DataSubscribers
~~~~~~~~~~~~~~~
.. doxygenfunction:: SilKit_DataSubscriber_Create
.. doxygenfunction:: SilKit_DataSubscriber_SetDataMessageHandler
.. doxygenfunction:: SilKit_DataSubscriber_AddExplicitDataMessageHandler

Handlers
~~~~~~~~
The DataSubscriber is created with a handler for data reception and a handler
for notification about new sources:

.. doxygentypedef:: SilKit_DataMessageHandler_t
.. doxygentypedef:: SilKit_NewDataPublisherHandler_t

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: SilKit_DataMessageEvent
   :members:
.. doxygenstruct:: SilKit_NewDataPublisherEvent
   :members:


