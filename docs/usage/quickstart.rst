===============
Quick Start
===============

.. |Participant| replace:: :ref:`Participant<sec:iparticipant-api>`

.. contents::
   :local:
   :depth: 3

The Vector SIL Kit is a C++ library for the distributed simulation of automotive networks.
It is designed to work on Windows and Linux.
This quick start demonstrates how to get started using the pre-built SIL Kit release distribution.

This guide will walk you through setting up a SIL Kit project from scratch. First, the terminology
required to understand the functionality is briefly discussed, then the build process and the actual code.

.. _sec:quickstart-terminology:

Terminology
-----------

.. list-table:: 
 :widths: 20 80

 * - :doc:`Participant<../api/participant>`
   - A communication node in the distributed simulation and entry point to the SIL Kit library. 
     Allows creation of vehicle network controllers and other services. 
 * - :ref:`Services<sec:api-services>`
   - Participants interact with each other through the means of services, e.g.,
     a :doc:`CAN Controller<../api/can>` or a :doc:`DataPublisher<../api/pubsub>`. A special service is the
     :doc:`Life Cycle Service<../api/lifecycleService>`, which
     provides state handling and access to the time synchronization service.
 * - :doc:`Configuration<../configuration/configuration>`
   - The optional participant configuration file allows to easily configure a participant and its interconnection within the 
     simulation. It can be used to change a participants behavior without needing to recompile its sources.
 * - Registry
   - The registry is a central service that enables participant discovery in a distributed simulation.
 * - Registry URI
   - The registry URI specifies where the registry can be reached.
     It defaults to 'silkit://localhost:8500', that is, the registry is reachable via TCP on the localhost on port 8500.
 * - :doc:`Middleware<../configuration/middleware-configuration>`
   - The concrete distributed communication implementation. That is, the software layer
     implementing the distributed message passing mechanism.
 * - :ref:`Simulation Time <sec:sim-synchronization>`
   - The simulated time within a simulation as it is perceived by a participant. Participants might be synchronized or
     unsynchronized.

A simulation consists of participants which all connect to the same registry URI.
An instance of the registry is required for coordination, either as a standalone process (see :ref:`sec:util-registry`) or created programmatically.
The participants might be physically distributed in a network or running on the same host.

Thus, it is feasible to have multiple simulations running in parallel on the same host computer.
Some participants can have special roles, depending on e.g., the synchronization and detail of the simulation.

Writing your first SIL Kit application
--------------------------------------
This tutorial assumes that you are familiar with `CMake (https://cmake.org) <https://cmake.org>`_ and C++.

Using the SIL Kit package
~~~~~~~~~~~~~~~~~~~~~~~~~
The SIL Kit distribution contains a self-contained and deployable installation in the *SilKit* directory.
The  CMake build configuration required is exported to ``SilKit/lib/cmake/SilKit`` and
defines the ``SilKit::SilKit`` target. 

From CMake this can be easily used via the  ``find_package(SilKit CONFIG)`` mechanism.
For example, the following CMakeLists.txt is able to import the SIL Kit library based on its filesystem path.

.. literalinclude::
   sample_silkit/CMakeLists.txt
   :language: cmake
   :lines: 22-31

Properties, like include directories and compile flags, are automatically handled by the imported target.
If you use another method to build your software you can directly use the ``SilKit/include`` and
``SilKit/lib`` directories for C++ headers and libraries.

.. _sec:quickstart-simple:

A simple Publish / Subscribe application
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
We'll create a simple, self-contained SIL Kit application that uses :doc:`Publish/Subscribe<../api/pubsub>`
to exchange user-defined data among its participants.

To use the SIL Kit, you first have to create a valid configuration.  This can 
be done by loading an existing :ref:`YAML file<sec:ibconfig-json>`.

We use a configuration file ``simple.yaml`` for creating our simulation.
The file will be loaded by our application and from helper :doc:`utilities`::

    auto config = SilKit::Config::ParticipantConfigurationFromFile("simple.yaml")


The configuration file itself contains an empty JSON object, that later on can be used to configure our simulation participants 
without recompiling it.

.. literalinclude::
   sample_silkit/simple.yaml
   :language: yaml

The application will run two participants concurrently, each in their own thread.
One thread will act as a publisher by sending a test string to its subscribers:

.. literalinclude::
   sample_silkit/simple.cpp
   :language: cpp
   :lines: 36-65


First, the simulation is joined by creating a participant ``PublisherParticipant``.
Creating the Participant properly initializes the SIL Kit library and allows to instantiate
:doc:`Services<../api/api>` and offers access to the
:doc:`Life Cycle Service<../api/lifecycleService>`.

Next, we create a :cpp:class:`publisher<SilKit::Services::PubSub::IDataPublisher>` for the ``DataService`` topic.
This allows sending data through its :cpp:func:`Publish()<SilKit::Services::PubSub::IDataPublisher::Publish()>`
method.

The actual simulation is performed in the simulation task. The simulation task
is a callback that is executed by the SIL Kit runtime whenever the the simulation
time advances. This callback has to be registered with the time synchronization service's
:cpp:func:`SetSimulationStepHandler()<SilKit::Services::Orchestration::ITimeSyncService::SetSimulationStepHandler()>`.

The subscriber runs in its own thread, too:

.. literalinclude::
   sample_silkit/simple.cpp
   :language: cpp
   :lines: 67-97

The setup is similar to the publisher, except that we instantiate a 
:cpp:class:`subscriber<SilKit::Services::PubSub::IDataSubscriber>` interface.
This allows us to register a
:cpp:func:`SetDataMessageHandler()<SilKit::Services::PubSub::IDataSubscriber::SetDataMessageHandler()>`
callback to receive data value updates.
The simulation task has to be defined, even though no simulation work is performed.

To run this sample, you have to use the :ref:`sec:util-registry` and
:ref:`sec:util-system-controller` processes.
The registry is required for participants discovery.
The :ref:`sec:util-system-controller` initializes the connected participants and starts the
simulation until the return key is pressed. For convenience and to reduce code
duplication, these utility programs are implemented in separate executables and
distributed in binary forms.

The final simulation setup can be run through the following commands:

.. code-block::
      
      # Start the Middleware Registry
      ./sil-kit-registry.exe

      # Start the System Controller and tell it to wait for PublisherParticipant and SubscriberParticipant
      ./sil-kit-system-controller.exe PublisherParticipant SubscriberParticipant

      # Start the application running the two participants
      # Make sure that the SilKit.dll and simple.yaml are available 
      ./SampleSilKit.exe

The complete source code of this sample can be found here: :download:`CMakeLists.txt<sample_silkit/CMakeLists.txt>`
:download:`simple.cpp<sample_silkit/simple.cpp>` :download:`simple.yaml<sample_silkit/simple.yaml>`


Further Reading
---------------

More real-world examples involving time synchronization and simulated
automotive networks, can be found in the :doc:`API sections<../api/api>`.  Also,
studying the source code of the bundled :doc:`demo applications<demos>` is a
good start. The simulation lifecycle and supported simulation time
synchronization are discussed in :doc:`../simulation/simulation`. 
Additionally, :doc:`../configuration/configuration` describes how the participant
configuration file can be used to change the behavior of participants.
