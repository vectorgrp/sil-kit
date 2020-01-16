===================
Generic Message API
===================
.. Macros for docs use
.. |IComAdapter| replace:: :cpp:class:`IComAdapter<ib::mw::IComAdapter>` 
.. |CreateGenericPublisher| replace:: :cpp:func:`CreateGenericPublisher<ib::mw::IComAdapter::CreateGenericPublisher()>`
.. |CreateGenericSubscriber| replace:: :cpp:func:`CreateGenericSubscriber<ib::mw::IComAdapter::CreateGenericSubscriber()>`
.. |Publish| replace:: :cpp:func:`Publish()<ib::sim::generic::IGenericPublisher::Publish()>` 
.. |SetReceiveMessageHandler| replace:: :cpp:func:`SetReceiveMessageHandler()<ib::sim::generic::IGenericSubscriber::SetReceiveMessageHandler()>`
.. |IGenericPublisher| replace:: :cpp:class:`IGenericPublisher<ib::sim::generic::IGenericPublisher>`
.. |IGenericSubscriber| replace:: :cpp:class:`IGenericPublisher<ib::sim::generic::IGenericSubscriber>`
.. contents::
   :local:
   :depth: 3
   
Using the Generic Message API
-----------------------------
A Generic Message is a plain byte vector which contains arbitrary user data.
It can be distributed among the participants of a simulation using a publish / subscribe mechanism.
One publisher can be connected to several subscribers, without modelling an underlying network or bus.
Published messages are immediately transmitted to all connected subscribers, that is, without any
modelled latency.

Publishers and subscribers are identified by a name and are connected by links.
For each link there is exactly one publisher, and possibly many subscribers.
Publisher and subscribers may only be connected by a single link.
That is, a susbcriber may not be served by mutliple publishers.


The Publisher and Subscriber interfaces are instantiated from an |IComAdapter| 
interface by calling |CreateGenericPublisher| and |CreateGenericSubscriber|, respectively.
Their name is used in the configuration and instantiation of the interfaces.
Additionally, publishers can specify a protocol and a datatype definition URI, which can be
queried by the subscriber.

Data can be transmitted using the |Publish| method.
The data is received asynchronously and delivered via a callback, which can 
be set on a subscriber using the |SetReceiveMessageHandler| method.

Usage Examples
~~~~~~~~~~~~~~
The interfaces for the publish/subscribe mechanism can be instantiated from an IComAdapter:

.. code-block:: cpp

    auto comAdapter = ib::CreateComAdapter(std::move(config), participant_name, domainId);
    auto* publishData = comAdapter->CreateGenericPublisher("Message1");
    publishData->Publish(user_data);

    auto* subscribeData = comAdapter->CreateGenericSubscriber("Message1");
    subscribeData->SetReceiveMessageHandler([](IGenericSubscriber* subscriber,
                        const std::vector<uint8_t>& data) {
        //handle data
    });

For a full example refer to the :ref:`VIB Quick Start Guide<sec:quickstart-simple>` 
which contains a simple application that demonstrates the usage of the Generic Message 
API in detail. 

API and Data Type Reference
--------------------------------------------------
The |IGenericPublisher| provides a simple publish interface for standard vector.
For convenience an overload for raw data pointer  and size exists.

The |IGenericSubscriber| provides a callback registration mechanism.

The publisher's and subscriber's read-only :cpp:class:`configuration<ib::cfg::GenericPort>`
can also be accessed.
The protocol and data type definition can be set up via the configuration mechanism.


Generic Publisher API
~~~~~~~~~~~~~~~~~~~~~
    .. doxygenclass:: ib::sim::generic::IGenericPublisher
       :members:

Generic Subscriber API
~~~~~~~~~~~~~~~~~~~~~~

    .. doxygenclass:: ib::sim::generic::IGenericSubscriber
       :members:
