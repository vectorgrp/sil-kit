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
   - A communication node in the distributed simulation. Every VIB application must at least define one participant.
 * - Domain
   - A numerical label to uniquely identify a simulation run. This allows running multiple simulatons on the same hosts.
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


A simple VIB application
~~~~~~~~~~~~~~~~~~~~~~~~
We'll create a simple, self-contained VIB application that uses a :doc:`generic publish/subscribe mechanism<../api/genericmessage>` to exchange messages among its participants.
To use the VIB you first have to create a valid configuration.
This can either be done by loading a :ref:`JSON file<sec:ibconfig-json>` or by using the configuration builder :doc:`API<../api/config>` -- we'll use the API.


.. literalinclude::
   sample_vib/simple.cpp
   :language: cpp

We created a configuration with the two participants ``PublisherParticipant`` and ``SubscriberParticipant``.
They both have links to the ``SharedService`` generic message.
