
.. Communication system Demos

.. |DemoAbstractCAN| replace:: 
  The ``CanWriter`` participant sends Can frames to the ``CanReader`` participant including frame acknowledgment handling.
.. |DemoAbstractETH| replace:: 
  The ``EthernetWriter`` participant sends Ethernet frames to the ``EthernetReader`` participant including frame acknowledgment handling.
.. |DemoAbstractLIN| replace:: 
  A two-node Lin Setup with a ``LinMaster`` and a ``LinSlave``. 
  Includes a simple scheduling mechanism and demonstrates controller sleep / wakeup handling.
.. |DemoAbstractFlexRay| replace::
  A two-node FlexRay Setup with a full cluster and node parametrization. Includes POC Status handling, buffer updates and reconfiguration.
  This Demo requires a separate ``Network Simulator`` application to simulate the details of the FlexRay cluster, which is not included in the SIL Kit.
.. |DemoAbstractPubSub| replace:: 
  One participant publishes GPS and temperature data, another participant subscribes to these topics.
  Including (de-)serialization of the C++ structures into a transmittable format.
.. |DemoAbstractRPC| replace:: 
  The Rpc server participant provides two simple functions which are called by a Rpc client participant.
  Includes (de-)serialization of the function parameters.


.. API feature Demos

.. |DemoAbstractSimpleCan| replace:: 
  Minimalistic demo for a participant with Can communication and virtual time synchronization.

.. |DemoAbstractNetSim| replace:: 
  Demonstrates the usage of the experimental SIL Kit NetworkSimulator API.
  A custom network simulation for Can is set up, the network simulator application can be used together with the Can demo.

.. |DemoAbstractAutonomous| replace:: 
  Minimal example of a participant with an autonomous lifecycle.
  The demo shows that this participant can be started and stopped independently from other participants.

.. |DemoAbstractCoordinated| replace:: 
  Minimal example of a participant with a coordinated lifecycle.
  This shows how multiple participants can be started and stopped simultaneously, controlled by the ``sil-kit-system-controller``.

.. |DemoAbstractSimStep| replace:: 
  Minimal example of a participant with time synchronization and a simulation step handler.

.. |DemoAbstractSimStepAsync| replace:: 
  Minimal example of a participant with time synchronization and an asynchronous simulation step handler.


.. Tools

.. |DemoAbstractBenchmark| replace:: 
  This demo sets up a simulation with various command line arguments for benchmarking purposes.
  A configurable amount of participants are spawned by a single process on different threads.
  The demo calculates averaged running times, throughput, speed-up and message rates for performance evaluation.
.. |DemoAbstractLatency| replace:: 
  A sender and a receiver application use the Publish/Subscribe services and measure the round trip time of the communication.
  This setup is useful to evaluate the performance of a SIL Kit setup running on different platforms.
  E.g., between a local host, a virtual machine, a remote network, etc.


