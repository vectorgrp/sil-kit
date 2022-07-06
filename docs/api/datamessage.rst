================
Data Message API
================

.. Macros for docs use
.. |IParticipant| replace:: :cpp:class:`IParticipant<SilKit::IParticipant>`
.. |CreateDataPublisher| replace:: :cpp:func:`CreateDataPublisher<SilKit::IParticipant::CreateDataPublisher()>`
.. |CreateDataSubscriber| replace:: :cpp:func:`CreateDataSubscriber<SilKit::IParticipant::CreateDataSubscriber()>`
.. |Publish| replace:: :cpp:func:`Publish()<SilKit::Services::PubSub::IDataPublisher::Publish()>`
.. |SetDefaultDataMessageHandler| replace:: :cpp:func:`SetDefaultDataMessageHandler()<SilKit::Services::PubSub::IDataSubscriber::SetDefaultDataMessageHandler()>`
.. |AddExplicitDataMessageHandler| replace:: :cpp:func:`AddExplicitDataMessageHandler()<SilKit::Services::PubSub::IDataSubscriber::AddExplicitDataMessageHandler()>`
.. |IDataPublisher| replace:: :cpp:class:`IDataPublisher<SilKit::Services::PubSub::IDataPublisher>`
.. |IDataSubscriber| replace:: :cpp:class:`IDataPublisher<SilKit::Services::PubSub::IDataSubscriber>`
.. contents::
   :local:
   :depth: 3

Using the Data Message API
--------------------------

The Data Message API provides a topic-based publish / subscribe mechanism to exchange plain byte vectors containing
arbitrary user data. Published messages are transmitted immediately to all matching subscribers, that is, without 
any modelled latency.

Data can be transmitted using the |Publish| method of a DataPublisher, either providing a standard vector of data or a
data pointer and size. DataSubscribers can provide a handler that is called upon incoming data on their topic.

Topics
~~~~~~

DataPublishers and DataSubscribers provide a topic name, communications only takes place among controllers with the 
same topic. The topic has no wildcard functionality.

Media type
~~~~~~~~~~

Both DataPublishers and DataSubscribers define a media type in accordance to 
`RFC2046 <https://datatracker.ietf.org/doc/html/rfc2046>`_, a meta description of the transmitted data. It can be used
to provide infomation about the de- / serialization of the underlying user data. Just like the topic, the media type has
to match between DataPublishers / DataSubscribers for communicaiton to take place. An empty string on a DataSubscriber
is a wildcard and will match any other media type of a DataPublisher.

Labels
~~~~~~

DataPublishers and DataSubscribers can be annotated with string-based key-value pairs (labels). Additional to the
matching requirements regarding topic and media type, DataSubscribers will only receive messages by DataPublishers if
their labels apply the following matching rules:

* A DataSubscriber without labels matches any other DataPublisher.
* If labels are specified on a DataSubscriber, all of the labels must be found on a DataPublisher.
* An empty label value on a DataSubscriber is a wildcard for the DataPublisher label value of their common label key.

Explicit handlers
~~~~~~~~~~~~~~~~~

In a scenario where multiple DataPublishers publish on a common topic but a DataSubscriber wants to treat the incoming
messages differently, the DataSubscriber can route the publications to explicit data handlers based on the
DataPublishers labels and media type using the |AddExplicitDataMessageHandler| method on a DataSubscriber instance. The
labels and media type given there will be used to redirect incoming messages by matching DataPublishers to one or more
explicit data handlers instead of the default handler. The latter will not be invoked if a specific handler is
availabe. Note that the wildcard patters for DataSubscribers also apply to labels / media type given to
|AddExplicitDataMessageHandler|: An empty string in a label value or the media type is a wildcard.

Source discovery
~~~~~~~~~~~~~~~~

If the labels / media type of DataPublishers are unknown beforehand, this information can be obtained by a  
handler on the DataSubscriber that notifies about new DataPublishers on its topic. Note that the source discovery
handler will only be invoked once per uniqe set of media type and labels, even if this set is used by multiple data 
publishers.

