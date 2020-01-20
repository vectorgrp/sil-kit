===============
VIB Quick Start
===============

.. |ComAdapter| replace:: :ref:`ComAdapter<sec:comadapter-factory>`

.. contents::
   :depth: 3

The Vector Integration Bus (VIB) is a C++ library for the distributed simulation of automotive networks.
It is designed to work on Windows and Linux.
This quick start demonstrates how to get started using the pre-built VIB release distribution.

For building from the source tree, refer to :doc:`../development/build`.

This guide will walk you through setting up a VIB project from scratch.
First the terminology required to understand the function is briefly discussed, then the build process and the actual code.


Introduction
------------
Have a look at our :ref:`architecture overview <base-architecture>` to get a high level introduction to the VIB and its features.


.. list-table:: Terminology
 :widths: 20 80

 * - :ref:`Participant<sec:cfg-participant>`
   - A communication node in the distributed simulation. Every simulation must at least define one participant.
 * - :doc:`Configuration<../configuration/simulation-setup>`
   - A simulation is defined by its configuration.
 * - Domain
   - A numerical label to uniquely identify a simulation run. This allows running multiple simulations on the same hosts.
 * - :ref:`Links<sec:cfg-links>`
   - A virtual connection between the components of a simulation, e.g. between a participant and a service.
 * - :doc:`Middleware<../configuration/middleware-configuration>`
   - The concrete distributed communication implementation. That is, the software layer implementing the distributed message passing mechanism.
 * - |ComAdapter|
   - Entry point to the VIB library. Abstracts away the underlying middleware.
     Allows creation of network controllers and services.
 * - :doc:`Synchronization<../configuration/simulation-setup>`
   - A configuration option that determines if and how a participant synchronizes with all other participants.
 * - :ref:`Simulation Time <sec:cfg-time-sync>`
   - The granularity of the simulation time depends on the configured protocol.

A simulation consists of participants, which all share the same domain identifier.
The participants might be physically distributed in a network, but not necessarily so.
The simulation is identified by its domain among all network hosts taking part in the simulation.

.. admonition:: Note

    For the FastRTPS middleware the domain ID must be in the range [1, 232].

Thus, it is feasible to have multiple simulations running in parallel on the same host computer.
Some participants have special roles, depending on the configuration of the middleware, syncronization protocol or :doc:`../vibes/overview`.
Some configurations require auxiliary programs to run on your host computer(s), for example the :ref:`VAsio Middleware<sec:mwcfg-vasio>` requires the :ref:`sec:util-registry` to work properly.
The :ref:`sec:util-launcher` is designed to simplify starting ensembles of programs that make up elaborate simulations environments.

Writing your first VIB application
----------------------------------
This tutorial assumes that you are familiar with `CMake (https://cmake.org) <https://cmake.org>`_ and C++.

Using the VIB package
~~~~~~~~~~~~~~~~~~~~~
The VIB distribution contains a self-contained and deployable installation in the *IntegrationBus* directory.
The  CMake build configuration required is exported to ``IntegrationBus/lib/cmake/IntegrationBus`` and defines the ``IntegrationBus::IntegrationBus`` target. 

From CMake this can be easily used via the  ``find_package(IntegrationBus CONFIG)`` mechanism.
For example, the following CMakeLists.txt is able to import the IntegrationBus library based on its filesystem path.

.. literalinclude::
   sample_vib/CMakeLists.txt
   :language: cmake

Properties, like include directories and compile flags, are automatically handled by the imported target.
If you use another method to build your software you can directly use the ``IntegrationBus/include`` and ``IntegrationBus/lib`` directories for C++ headers and libraries.

.. _sec:quickstart-simple:

A simple Generic Message application
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
We'll create a simple, self-contained VIB application that uses :doc:`Generic messages<../api/genericmessage>` to exchange user-defined data among its participants.
The messages are exchanged using a publish / subscribe pattern.

To use the VIB you first have to create a valid configuration.
This can either be done by loading an existing :ref:`JSON file<sec:ibconfig-json>` or 
by using the :doc:`configuration builder API<../api/config>` to create one programmatically.

We use a configuration file ``simple.json`` for creating our simulation.
The file will be loaded by our application and from helper :doc:`utilities`::

    auto config = Config::FromJsonFile("simple.json")


The configuration consists of the active middleware, two participants that make up 
our simulation (``PublisherParticipant`` and ``SubscriberParticipant``), and additional
information like the synchronization mode and granularity.
The participants both have links to the ``DataService`` generic message resource.
These configured names will also be referenced directly in the C++ code.

The application will run two participants concurrently, each in their own thread.
One thread will act as a publisher by sending a test string to its subscribers:

.. literalinclude::
   sample_vib/simple.cpp
   :language: cpp
   :lines: 16-41


First the configured middleware domain is joined as the named participant using the
|ComAdapter|.
Creating the ComAdapter properly initializes the VIB library and allows to instantiate
:doc:`services<../api/api>` and :doc:`participant controllers<../api/participantcontroller>`.

Next, we create a :cpp:class:`publisher<ib::sim::generic::IGenericPublisher>` for the ``DataService`` resource.
This allows sending data through its :cpp:func:`Publish()<ib::sim::generic::IGenericPublisher::Publish()>`
method, when we are in an active simulation.

The actual simulation is performed in the simulation task.
This task is a callback that has to be registered with the participant controller's 
:cpp:func:`SetSimulationTask()<ib::mw::sync::IParticipantController::SetSimulationTask()>`.
The task is then evoked by the VIB runtime.

To start the runtime and perform the actual simulation work in the current thread,
the blocking :cpp:func:`Run()<ib::mw::sync::IParticipantController::Run()>`
method must be invoked.

The subscriber runs in its own thread, too:

.. literalinclude::
   sample_vib/simple.cpp
   :language: cpp
   :lines: 43-69

The setup is similar to the publisher, except that we instantiate a 
:cpp:class:`subscriber<ib::sim::generic::IGenericSubscriber>` interface.
This allows us to register a
:cpp:func:`SetReceiveMessageHandler()<ib::sim::generic::IGenericSubscriber::SetReceiveMessageHandler()>`
callback to receive data value updates.
The simulation task has to be defined, even though no simulation work is performed.

To run this sample you have to use the  :ref:`sec:util-registry` and :ref:`sec:util-system-controller` processes.
The registry is required by the :ref:`VAsio middleware<sec:mwcfg-vasio>` for connecting the participants and setting up the distributed services.
The SystemController takes care of starting the simulation when all required participants are connected and properly configured and in a well defined state -- this is a task that is required in every simulation.
For convenience and to reduce code duplication, these utility programs are implemented in separate executables and distributed in binary forms.

The complete source code of this sample: :download:`CMakeLists.txt<sample_vib/CMakeLists.txt>` :download:`simple.cpp<sample_vib/simple.cpp>` :download:`simple.json<sample_vib/simple.json>`


Further Reading
---------------
More real-world examples, involving time synchronization and simulated
automotive networks, can be found in the :doc:`API<../api/api>` sections.
Also, studying the source code and mode of operation of the bundled
:doc:`demo applications<demos>` is a good start.
The simulation lifecycle and supported simulation time synchronization
is discussed in :doc:`../vib-simulation`.
