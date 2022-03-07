===================
!!! Data Message API
===================

.. Macros for docs use
.. |IComAdapter| replace:: :cpp:class:`IComAdapter<ib::mw::IComAdapter>`
.. |CreateDataPublisher| replace:: :cpp:func:`CreateDataPublisher<ib::mw::IComAdapter::CreateDataPublisher()>`
.. |CreateDataSubscriber| replace:: :cpp:func:`CreateDataSubscriber<ib::mw::IComAdapter::CreateDataSubscriber()>`
.. |Publish| replace:: :cpp:func:`Publish()<ib::sim::data::IDataPublisher::Publish()>`
.. |SetDefaultReceiveMessageHandler| replace:: :cpp:func:`SetDefaultReceiveMessageHandler()<ib::sim::data::IDataSubscriber::SetDefaultReceiveMessageHandler()>`
.. |RegisterSpecificDataHandler| replace:: :cpp:func:`RegisterSpecificDataHandler()<ib::sim::data::IDataSubscriber::RegisterSpecificDataHandler()>`
.. |IDataPublisher| replace:: :cpp:class:`IDataPublisher<ib::sim::data::IDataPublisher>`
.. |IDataSubscriber| replace:: :cpp:class:`IDataPublisher<ib::sim::data::IDataSubscriber>`
.. contents::
   :local:
   :depth: 3

!!! Using the Data Message API
--------------------------

The Data Message API provides a topic-based publish / subscribe mechanism to exchange plain byte vectors containing
arbitrary user data. Published messages are transmitted immediately to all connected subscribers, that is, without 
any modelled latency. DataSubscribers set a default handler that is called upon incoming data on their topic. Another 
handler notifies the DataSubscriber about new DataPublishers on its topic. For a more advanced routing of messages on 
a common topic, DataSubscriber can register specific reception handlers targeting certain annotated DataPublishers.

!!! Topics
~~~~~~

DataPublishers and DataSubscribers are identified by a topic name and are connected by links. For each link, the 
endpoints must be unique. That is, on one participant, there can only be one publisher / subscriber on a given topic.
However, it is possible to use multiple publishers/subscribers on the same topic distributed among different 
participants.

!!! DataExchangeFormat
~~~~~~~~~~~~~~~~~~

Both DataPublishers and DataSubscribers define a DataExchangeFormat, a meta description of the transmitted data. It can
be used to provide infomation about the de- / serialization of the underlying user data. Just like the topic, the 
DataExchangeFormat has to match between DataPublishers / DataSubscribers for communicaiton to take place. An empty 
string on a DataSubscriber will match any other string of that given field of the DataExchangeFormat of a 
DataPublisher. Currently, the DataExchangeFormat only consists of the field "mediaType".

!!! Labels
~~~~~~

DataPublishers and DataSubscribers can be annotated with string-based key-value pairs (labels). Additional to the 
matching requirements regarding topic and DataExchangeFormat, DataSubscribers will only receive messages by 
DataPublishers if their labels apply the following matching rules:

* A DataSubscriber without labels matches any other DataPublisher on that topic.
* If labels are specified on a DataSubscriber, all of the labels must be found on a DataPublisher.
* An empty value string on a DataSubscriber's label is a wildcard.

!!! Specific handlers
~~~~~~~~~~~~~~~~~

In a scenario where multiple DataPublisher publish on a common topic but DataSubscriber want to treat the incoming 
messages differently, DataSubscriber can route the publications to specific data handlers based on the DataPublisher's 
labels and DataExchangeFormat using the |RegisterSpecificDataHandler| method on a DataSubscriber instance.
The labels and DataExchangeFormat given there will be used to redirect incoming messages by matching DataPublishers to
one or more specific data handlers instead of the default handler. The latter will not be invoked if a specific handler is 
availabe. Note that the wildcard patters for DataSubscribers also apply to labels / DataExchangeFormats given to 
|RegisterSpecificDataHandler|: An empty string in a label value or any field of the DataExchangeFormat is a wildcard.

If the labels / DataExchangeFormats of DataPublishers are unknown beforehand, |RegisterSpecificDataHandler| can 
be used in the handler for new data sources which provides this information. Note that multiple specific data handlers can 
be registered, possibly resulting in multiple calls for one incoming data message.

!!! History
~~~~~~~

DataPublishers additionally specify a history length N (currently 0 or 1). DataSubscribers that are created after a 
publication will still receive the N historic Data Messages from a DataPublisher with history > 0. Note that the
particiant that created the DataPublisher still has to be connected to the distributed simulation for the historic 
messages to be delivered.

!!! Usage
~~~~~

The Publisher and Subscriber interfaces are instantiated from an |IComAdapter| interface by calling 
|CreateDataPublisher| and |CreateDataSubscriber|, respectively. Their name corresponds to the topic and is used in the
configuration and instantiation of the interfaces.

Data can be transmitted using the |Publish| method. The data is received and delivered via a callback, which has to be
specified on creation of the DataSubscriber and can be overwritten using the |SetDefaultReceiveMessageHandler| method.

!!! Usage Examples
~~~~~~~~~~~~~~

The interfaces for the publish/subscribe mechanism can be instantiated from an IComAdapter:

.. code-block:: cpp

    // Participant1 (Publisher)
    // ------------------------

    auto* publisher = comAdapter->CreateDataPublisher("Topic1", DataExchangeFormat{ "json" }, {"KeyA", "ValA"}, 1);
    publisher->Publish(user_data);


    // Participant2 (Subscriber)
    // -------------------------

    auto defaultDataHandler = [](IDataSubscriber* subscriber, const std::vector<uint8_t>& data) 
    {
        // handle data
    });
    auto newDataSourceHandler = [](IDataSubscriber* subscriber, const std::string& topic,
                   const DataExchangeFormat& dataExchangeFormat,
                   const std::map<std::string, std::string>& labels)
    {
        // handle new sources
    });

    auto* subscriber = comAdapter->CreateDataSubscriber("Topic1", DataExchangeFormat{ "" }, {}, 
        defaultDataHandler, newDataSourceHandler);

    auto specificDataHandler = [](IDataSubscriber* subscriber, const std::vector<uint8_t>& data) 
    {
        // handle data for publishers with label key "KeyB"
    });
    subscriber->RegisterSpecificDataHandler(DataExchangeFormat{""}, {{"KeyB", ""}}, specificDataHandler);


!!! API and Data Type Reference
---------------------------

The |IDataPublisher| provides a simple publish interface for standard vector. For convenience an overload for raw data 
pointer  and size exists.

The |IDataSubscriber| provides a callback registration mechanism.

The publisher's and subscriber's read-only :cpp:class:`configuration<ib::cfg::DataPort>` can also be accessed. Note 
that the DataExchangeFormat, history and labels can only be set on the creation calls and not via the configuration 
mechanism.

!!! Data Publisher API
~~~~~~~~~~~~~~~~~~

    .. doxygenclass:: ib::sim::data::IDataPublisher
       :members:

!!! Data Subscriber API
~~~~~~~~~~~~~~~~~~~

    .. doxygenclass:: ib::sim::data::IDataSubscriber
       :members:


!!! Data Structures
~~~~~~~~~~~~~~~

    .. doxygenstruct:: ib::cfg::DataPort
       :members:
