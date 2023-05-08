=============================================
Vector SIL Kit
=============================================

The Vector SIL Kit is a runtime component that enables distributed simulation for
automotive applications. For this, the Vector SIL Kit provides:

* Communication on different abstraction levels including

  * Vehicle networks (CAN / CAN FD, Ethernet, FlexRay, LIN)
  * High level data messages for arbitrary application specific communication in a publish/subscribe pattern
  * Remote Procedure Calls for calling arbitrary procedures on remote participants

* Synchronization of virtual simulation time

  * Event based synchronization according to lower bound event time

* State handling to control and observe execution of the simulated system
* An optional participant configuration YAML/JSON file to configure simulation participants behavior after
  compile time


For getting started developing with the Vector SIL Kit, see the :doc:`quick start guide <usage/quickstart>`.

Architecture
----------------------------

The Vector SIL Kit implements a layered architecture comprising three layers:

.. figure:: _static/SilKitArchitecture.svg
    :width: 800
    
|
|

Supported Services
------------------

The currently supported services of the SIL Kit consist of the following categories:

* **Vehicle Network Controllers:** CAN / CAN FD, Ethernet, FlexRay, and LIN
* **Data Publish/Subscribe:** without any data type constraints
* **RPC Servers/Clients:** for remote procedure call functionality

Vehicle Networks
~~~~~~~~~~~~~~~~

The SIL Kit provides means to simulate CAN / CAN FD, Ethernet, FlexRay, and LIN networks.
All networks except for FlexRay can be simulated with two different levels of detail: a simple, functional simulation
or a detailed simulation with accurate timings. Simulating accurate timings requires the detailed simulation, which
needs an additional network simulator. Because of its intrinsic complexity, FlexRay is only provided in a detailed
simulation.

Vehicle Network Controllers
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Applications access vehicle networks using matching controller models. The
controller interfaces are the same for simple and detailed simulation. As a result,
an application that works in the simple use case also works when switching to
a detailed simulation using the network simulator. The CAN, LIN, and Ethernet demos
provided with the SIL Kit have all been tested both with and without a network simulator.
The FlexRay feature is only available in the detailed simulation based on a network simulator.

The section :ref:`sec:api-services` describes how to configure and use Vehicle Network Controllers in detail.
In addition, the use of other services (Data Message, RPC, ...) is presented as well.


Prerequisites for Usage
------------------------

* For Windows:
    * Visual Studio 2017 (toolset v141) and higher
* For Ubuntu 18.04 LTS:
    * GCC 8 **or**
    * Clang

These are specific versions the Vector SIL Kit is tested and built against.
