Integration Bus (IB) Overview
=============================

.. contents::


Definition and Functionalities
------------------------------

The Integration Bus is a runtime component that enables distributed simulation for automotive
applications. For this, the Integration Bus provides:

* Communication on different abstraction levels including

  * Low level analog digital IO
  * Vehicle networks (CAN / CAN FD, Ethernet, FlexRay, LIN)
  * High level generic messages for arbitrary application specific communication

* Synchronization of simulation time implementing different protocols

  * Fixed tick rate with acknowledgement (Tick/TickDone)
  * Quantum based synchronization with variable quantum durations
  * Event based synchronization according to lower bound event time

* State handling to control and observe execution of the simulated system
* System configuration from a single JSON file to specify the simulated including communication
  and synchronization needs


.. _base-architecture:

Integration Bus Architecture
----------------------------

The Integration Bus implements a layered architecture comprising three layers:

.. figure:: _static/IntegrationBusArchitecture.png
    :align: center


Supported Services
------------------

The currently supported services of the Integration Bus consist of the following categories:

* **IO Ports:** Analog IO, digital IO, Pattern IO and Pulse-width modulation (PWM) IO
* **Vehicle Network Controllers:** CAN / CAN FD, Ethernet, FlexRay and LIN
* **Generic Messages:** without any data type constraints

Vehicle Networks
~~~~~~~~~~~~~~~~

The Integration Bus provides means to simulate CAN / CAN FD, Ethernet, FlexRay, and LIN networks.
All networks can be simulated with two different levels of detail: simple, functional simulation
or high detail, timing accurate simulation. Timing accurate simulation requires the Vector Network
Simulator, which is part of the :ref:`Vector Integration Bus Extensions (VIBEs) <vibe-index>`.

Vehicle Network Controllers
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Applications access vehicle networks using matching controller models. While the
controller interfaces are the same for simple and high detail simulation, the Vector
Network Simulator VIBE requires controllers to be properly configured, initialized, and used
according to the corresponding network. Without the Network Simulator, on the other hand,
applications can directly send messages without initializing the controllers. As a result,
an application that works in the simple use case do not necessarily work when switching to
a high detail simulation using the Network Simulator. However, applications that have been tested
with Network Simulator, will also work without it::

    Note: the CAN, LIN, Ethernet, and FlexRay demos provided with the Integration Bus
    have all been tested both with and without the Vector Network Simulator.

In Section :doc:`Integration Bus Services <services>` it is described how to configure and use
Vehicle Network Controllers and pointed out which differences for simple simulation and
high detail VIBE simulation with the Network Simulator exists. Furthermore, the usage of
the other services (IO, Generic Message, ...) will also be presented in detail.


.. _vibe-index:

Vector Integration Bus Extensions (VIBEs)
-----------------------------------------

VIBEs are extensions developed by Vector for the Integration Bus. Currently, they only contain
the Network Simulator.


Vector Network Simulator
~~~~~~~~~~~~~~~~~~~~~~~~

The Vector Network Simulator supports simulation of CAN, LIN, Ethernet, and FlexRay networks.
Compared to the simple, functional simulation, the network simulator adds the following details:

CAN / CAN FD
^^^^^^^^^^^^

Delay Model Features:

* Priority based arbitration of frames according to CAN identifiers.
* Delayed transmission due to a busy bus.
* Transmission duration according to configured baud rate and frame length including stuff bits.

Transmission Acknowledgement Features:

* Models transmission errors due to a missing ACK when there is only one active CAN controller.
  This allows reproducing effects that arise when starting the communication, e.g.,
  a CAN controller entering error passive state.

Ethernet
^^^^^^^^

Ethernet simulation is based point-to-point connection between two controllers with the
following features:

* Transmission delays according to full-duplex operation.
* Priority ordered transmission queues according to the priority values encoded in the VLAN tag.
* Delayed connection establishment after controller activation.

Ethernet switches are modeled according to the store-and-forward and provide the
following features:

* Switch ports act as controllers to implement point-to-point as mentioned above.
* Port based filtering according to MAC addresses and VLAN IDs.
* Frame dropping with respect to VLAN tags, i.e., frames with drop eligible indicator and
  lower priority will be dropped first in case of congestion.

FlexRay
^^^^^^^

The FlexRay simulation models the synchronous transmission of frames in a FlexRay cluster.

* Entire synchronous transmission cycle is modelled.
* Detailed simulation of the startup phase from the point of powering controllers on until
  a stable synchronous cycle has been established.
* Erroneous shutdown due to a loss of synchronicity can be modeled.
* Supports periodic transmission according to repetition configuration of TX buffers.
* Supports sending and receiving Wake-Up Symbols.
* Requires valid FlexRay cluster and node parameters.

LIN
^^^

The LIN simulation has the following features:

* Simulated data transmission according to the LIN master/slave concept where all transmissions
  are initiated by the master.
* Calculation of the transmission delay according to payload length and configured baud rate.
* Allows reproducing communication errors that arise at a master due to missing slave responses.


IB Prerequisites for Usage
--------------------------

* For Windows:
    * Visual Studio 2015 Toolset v140 and higher (also tested with MSVC++ 14.12)
* For Ubuntu Xenial (16.04 LTS):
    * GCC 5.4.0-6ubuntu1~16.04.10 **or**
    * Clang 3.8.0-2ubuntu4

These are the specific versions Integration Bus is tested and built against.