DataMessage C API
-----------------

.. contents::
   :local:
   :depth: 3

The Data Message API provides a topic-based publish / subscribe mechanism to exchange arbitrary user data. 
It consists of DataPublishers and DataSubscribers.

DataPublishers
~~~~~~~~~~~~~~
.. doxygenfunction:: ib_Data_Publisher_Create
.. doxygenfunction:: ib_Data_Publisher_Publish

DataSubscribers
~~~~~~~~~~~~~~~
.. doxygenfunction:: ib_Data_Subscriber_Create
.. doxygenfunction:: ib_Data_Subscriber_SetDefaultDataMessageHandler
.. doxygenfunction:: ib_Data_Subscriber_AddExplicitDataMessageHandler

Handlers
~~~~~~~~
The DataSubscriber is created with a handler for data reception and a handler
for notification about new sources:

.. doxygentypedef:: ib_Data_DataMessageHandler_t
.. doxygentypedef:: ib_Data_NewDataPublisherHandler_t

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: ib_Data_DataMessageEvent
   :members:
.. doxygenstruct:: ib_Data_NewDataPublisherEvent
   :members:


