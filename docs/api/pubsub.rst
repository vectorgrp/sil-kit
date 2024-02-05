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
.. |IDataSubscriber| replace:: :cpp:class:`IDataSubscriber<SilKit::Services::PubSub::IDataSubscriber>`

.. |MediaTypeData| replace:: :cpp:func:`MediaTypeData()<SilKit::Util::SerDes::MediaTypeData()>`

.. _chap:pubsub-service-api:

==========================
Data Publish/Subscribe API
==========================

.. contents::
   :local:
   :depth: 3


Using the Data Publish/Subscribe API
====================================

The Data Publish/Subscribe API provides a topic-based publish/subscribe mechanism to exchange serialized application data.

Publishing Data on a Publisher
------------------------------

The |IDataPublisher| is instantiated from an |IParticipant| instance by calling the |CreateDataPublisher| method.

.. code-block:: cpp

    SilKit::Services::PubSub::PubSubSpec pubSpec{"OilTemperature", SilKit::Util::SerDes::MediaTypeData()};
    auto* publisher = participant->CreateDataPublisher("PubOilTemperature", pubSpec);

Data can be transmitted using the |Publish| method of an |IDataPublisher| instance.
Published messages are transmitted immediately to all matching subscribers, that is, without any modelled latency.

.. code-block:: cpp

    SilKit::Util::SerDes::Serializer serializer;
    serializer.Serialize(uint32_t{110});

    publisher->Publish(serializer.ReleaseBuffer());

Receiving Data on a Subscriber
------------------------------

The |IDataSubscriber| is instantiated from an |IParticipant| instance by calling the |CreateDataSubscriber| method.
Upon incoming data from a publisher, the handler provided to the |CreateDataSubscriber| method is called.

.. code-block:: cpp

    auto subscriberDataHandler = [](IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent) {
        SilKit::Util::SerDes::Deserializer deserializer{SilKit::Util::ToStdVector(dataMessageEvent.data)};
        std::cout << "oil temperature is " << deserializer.Deserialize<uint32_t>() << "°C" << std::endl;
    });

    SilKit::Services::PubSub::PubSubSpec subSpec{"OilTemperature", SilKit::Util::SerDes::MediaTypeData()};
    auto* subscriber = participant->CreateDataSubscriber("SubOilTemperature", subSpec, subscriberDataHandler);

Data is represented as a byte vector, so the serialization schema can be chosen by the user.
Nonetheless, it is highly recommended to use SIL Kit's :doc:`Data Serialization/Deserialization API</api/serdes>` to ensure compatibility among all SIL Kit participants.


API and Data Type Reference
===========================

Data Publisher API
------------------

.. doxygenclass:: SilKit::Services::PubSub::IDataPublisher
   :members:

Data Subscriber API
-------------------

.. doxygenclass:: SilKit::Services::PubSub::IDataSubscriber
   :members:

Data Structures
---------------

.. doxygenstruct:: SilKit::Services::PubSub::DataMessageEvent
   :members:

.. doxygenclass:: SilKit::Services::PubSub::PubSubSpec
   :members:


Usage Examples
==============

Example: Oil Temperature
------------------------

Publisher - Oil Temperature Sensor
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

    // creation of the data publisher

    SilKit::Services::PubSub::PubSubSpec pubSpec{"OilTemperature", SilKit::Util::SerDes::MediaTypeData()};

    auto* publisher = participant->CreateDataPublisher("OilTemperatureSensor", pubSpec, 1);

    // serialization of data and publishing

    float oilTemperature{model.GetOilTemperatureInCelsius()};

    SilKit::Util::SerDes::Serializer serializer;
    serializer.Serialize(oilTemperature);

    publisher->Publish(serializer.ReleaseBuffer());

Subscriber - Engine Dashboard
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

    // creation of the data subscriber

    SilKit::Services::PubSub::PubSubSpec subSpec{"OilTemperature", SilKit::Util::SerDes::MediaTypeData()};

    auto subscriberDataHandler = [&gauge](IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent)
    {
        // deserialization and processing

        SilKit::Util::SerDes::Deserializer deserializer;
        auto oilTemperature = deserializer.Deserialize<float>();

        gauge.SetValueInCelsius(oilTemperature);
    });

    auto* subscriber = participant->CreateDataSubscriber("OilTemperatureGauge", subSpec, subscriberDataHandler);

Example: Wheel and Control with Structure (De-)Serialization
------------------------------------------------------------

Common
~~~~~~

.. code-block:: cpp

    struct WheelData
    {
        uint32_t rpm;
        float temperature;
    };

Publisher - Wheel
~~~~~~~~~~~~~~~~~

.. code-block:: cpp

    // creation of the data publisher

    SilKit::Services::PubSub::PubSubSpec pubSpec{"Wheel", SilKit::Util::SerDes::MediaTypeData()};
    pubSpec.AddLabel("Instance", "FrontLeftWheel", SilKit::Services::MatchingLabel::Kind::Optional);

    auto* publisher = participant->CreateDataPublisher("PubFrontLeftWheel", pubSpec, 1);

    // serialization of data and publishing

    auto wheelData{model.GetFrontLeftWheelData()};

    SilKit::Util::SerDes::Serializer serializer;
    serializer.BeginStruct();
    serializer.Serialize(wheelData.rpm);
    serializer.Serialize(wheelData.temperature);
    serializer.EndStruct();

    publisher->Publish(serializer.ReleaseBuffer());

Subscriber - Wheel Monitor
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

    // creation of the data subscriber

    SilKit::Services::PubSub::PubSubSpec subSpec{"Wheel", SilKit::Util::SerDes::MediaTypeData()};
    subSpec.AddLabel("Instance", "FrontLeftWheel", SilKit::Services::MatchingLabel::Kind::Optional);

    auto subscriberDataHandler = [&wheelMonitorModel](IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent)
    {
        WheelData wheelData;

        // deserialization and processing

        SilKit::Util::SerDes::Deserializer deserializer;
        deserializer.BeginStruct();
        wheelData.rpm = deserializer.Deserialize<uint32_t>();
        wheelData.temperature = deserializer.Deserialize<float>();
        deserializer.EndStruct();

        model.ProcessFrontLeftWheelData(wheelData);
    });

    auto* subscriber = participant->CreateDataSubscriber("SubFrontLeftWheel", subSpec, subscriberDataHandler);


Advanced Usage and Configuration
================================

Topics
------

Data publishers and data subscribers provide a topic name which is part of their |PubSubSpec|.
Communications only takes place among controllers with the same topic. The topic has no wildcard functionality.

Media Type
----------

Both data publishers and data subscribers define a media type as part of their |PubSubSpec|.
It is a meta description of the transmitted data in accordance to `RFC2046 <https://datatracker.ietf.org/doc/html/rfc2046>`_ and should be used to provide information about the de-/serialization of the underlying user data.
Just like the topic, the media type has to match between data publishers/subscribers for communication to take place.
An empty string on a data subscriber is a wildcard and will match any other media type of data publishers.
Data publishers should provide information about the data they are going to publish and have no wildcard functionality for the media type.

When data is serialized using SIL Kit's :doc:`Data Serialization/Deserialization API</api/serdes>`, the media type constant |MediaTypeData| must be used.

Labels
------

Both data publishers and data subscribers can be annotated with string-based key-value pairs (labels) which can be either mandatory or optional.
In addition to the matching requirements given by topic and media type, data publishers and Data subscribers will only communicate if their labels match.

The labels are stored in the |PubSubSpec|. A |MatchingLabel| can be added via |AddLabel|, see the following code snippet:

.. code-block:: cpp

    SilKit::Services::PubSub::PubSubSpec subDataSpec{"WheelSpeed", "application/json"};
    subDataSpec.AddLabel("Instance", "FrontLeftWheel", SilKit::Services::MatchingLabel::Kind::Optional);
    auto* subscriber = participant->CreateDataSubscriber("Sub1", subDataSpec, defaultDataHandler);

To communicate, data publishers and data subscribers must conform to the following matching rules:

* A mandatory label matches, if a label of the same key and value is found on the corresponding counterpart.
* An optional label matches, if the label key does not exist on the counterpart or both its key and value are equal.

The following table shows how data publishers and data subscribers with matching topics and matching media type would match corresponding to their labels.
Note that the label matching is symmetric, so publishers and subscribers are interchangeable here.

.. list-table:: Label combinations
   :header-rows: 1

   * -
     - Subscriber {"Instance", "FrontLeft", Optional}
     - Subscriber{"Instance", "FrontLeft", Mandatory}
   * - Publisher {}
     - Match
     - No Match
   * - Publisher {"Instance", "FrontLeft", Optional}
     - Match
     - Match
   * - Publisher {"Instance", "RearRight", Optional}
     - No Match
     - No Match
   * - Publisher {"Namespace", "Car", Optional}
     - Match
     - No Match
   * - Publisher {"Namespace", "Car", Mandatory}
     - No Match
     - No Match

History
-------

Data publishers additionally specify a history length N (restricted to 0 or 1).
Data subscribers that are created after a publication will still receive the N historic data messages from a data publisher with history > 0.
Note that the participant that created the data publisher still has to be connected to the distributed simulation for the historic messages to be delivered.

Configuration
-------------

The controller name passed in |CreateDataPublisher| and |CreateDataSubscriber| is used to identify the controller in a YAML configuration.
Currently, only the topic can be configured.
If a topic is set in the configuration, it will be preferred over a programmatically set topic.

.. code-block:: yaml

    ParticipantName: Participant1

    DataPublishers:
      - Name: DataPublisherController1
        Topic: TopicA

    DataSubscribers:
      - Name: DataSubscriberController1
        Topic: TopicB