.. code-block:: cpp

    auto newDataPublisherHandler = [](IDataSubscriber* subscriber, const NewDataPublisherEvent& dataSource)
    {
        // handle new sources, e.g. by adding an explicit handler for this set of media type and labels:
        subscriber->AddExplicitDataMessageHandler(dataHandler, mediaType, labels);
    });
    auto* subscriber = participant->CreateDataSubscriber("SubCtrl1", "Topic1", "", {}, defaultDataHandler, 
                                                         newDataPublisherHandler);

History
~~~~~~~

DataPublishers additionally specify a history length N (restricted to 0 or 1). DataSubscribers that are created after a 
publication will still receive the N historic Data Messages from a DataPublisher with history > 0. Note that the
particiant that created the DataPublisher still has to be connected to the distributed simulation for the historic 
messages to be delivered.

Configuration
~~~~~~~~~~~~~

The controller name passed in |CreateDataPublisher| and |CreateDataSubscriber| is used to identify the controller in 
a YAML/json configuration. Currently, only the topic can be configured. If a topic is set in the configuration, it will
be preferred over a programmatically set topic.

.. code-block:: yaml

    ParticipantName: Participant1
    DataPublishers:
    - Name: DataPublisherController1
      Topic: TopicA
    DataSubscribers:
    - Name: DataSubscriberController1
      Topic: TopicB

Usage
~~~~~

The Publisher and Subscriber interfaces are instantiated from an |IParticipant| interface by calling 
|CreateDataPublisher| and |CreateDataSubscriber|, respectively. 

The simplified overloads only need a controller name as a single argument which will be used as topic. Media type, 
labels and handlers are left emtpy in this variant. Note that in this case, the DataSubscriber still has to provide a 
handler for incoming messages via |SetDefaultDataMessageHandler|.

Usage Examples
~~~~~~~~~~~~~~

The interfaces for the publish/subscribe mechanism can be instantiated from an IParticipant:

.. code-block:: cpp

    // Participant1 (Publisher)
    // ------------------------

    auto* publisher = participant->CreateDataPublisher("PubCtrl1", "Topic1", "application/json", {"KeyA", "ValA"}, 1);
    publisher->Publish(user_data);

    // Participant2 (Subscriber)
    // -------------------------

    auto defaultDataHandler = [](IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent) 
    {
        // publication timestamp in dataMessageEvent.timestamp
        // raw data in dataMessageEvent.data
    });
    auto newDataPublisherHandler = [](IDataSubscriber* subscriber, const NewDataPublisherEvent& dataSource)
    {
        // handle new sources
    });

    auto* subscriber = participant->CreateDataSubscriber("SubCtrl1", "Topic1", "", {}, defaultDataHandler, newDataPublisherHandler);

    auto explicitDataHandler = [](IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent) 
    {
        // handle data for publishers with label key "KeyB"
    });
    subscriber->AddExplicitDataMessageHandler("", {{"KeyB", ""}}, explicitDataHandler);


API and Data Type Reference
---------------------------

The |IDataPublisher| provides a simple publish interface for standard vector. For convenience an overload for raw data 
pointer and size exists.

The |IDataSubscriber| provides a callback registration mechanism for the default callback via 
|SetDefaultDataMessageHandler| and for targeting explicit DataPublishers via |AddExplicitDataMessageHandler|.

Data Publisher API
~~~~~~~~~~~~~~~~~~

    .. doxygenclass:: SilKit::Services::PubSub::IDataPublisher
       :members:

Data Subscriber API
~~~~~~~~~~~~~~~~~~~

    .. doxygenclass:: SilKit::Services::PubSub::IDataSubscriber
       :members:

Data Structures
~~~~~~~~~~~~~~~

    .. doxygenstruct:: SilKit::Services::PubSub::DataMessageEvent
       :members:

    .. doxygenstruct:: SilKit::Services::PubSub::NewDataPublisherEvent
       :members:
       