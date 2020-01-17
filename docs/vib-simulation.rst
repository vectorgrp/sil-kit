VIB Simulation 
================================
.. macros for internal use
.. |ComAdapter| replace:: :doc:`ComAdapter<api/comadapter>`
.. |Middleware| replace:: :doc:`Middleware<configuration/middleware-configuration>`
.. |Participant| replace:: :doc:`Participant<api/participantcontroller>`
.. |System| replace:: :doc:`System<api/synchronisation>`

.. contents::
    :local:
    :depth: 2

This document discusses the distributed algorithm of the integration bus
and the simulation time synchronization.

Distributed State Machine
-------------------------
The IntegrationBus simulation relies on a distributed state machine.
The distributed algorithm takes each individual participant's state into 
account and allows for computing a global system state and controlling
the simulation in its entirety.

The underlying |ComAdapter| and |Middleware| implement the low-level 
communication required for the exchange and synchronization of 
state updates.
On top of this, a user accesible API is implemented which allows
introspection of |Participant| states and |System| states, and sending commands to
transition into new states.


The participant lifecycle
~~~~~~~~~~~~~~~~~~~~~~~~~~~
discuss the participants controllers life cycle and how the 
states are transitioned, e.g. by external commands from a system
controller.

.. figure:: _static/ParticipantLifeCycle_1.png
   :alt: The participant controller life cycle
   :align: center
   :width: 80%
   
   The Participant Controller Life Cycle.

The system life 
~~~~~~~~~~~~~~~~~~
To understand how each participants state affects the system state
it is helpful to examine the system states

.. figure:: _static/SystemMonitor_1.png
   :alt: The SystemMonitor
   :align: center
   :width: 80%
   
   The SystemMonitor state machine
    

Time Synchronization
--------------------
Discuss how simulation time is synchronized when the system is in state ``running``
