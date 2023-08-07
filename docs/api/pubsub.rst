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

.. |PubSubSpec| replace:: :cpp:class:`PubSubSpec<SilKit::Services::PubSub::PubSubSpec>`
.. |AddLabel| replace:: :cpp:func:`AddLabel()<SilKit::Services::PubSub::PubSubSpec::AddLabel>`
.. |MatchingLabel| replace:: :cpp:class:`MatchingLabel<SilKit::Services::MatchingLabel>`

.. |IDataPublisher| replace:: :cpp:class:`IDataPublisher<SilKit::Services::PubSub::IDataPublisher>`
.. |IDataSubscriber| replace:: :cpp:class:`IDataPublisher<SilKit::Services::PubSub::IDataSubscriber>`

.. |MediaTypeData| replace:: :cpp:func:`MediaTypeData()<SilKit::Util::SerDes::MediaTypeData()>`
.. contents::
   :local:
   :depth: 3

Using the Data Publish/Subscribe API
------------------------------------

The Data Publish/Subscribe API provides a topic-based publish/subscribe mechanism to exchange plain byte vectors 
containing arbitrary user data. The ``DataPublisher`` and ``DataSubscriber`` interfaces are instantiated from an |IParticipant| 
interface by calling |CreateDataPublisher| and |CreateDataSubscriber|, respectively.

Data can be transmitted using the |Publish| method of a ``DataPublisher``, either providing a standard vector of data or a
data pointer and size. Published messages are transmitted immediately to all matching subscribers, that is, without 
any modelled latency. Data subscribers provide a handler that is called upon incoming data on their topic.

Data
~~~~

Data is represented as a byte vector, so the serialization schema can be chosen by the user. Nonetheless, it is highly recommended 
to use SIL Kit's :doc:`Data Serialization/Deserialization API</api/serdes>` to ensure compatibility among all SIL Kit participants.

Topics
~~~~~~

Data publishers and data subscribers provide a topic name which is part of their |PubSubSpec|.
Communications only takes place among controllers with the same topic. The topic has no wildcard functionality.

Media Type
~~~~~~~~~~

Both data publishers and data subscribers define a media type as part of their |PubSubSpec|. It is a meta description
of the transmitted data in accordance to `RFC2046 <https://datatracker.ietf.org/doc/html/rfc2046>`_ and should be used
to provide information about the de-/serialization of the underlying user data. Just like the topic, the media type has
to match between data publishers/subscribers for communication to take place. An empty string on a data subscriber
is a wildcard and will match any other media type of data publishers. Data publishers should provide information
about the data they are going to publish and have no wildcard functionality for the media type.

When data is serialized using SIL Kit's :doc:`Data Serialization/Deserialization API</api/serdes>`, media type constant |MediaTypeData| 
must be used.

Labels
~~~~~~

Both data publishers and data subscribers can be annotated with string-based key-value pairs (labels) which can be either
mandatory or optional. In addition to the matching requirements given by topic and media type, data publishers and
Data subscribers will only communicate if their labels conform to the following matching rules:

* A mandatory label matches, if a label of the same key and value is found on the corresponding counterpart.
* An optional label matches, if the label key does not exist on the counterpart or both its key and value are equal.

The following table shows how data publishers and data subscribers with matching topics and matching media type would 
match corresponding to their labels. Note that the label matching is symmetric, so publishers and subscribers
are interchangeable here.

.. list-table:: Label combinations
   :header-rows: 1

   * - 
     - Subscriber {"KeyA", "Val1", Optional}
     - Subscriber{"KeyA", "Val1", Mandatory}
   * - Publisher {}
     - Match
     - No Match
   * - Publisher {"KeyA", "Val1", Optional}
     - Match
     - Match
   * - Publisher {"KeyA", "Val2", Optional}
     - No Match
     - No Match
   * - Publisher {"KeyB", "Val1", Optional}
     - Match
     - No Match
   * - Publisher {"KeyB", "Val1", Mandatory}
     - No Match
     - No Match

The labels are stored in the |PubSubSpec|. A |MatchingLabel| can be added via |AddLabel|,
see the following code snippet:

.. code-block:: cpp

    SilKit::Services::PubSub::PubSubSpec subDataSpec{"Topic1", "application/json"};
    subDataSpec.AddLabel("KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Optional);
    auto* subscriber = participant->CreateDataSubscriber("SubCtrl1", subDataSpec, defaultDataHandler);


History
~~~~~~~

Data publishers additionally specify a history length N (restricted to 0 or 1). Data subscribers that are created after a 
publication will still receive the N historic data messages from a data publisher with history > 0. Note that the
participant that created the data publisher still has to be connected to the distributed simulation for the historic 
messages to be delivered.

Configuration
~~~~~~~~~~~~~

The controller name passed in |CreateDataPublisher| and |CreateDataSubscriber| is used to identify the controller in 
a YAML configuration. Currently, only the topic can be configured. If a topic is set in the configuration, it will
be preferred over a programmatically set topic.

.. code-block:: yaml

    ParticipantName: Participant1
    DataPublishers:
    - Name: DataPublisherController1
      Topic: TopicA
    DataSubscribers:
    - Name: DataSubscriberController1
      Topic: TopicB

Usage Examples
~~~~~~~~~~~~~~


The interfaces for the publish/subscribe mechanism can be instantiated from an |IParticipant|:

.. code-block:: cpp

    // Participant1 (Publisher)
    // ------------------------
    SilKit::Services::PubSub::PubSubSpec pubDataSpec{"Topic1", "application/json"};
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
    SilKit::Services::PubSub::PubSubSpec subDataSpec{"Topic1", "application/json"};
    subDataSpec.AddLabel("KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Optional);
    auto* subscriber = participant->CreateDataSubscriber("SubCtrl1", subDataSpec, defaultDataHandler);


API and Data Type Reference
---------------------------

The |IDataPublisher| provides a simple publish interface for standard vector. An overload for raw data 
pointer and size exists for ease of use.

The |IDataSubscriber| provides a callback registration mechanism for the default callback via 
|SetDataMessageHandler| and for targeting explicit data publishers via |AddExplicitDataMessageHandler|.

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

.. doxygenclass:: SilKit::Services::PubSub::PubSubSpec
   :members:
