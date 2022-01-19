===================
Data Message API
===================

.. Macros for docs use
.. |IComAdapter| replace:: :cpp:class:`IComAdapter<ib::mw::IComAdapter>`
.. |CreateDataPublisher| replace:: :cpp:func:`CreateDataPublisher<ib::mw::IComAdapter::CreateDataPublisher()>`
.. |CreateDataSubscriber| replace:: :cpp:func:`CreateDataSubscriber<ib::mw::IComAdapter::CreateDataSubscriber()>`
.. |Publish| replace:: :cpp:func:`Publish()<ib::sim::data::IDataPublisher::Publish()>`
.. |SetReceiveMessageHandler| replace:: :cpp:func:`SetReceiveMessageHandler()<ib::sim::data::IDataSubscriber::SetReceiveMessageHandler()>`
.. |IDataPublisher| replace:: :cpp:class:`IDataPublisher<ib::sim::data::IDataPublisher>`
.. |IDataSubscriber| replace:: :cpp:class:`IDataPublisher<ib::sim::data::IDataSubscriber>`
.. contents::
   :local:
   :depth: 3

Using the Data Message API
--------------------------

A Data Message is a plain byte vector containing arbitrary user data. It can be distributed among the participants of a
simulation using a topic-based publish / subscribe mechanism. Published messages are transmitted immediately to all 
connected subscribers, that is, without any modelled latency. Subscribers specify a handler that is called upon 
incoming data on their topic.

Linkage
~~~~~~~

DataPublishers and DataSubscribers are identified by a topic name and are connected by links. For each link, the 
endpoints must be unique. That is, on one participant, there can only be one publisher / subscriber on a given topic.
However, it is possible to use multiple publishers/subscribers on the same topic distributed among different 
participants.

DataExchangeFormat
~~~~~~~~~~~~~~~~~~

Both DataPublishers and DataSubscribers define a DataExchangeFormat, a meta description of the transmitted data. It can
be used to provide infomation about the de- / serialization of the underlying user data. Just like the topic, the 
DataExchangeFormat has to match between publishers / subscribers for communicaiton to take place. The wildcard 
character "*" will match any other string of that given field of the DataExchangeFormat. Currently, the 
DataExchangeFormat only consists of the field "mimeType". Wildcards encoded in a string (e.g. "json*") are not 
supported. Alongside with the data, the reception handler on the subscriber provides a joined DataExchangeFormat of the
current transmission. This allows to interpret incoming data on a subscriber using wildcards in case of multiple 
publishers on a topic with different DataExchangeFormats.

History
~~~~~~~

DataPublishers additionally specify a history length N (currently 0 or 1). DataSubscribers that are created after a 
publication will still receive the N historic Data Messages from a DataPublisher with history > 0. Note that the
particiant that created the DataPublisher has to be connected to the distributed simulation for the historic messages
to be delivered.

Usage
~~~~~

The Publisher and Subscriber interfaces are instantiated from an |IComAdapter| interface by calling 
|CreateDataPublisher| and |CreateDataSubscriber|, respectively. Their name corresponds to the topic and is used in the
configuration and instantiation of the interfaces.

Data can be transmitted using the |Publish| method. The data is received and delivered via a callback, which has to be
specified on creation of the DataSubscriber and can be overwritten using the |SetReceiveMessageHandler| method.

Usage Examples
~~~~~~~~~~~~~~

The interfaces for the publish/subscribe mechanism can be instantiated from an IComAdapter:

.. code-block:: cpp

    auto comAdapter = ib::CreateComAdapter(std::move(config), participant_name, domainId);
    auto* publishData = comAdapter->CreateDataPublisher("Message1", DataExchangeFormat{ "json" }, 1);
    publishData->Publish(user_data);

    auto* subscribeData = comAdapter->CreateDataSubscriber("Message1", DataExchangeFormat{ "*" },
        [](IDataSubscriber* subscriber, const std::vector<uint8_t>& data, const DataExchangeFormat& joinedDataExchangeFormat ) {
            //handle data
        });

API and Data Type Reference
---------------------------

The |IDataPublisher| provides a simple publish interface for standard vector. For convenience an overload for raw data 
pointer  and size exists.

The |IDataSubscriber| provides a callback registration mechanism.

The publisher's and subscriber's read-only :cpp:class:`configuration<ib::cfg::DataPort>` can also be accessed. Note 
that the DataExchangeFormat and History can only be set on the creation calls and not via the configuration mechanism.

Data Publisher API
~~~~~~~~~~~~~~~~~~

    .. doxygenclass:: ib::sim::data::IDataPublisher
       :members:

Data Subscriber API
~~~~~~~~~~~~~~~~~~~

    .. doxygenclass:: ib::sim::data::IDataSubscriber
       :members:


Data Structures
~~~~~~~~~~~~~~~

    .. doxygenstruct:: ib::cfg::DataPort
       :members:
