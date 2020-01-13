===================
Generic Message API
===================

.. contents::
   :local:
   :depth: 3
   
Using the Generic Message API
-----------------------------
A Generic Message is a plain byte vector which contains arbitrary user data.
It can be disseminated among the participants of a simulation using a publish / subscribe mechanism.
The message is identified by its name, and optionally a protocol string and a definition URI can be configured.
The name is used in the configuration and creation of the Generic Message interfaces.
The Generic Message interfaces are instantiated from an :cpp:class:`IComAdapter<ib::mw::IComAdapter>` interface by calling :cpp:func:`CreateGenericPublisher<ib::mw::IComAdapter::CreateGenericPublisher()>` and :cpp:func:`CreateGenericSubscriber<ib::mw::IComAdapter::CreateGenericSubscriber()>` respectively.

Messages can be transmitted using the :cpp:func:`Publish()<ib::sim::generic::IGenericPublisher::Publish()>` method.
The Generic Messages are received asynchronously and delivered via a callback, which can be set on a subscriber using the :cpp:func:`SetReceiveMessageHandler()<ib::sim::generic::IGenericSubscriber::SetReceiveMessageHandler()>` method.

Usage Examples
~~~~~~~~~~~~~~

A Generic Message ``Message1`` can be declared using the config builder API:

.. code-block:: cpp

   ConfigBuilder config("Example");
   auto&& setup = config.SimulationSetup();
   setup.AddParticipant("Producer")
        .AddGenericPublisher("Message1");
   setup.AddParticipant("Consumer")
        .AddGenericSubscriber("Message1");

Then the interfaces for the publish/subscribe mechanism can be instantiated:

.. code-block:: cpp

    auto&& comAdapter = ib::CreateComAdapter(std::move(config), participant_name, domainId);
    auto&& publishData = comAdapter->CreateGenericPublisher("Message1");
    publishData->Publish(user_data);

    auto&& subscribeData = comAdapter->CreateGenericSubscriber("Message1");
    subscribeData->SetReceiveMessageHandler([](IGenericSubscriber* subscriber,
                        const std::vector<uint8_t>& data) {
        //handle data
    });

For a full example refer to the :ref:`VIB Quick Start Guide<sec:quickstart-simple>` 
which contains a simple application that demonstrates the usage of the Generic Message 
API in detail. 

API and Data Type Reference
--------------------------------------------------
Generic Publisher API
~~~~~~~~~~~~~~~~~~~~~
    .. doxygenclass:: ib::sim::generic::IGenericPublisher
       :members:

Generic Subscriber API
~~~~~~~~~~~~~~~~~~~~~

    .. doxygenclass:: ib::sim::generic::IGenericSubscriber
       :members:

Data Structures
~~~~~~~~~~~~~~~~~~~~~
    .. doxygenstruct:: ib::sim::generic::GenericMessage
       :members: 


