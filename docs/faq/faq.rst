==========================
Frequently Asked Questions
==========================

.. contents::
   :depth: 3
   
This section provides frequently asked questions concerning SIL Kit and the corresponding answers.

Data Generation & Transmission
==============================

Is there a maximum payload size for Ethernet frames?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The size of the payload of Ethernet frames is not limited by SIL Kit.
Nonetheless, the maximum payload size might be constrained by the system resources available on the system on which SIL Kit is executed.

Is there a maximum payload size for Publish/Subscribe or RPC data?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The size of the data payload of the Publish Subscribe and RPC service are not limited by SIL Kit.
Nonetheless, the maximum payload size might be constrained by the system resources available on the system on which SIL Kit is executed.

Can I use a DBC file to generate CAN frames?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Within SIL Kit, there is no included support for DBC files.
You can use SIL Kit enabled tools that have support for DBC files, or you will need to implement DBC support within your implementation yourself.

Setup
=====

Is it possible to use statically configured ports for SIL Kit peer-to-peer communication between participants?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Since version 4.0.16 users can specify which port the participant opens for communication between participants by using the ``AcceptorUris`` field within the :ref:`Middeware section of the participant configuration<sec:mwcfg>`.
By doing so, a firewall can be configured to allow these static ports for communication.
The port used by the SIL Kit registry can be specified through its :ref:`CLI<sec:util-registry>`.

What is the sil-kit-registry?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The SIL Kit registry is a central utility that is necessary for running a SIL Kit simulation.
At the start, each participant connects to the registry and retrieves the connection information to all other participants.
In this sense, the registry acts as a phone book that tells the user the contact information to all other participants of a SIL Kit simulation.
The registry does not partake in the simulation itself.
It especially does not route messages between participants.
Please note that it is mandatory that all SIL Kit simulation participants can reach the SIL Kit registry for a proper simulation setup.

Can I use Vector SIL Kit together with real hardware/in a Hardware-in-the-Loop setup?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Vector SIL Kit was designed deliberately with Software-in-the-Loop environments in mind.
Many aspects of it are designed for SIL settings (especially the virtual time synchronization, that adapts its speed to the execution speed of the participants) and not HIL settings.
These aspects might imply that SIL Kit does not fit the needs of your hardware setup.
Furthermore, SIL Kit itself does not provide capabilities to connect it to a hardware setup.

Nonetheless, some SIL Kit enabled tools may allow you to bridge a SIL Kit simulation and a HIL setup.

How can I interoperate a SIL Kit simulation that is distributed between Windows and Linux clients?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For SIL Kit it does not matter whether all simulation participants are running on the same kind of operating system.
All SIL Kit simulation participants must be able to connect to the SIL Kit registry, and they must be able to establish a connection to all other simulation participants.
In such a distributed setting, the SIL Kit registry must be started such that it is reachable through the network by other hosts.

You might do so by running the command below:

.. code-block:: console
   
   ./sil-kit-registry.exe --listen-uri silkit://0.0.0.0:8501

.. admonition:: Note

   This example uses a different port, as a tool may have already opened a registry on the default port 8500.

All simulation participants must now specify a URI so that they connect to this registry (e.g., ``silkit://<IP of node running registry>:8501``).


Other
=====

How is the performance/latency/throughput of the SIL Kit?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The specific performance (latency, throughput, animation factor, etc.) of your SIL Kit simulation depends on a set of parameters that highly depend on your use case.
The number of simulation participants, whether virtual time synchronization is used, the size of simulation steps, the size and frequency of messages being exchanged, and many other aspects highly influence the performance for you use case.
Finally, the hardware being used, the network infrastructure and the distribution of simulation participants over different hosts affect the performance as well.

To get a first impression of the performance that can be expected of the SIL Kit in your use case, refer to the :ref:`Benchmark Demo<sec:util-benchmark-demo>` where you can specify a set of relevant parameters and get real performance measurements for your setup.

Can I control the order in which simulation participants execute their simulation steps, in case they occur at the same time?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

No, the order simulation step execution of synchronized participants can not be controlled.

How can I configure a SIL Kit participant to write a trace log file?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A trace log file can be used in debugging scenarios to get a detailed insight into the execution of a SIL Kit participant.

The :ref:`participant configuration<sec:sil-kit-config-yaml>` is a YAML file which enables you to modify some aspects of the runtime behavior of SIL Kit simulation participants.
In particular, you can also use it to let a participant generate a trace log file.

For example, by adding the following configuration:

.. code-block:: console
   
   Logging:
     Sinks:
     - Type: File
       Level: Trace
       LogName: TraceLogFile

.. admonition:: Note

   If your participant configuration already contains a logging section, you must only add the sink.

This will tell SIL Kit to write all log messages of Level trace and above to a file in the process working directory of the SIL Kit participant.
The file name will be ``TraceLogFile`` with a time-specific suffix.
The ``LogName`` field also supports an absolute path, e.g., ``LogName: C:\Temp\SimulationLog``.
