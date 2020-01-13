===============
VIB Quick Start
===============

.. contents::
   :depth: 3

The Vector Integration Bus (VIB) is a C++ library for distributed simulation of automotive networks.
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
 * - :ref:`ComAdapter<sec:comadapter-api>`
   - Entry point to the VIB library. Abstracts away the underlying middleware.
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
The :ref:`sec:util-launcher` is designed to simplify starting ensembles of programs that make up elaborate simulations.

Writing your first VIB application
----------------------------------
This tutorial assumes that you are familiar with `CMake (https://cmake.org) <https://cmake.org>`_ and C++.

Using the VIB package
~~~~~~~~~~~~~~~~~~~~~
The VIB distribution contains a self-contained and deployable installation in the *IntegrationBus* directory.
The  CMake build configuration required is exported to ``IntegrationBus/lib/cmake/IntegrationBus`` and defines the ``IntegrationBus::IntegrationBus`` target. 

From CMake this can be easily used via the  ``find_package(IntegrationBus CONFIG)`` mechanism.
For example, the following CMakeLists.txt is able to import the IntegrationBus library based on its filesystem path (VIB_DIR)

.. literalinclude::
   sample_vib/CMakeLists.txt
   :language: cmake

Properties, like include directories and compile flags, are automatically handled by the imported target.
If you use another method to build your software you can directly use the ``IntegrationBus/include`` and ``IntegrationBus/lib`` directories for C++ headers and libraries.

.. _sec:quickstart-simple:

A simple publish / subscribe application
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
We'll create a simple, self-contained VIB application that uses a :doc:`generic publish/subscribe mechanism<../api/genericmessage>` to exchange messages among its participants.
To use the VIB you first have to create a valid configuration.
This can either be done by loading a :ref:`JSON file<sec:ibconfig-json>` or by using the configuration builder :doc:`API<../api/config>` to create one programmatically.

We use the API to create a configuration and save it to a file, for later use with supporting :doc:`utilities`.

.. literalinclude::
   sample_vib/simple.cpp
   :lines: 89-112
   :language: cpp

The configuration consists of the middleware, the two participants that make up 
the simulation (``PublisherParticipant`` and ``SubscriberParticipant``) and the synchronization mode and granularity.
The participants both have links to the ``SharedService`` generic message.

The application will run two participants concurrently, each in their own thread.
The threads use the same code but differentiate the behavior based on the participants name: the publisher creates new a new generic message every 10ms.


.. literalinclude::
   sample_vib/simple.cpp
   :lines: 19-84
   :linenos:
   :language: cpp

To manage the participants lifecycle different handlers can be attached, cf. line 11.
The participants core simulation logic is in the simulation task callback (line 27), which is evoked by the VIB runtime.
The subscribers simulation task is left empty, as new generic messages are handled by the receive message handler callback (line 45).
Publishing data is accomplished by creating a IGenericPublisher interface from the com adapter and invoking its Publish method.
Generic message subscription is done via the IGenericSubscriber interface and registering a callback.

To run this sample you have to use the  :ref:`sec:util-registry` and :ref:`sec:util-system-controller` processes.
The registry is required by the :ref:`VAsio middleware<sec:mwcfg-vasio>` for connecting the participants and setting up the distributed services.
The SystemController implements the state machine for the distributed simulation.
It takes care of starting the simulation when all required participants are connected and properly configured -- this is a task that is required in every simulation.
For convenience and to reduce code duplication, these utility programs are implemented in separate executables and distributed in binary forms.

The complete source code of this sample: :download:`CMakeLists.txt<sample_vib/CMakeLists.txt>` :download:`simple.cpp<sample_vib/simple.cpp>` 


Further Reading
---------------

More real-world examples can be found in the :doc:`API<../api/api>` sections for the simulated automotive networks.
Also, studying the source code and mode of operation of the bundled :doc:`demo applications<demos>` is a good start.
