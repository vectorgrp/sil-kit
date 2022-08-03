==========================
Data Publish/Subscribe API
==========================

.. Macros for docs use
.. |IParticipant| replace:: :cpp:class:`IParticipant<SilKit::IParticipant>`
.. |CreateDataPublisher| replace:: :cpp:func:`CreateDataPublisher()<SilKit::IParticipant::CreateDataPublisher()>`
.. |CreateDataSubscriber| replace:: :cpp:func:`CreateDataSubscriber()<SilKit::IParticipant::CreateDataSubscriber()>`
.. |Publish| replace:: :cpp:func:`Publish()<SilKit::Services::PubSub::IDataPublisher::Publish()>`
.. |SetDataMessageHandler| replace:: :cpp:func:`SetDataMessageHandler()<SilKit::Services::PubSub::IDataSubscriber::SetDataMessageHandler()>`
.. |AddExplicitDataMessageHandler| replace:: :cpp:func:`AddExplicitDataMessageHandler()<SilKit::Services::PubSub::IDataSubscriber::AddExplicitDataMessageHandler()>`
.. |IDataPublisher| replace:: :cpp:class:`IDataPublisher<SilKit::Services::PubSub::IDataPublisher>`
.. |IDataSubscriber| replace:: :cpp:class:`IDataPublisher<SilKit::Services::PubSub::IDataSubscriber>`
.. contents::
   :local:
   :depth: 3

Using the Data Publish/Subscribe API
------------------------------------

The Data Publish/Subscribe API provides a topic-based publish/subscribe mechanism to exchange plain byte vectors containing
arbitrary user data. Published messages are transmitted immediately to all matching subscribers, that is, without 
any modelled latency.

Data can be transmitted using the |Publish| method of a DataPublisher, either providing a standard vector of data or a
data pointer and size. DataSubscribers can provide a handler that is called upon incoming data on their topic.

Topics
~~~~~~

DataPublishers and DataSubscribers provide a topic name, communications only takes place among controllers with the 
same topic. The topic has no wildcard functionality.

Media Type
~~~~~~~~~~

Both DataPublishers and DataSubscribers define a media type in accordance to 
`RFC2046 <https://datatracker.ietf.org/doc/html/rfc2046>`_, a meta description of the transmitted data. It can be used
to provide infomation about the de-/serialization of the underlying user data. Just like the topic, the media type has
to match between DataPublishers/DataSubscribers for communicaiton to take place. An empty string on a DataSubscriber
is a wildcard and will match any other media type of a DataPublisher.

Labels
~~~~~~

DataPublishers and DataSubscribers can be annotated with string-based key-value pairs (labels). In addition to the
matching requirements regarding topic and media type, DataSubscribers will only receive messages by DataPublishers if
their labels apply the following matching rules:

* A DataSubscriber without labels matches any DataPublisher.
* A mandatory label matches, if a label of the same key and value ist found on the corresponding DataPublisher.
* A preferred label matches, if the label key does not exist on the DataPublisher or both its key and value are equal.

The following table shows how DataPublishers and DataSubscribers with matching topics and matching media type would match corresponding to their labels.

.. list-table:: Label combinations
   :header-rows: 1

   * - 
     - Sub {"KeyA", "Val1", Preferred}
     - Sub {"KeyA", "Val1", Mandatory}
   * - Pub {}
     - Match
     - No Match
   * - Pub {"KeyA", "Val1"}
     - Match
     - Match
   * - Pub {"KeyA", "Val2"}
     - No Match
     - No Match
   * - Pub {"KeyB", "Val1"}
     - Match
     - No Match

The following code snippet shows how the labels of a DataSubscriber can be set.

.. code-block:: cpp

    SilKit::Services::PubSub::DataSubscriberSpec subDataSpec{"Topic1", "application/json"};
    subDataSpec.AddLabel("KeyA", "ValA", SilKit::Services::Label::Kind::Preferred);
    auto* subscriber = participant->CreateDataSubscriber("SubCtrl1", subDataSpec, defaultDataHandler);


History
~~~~~~~

DataPublishers additionally specify a history length N (restricted to 0 or 1). DataSubscribers that are created after a 
publication will still receive the N historic Data Messages from a DataPublisher with history > 0. Note that the
particiant that created the DataPublisher still has to be connected to the distributed simulation for the historic 
messages to be delivered.

Configuration
~~~~~~~~~~~~~

The controller name passed in |CreateDataPublisher| and |CreateDataSubscriber| is used to identify the controller in 
a YAML/JSON configuration. Currently, only the topic can be configured. If a topic is set in the configuration, it will
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

The simplified overloads only need a controller name as a single argument, which will be used as topic. Media type, 
labels and handlers are left emtpy in this variant. Note that in this case, the DataSubscriber still has to provide a 
handler for incoming messages via |SetDataMessageHandler|.

Usage Examples
~~~~~~~~~~~~~~

The interfaces for the publish/subscribe mechanism can be instantiated from an IParticipant:

.. code-block:: cpp

    // Participant1 (Publisher)
    // ------------------------
    SilKit::Services::PubSub::DataPublisherSpec pubDataSpec{"Topic1", "application/json"};
    pubDataSpec.AddLabel("KeyA", "ValA");
    auto* publisher = participant->CreateDataPublisher("PubCtrl1", pubDataSpec, 1);
    publisher->Publish(user_data);

    // Participant2 (Subscriber)
    // -------------------------

    auto defaultDataHandler = [](IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent) 
    {
        // publication timestamp in dataMessageEvent.timestamp
        // raw data in dataMessageEvent.data
    });
    SilKit::Services::PubSub::DataSubscriberSpec subDataSpec{"Topic1", "application/json"};
    subDataSpec.AddLabel("KeyA", "ValA", SilKit::Services::Label::Kind::Preferred);
    auto* subscriber = participant->CreateDataSubscriber("SubCtrl1", subDataSpec, defaultDataHandler);


API and Data Type Reference
---------------------------

The |IDataPublisher| provides a simple publish interface for standard vector. An overload for raw data 
pointer and size exists for ease of use.

The |IDataSubscriber| provides a callback registration mechanism for the default callback via 
|SetDataMessageHandler| and for targeting explicit DataPublishers via |AddExplicitDataMessageHandler|.

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

.. doxygenstruct:: SilKit::Services::Label
   :members:

.. doxygenclass:: SilKit::Services::PubSub::DataPublisherSpec
   :members:

.. doxygenclass:: SilKit::Services::PubSub::DataSubscriberSpec
   :members:
       