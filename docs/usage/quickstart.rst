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
   - The optional participant configuration file allows easy configuration of a participant and its interconnection within the 
     simulation. It can be used to change a participant's behavior without recompiling its sources.
 * - Registry
   - The registry is a central service that enables participant discovery in a distributed simulation.
 * - Registry URI
   - The registry's URI specifies where the registry can be reached.
     It defaults to ``silkit://localhost:8500``, that is, the registry is reachable via TCP on the 'localhost' on port 8500.
 * - :doc:`Middleware<../configuration/middleware-configuration>`
   - The concrete distributed communication implementation. That is, the software layer
     implementing the distributed message passing mechanism.
 * - :ref:`Simulation Time <sec:sim-synchronization>`
   - The simulated time within a simulation as it is perceived by a participant. Participants might be synchronized or
     unsynchronized.

A simulation consists of participants which all connect to the same registry's URI.
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
The CMake build configuration required is exported to ``SilKit/lib/cmake/SilKit`` and
defines the ``SilKit::SilKit`` target. 

From CMake this can be easily used via the ``find_package(SilKit CONFIG)`` mechanism.
For example, the following CMakeLists.txt is able to import the SIL Kit library based on its file system path.

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
to exchange user-defined data among two participants. First, we include the system and SIL Kit headers and 
define namespaces and constants in our C++ file ``simple.cpp``:

.. literalinclude::
   sample_silkit/simple.cpp
   :language: cpp
   :lines: 22-31

SIL Kit participants are created with a configuration that is used to change certain aspects of the simulation 
without recompiling the application. This can be done by loading an existing :ref:`YAML file<sec:ibconfig-json>`.
Here, we use the configuration file ``simple.yaml`` that logs all error messages to a file:

.. literalinclude::
   sample_silkit/simple.yaml
   :language: yaml

We load it in the main function of our code::

    int main(int argc, char** argv)
    {
        auto config = SilKit::Config::ParticipantConfigurationFromFile("simple.yaml");
        // TODO: Use config to create participants
    }

The application will run two participants concurrently, each in its own thread.
One thread will act as a publisher by sending a test string to its subscribers:

.. literalinclude::
   sample_silkit/simple.cpp
   :language: cpp
   :lines: 33-69

First, the simulation is joined by creating the participant called "PublisherParticipant".
This properly initializes the SIL Kit library; enables the instantiation of
:doc:`Services<../api/api>` and offers access to the :doc:`Life Cycle Service<../api/lifecycleService>`, which 
controls the orchestration of our simulation. Next, we create a 
:cpp:class:`publisher<SilKit::Services::PubSub::IDataPublisher>` for the ``DataService`` topic. Later, we subscribe 
to the same topic name in our subscriber to enable communication between the participants. The actual simulation 
is performed in the simulation task. This is a callback that is executed by the SIL Kit runtime whenever the
simulation time advances. This callback has to be registered with the time synchronization service's
:cpp:func:`SetSimulationStepHandler()<SilKit::Services::Orchestration::ITimeSyncService::SetSimulationStepHandler()>`.
We hand over the publisher object in the capture list of our simulation task and use it to send data through its 
:cpp:func:`Publish()<SilKit::Services::PubSub::IDataPublisher::Publish()>` method.

The subscriber runs in its own thread, too:

.. literalinclude::
   sample_silkit/simple.cpp
   :language: cpp
   :lines: 71-102

The setup is similar to the publisher, except that we instantiate a 
:cpp:class:`subscriber<SilKit::Services::PubSub::IDataSubscriber>` interface. This allows us to register a
:cpp:func:`SetDataMessageHandler()<SilKit::Services::PubSub::IDataSubscriber::SetDataMessageHandler()>`
callback to receive data value updates. The simulation task has to be defined, even though no simulation work is
performed.

We extend our main function to spawn both threads and join them again once finished.
Also, we use a try-catch block here to get proper error handling e.g. if the configuration file cannot be loaded.

.. literalinclude::
   sample_silkit/simple.cpp
   :language: cpp
   :lines: 104-127

The application is built with CMake on the command line (from a build directory) by calling ``cmake ..`` to generate 
and then build via ``cmake --build .``. A more convenient way is to open the folder in an IDE with CMake support.
To run this sample, copy the shared library files (e.g. on Windows the ``SilKit.dll``, ``SilKitd.dll`` from ``SilKit/bin``) and the ``simple.yaml`` next 
to the compiled executable.

Running the simulation
~~~~~~~~~~~~~~~~~~~~~~

Our sample needs the utility processes :ref:`sec:util-registry` and :ref:`sec:util-system-controller` to run. The 
registry is required for participant discovery. The :ref:`sec:util-system-controller` takes the participant names
as command line arguments, initializes the connected participants and starts the simulation until the return key is 
pressed. For convenience and to reduce code duplication, these utility programs are implemented in separate executables
and distributed in binary forms.

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
