=============================================
Vector SilKit
=============================================

The Vector SilKit is a runtime component that enables distributed simulation for
automotive applications. For this, the Vector SilKit provides:

* Communication on different abstraction levels including

  * Vehicle networks (CAN / CAN FD, Ethernet, FlexRay, LIN)
  * High level data messages for arbitrary application specific communication in a publish / subscribe pattern
  * Remote Procedure Calls for calling arbitrary procedures on remote participants

* Synchronization of simulation time implementing different protocols

  * Event based synchronization according to lower bound event time

* State handling to control and observe execution of the simulated system
* An optional participant configuration YAML/JSON file to configure simulation participants behavior even after
  compile time


For getting started developing with the Vector SilKit, see the :doc:`quickstart guide <usage/quickstart>`.

.. _base-architecture:

Architecture
----------------------------

The Vector SilKit implements a layered architecture comprising three layers:

.. figure:: _static/SilKitArchitecture.svg
    :align: center

Supported Services
------------------

The currently supported services of the SILKIT consist of the following categories:

* **Vehicle Network Controllers:** CAN / CAN FD, Ethernet, FlexRay and LIN
* **Data Messages:** without any data type constraints
* **RPC Servers/Clients:** for remote procedure call functionality

Vehicle Networks
~~~~~~~~~~~~~~~~

The SILKIT provides means to simulate CAN / CAN FD, Ethernet, FlexRay, and LIN networks.
All networks can be simulated with two different levels of detail: simple, functional simulation
or high detail, timing accurate simulation. Timing accurate simulation requires the detailed simulation, which  
needs an additional network simulator.

Vehicle Network Controllers
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Applications access vehicle networks using matching controller models. The
controller interfaces are the same for simple and detailed simulation. As a result,
an application that works in the simple use case does also work when switching to
a high detail simulation using the network simulator. The CAN, LIN, and Ethernet demos 
provided with the SILKIT have all been tested both with and without a network simulator. 
The FlexRay feature is only available in the detailed simulation based on a network simulator.

The section :ref:`sec:api-services` describes how to configure and use Vehicle Network Controllers in detail. 
In addition, the use of other services (Data Message, Rpc, ...) is presented as well.


Prerequisites for Usage
---------------------------

* For Windows:
    * Visual Studio 2015 Toolset v140 and higher (also tested with MSVC++ 14.12)
* For Ubuntu Bionic Beaver (18.04 LTS):
    * GCC 7.4.0 **or**
    * Clang 6.0

These are specific versions the Vector SilKit is tested and built against.
