==============================
User Guide
==============================

..
  macros for internal use
..
  General macros
.. |ProductName| replace:: SIL Kit
..

The following section explains how to bring together Vector |ProductName| enabled applications and how to successfully run a simulation.

.. contents::
   :local:
   :depth: 2


.. _label:users-setup-simple-example:
.. figure:: ../_static/simSetup_simple.svg
   :alt: : Simple simulation setup with two participants communicating on a network named "CAN1".
   :align: center
   :width: 800

   : Example of a simple simulation setup with two participants that communicate on a bus network named "CAN1".


.. _sec:users-setup:

Preparations
------------

If you do not have a |ProductName| package at hand, please do the following:

1. Visit our Releases page on GitHub (`https://github.com/vectorgrp/sil-kit/releases <https://github.com/vectorgrp/sil-kit/releases>`_).
2. Find your desired |ProductName| version.
   We recommend the most recent version, as it may have new features and bug fixes.
3. Download the zip file for your platform.
   If there are no prebuilt binaries for your platform, then you must build from source.
   For more information, please see `Build Instructions <https://github.com/vectorgrp/sil-kit#getting-started---git-clone>`_.
4. Unzip the archive.
5. |ProductName| might be installed on your system.
   In that case a registry service may already be running on your system.
   Otherwise, if you want to run the simulation locally, start the registry service (located at `SilKit/bin/sil-kit-registry`).


Quick Start
-----------

.. admonition:: Note

  This guide assumes you want to run the simulation locally.
  If you have a more complex scenario, take a look at :ref:`The Participant Configuration File<sec:participant-config>` first.

In general a SIL Kit simulation consists out of a central sil-kit-registry process and one or multiple simulation participants/SIL Kit enabled applications.
The registry's purpose is to provide SIL Kit participants with information of which participants are present and how to connect to them. 
Besides this central book keeping, a SIL Kit registry does not participate in the SIL Kit simulation itself.
To join the simulation SIL Kit participants/SIL Kit enabled applications must only be able to connect to this SIL Kit registry.

The following steps are required to set up a simulation and run it:

1. Make sure a SIL Kit registry is running.
   If no sil-kit-registry process is running, you can start one by simply starting the executable.
2. Start the |ProductName| enabled application(s) with the desired arguments.
   Participants without a lifecycle and autonomous participants will start directly.
   For example, the applications to start in :numref:`Figure %s <label:users-setup-simple-example>` are both `SilKitDemoCan`` with their respective participant name `CanWriter` and `CanReader` as arguments.
   See :doc:`demo applications<../usage/demos>` for informatio of how to build and start the sample applications.
   If you have access to the registry's log, you should see log entries stating that the participant(s) connected.
3. Depending of the needs of your SIL Kit simulation, it may be necessary to start the :ref:`System Controller Utility<sec:util-system-controller>` (see :ref:`Configuring the Lifecycle Service<subsec:sim-configuring-lifecycle>` for more information).
   Once all required simulation participants have joined the simulation, the coordinated participants within the simulation will start.
4. Coordinated participants can be stopped externally by the system controller utility; autonomous participants must stop themselves (see :ref:`Terminology<sec:overview-terminology>`).

.. admonition:: Note

  If you encounter any problems during the initial setup (e.g., you started the registry and the participants but nothing happens), refer to :doc:`Troubleshooting<../usage/troubleshooting>`.

.. _sec:users-participants:

Participants
------------

A participant in |ProductName| is a node in a simulation; Depending on the application, it may send or receive messages from other participants in the simulation.
Each participant must have a unique name to be able to join the simulation.
After a participant connects to a |ProductName| Registry, peer-to-peer connections are established with all other participants.

Participants have their own set of |ProductName| services to communicate with other participants.
For example, CAN controllers send and receive CAN frames, which comprise an ID, flags, and the actual payload.
Refer to :ref:`Supported Services<sec:overview-supported-services>` for each service's capabilities.

Participants can be configured to coordinate their simulation start and stop behavior with other participants through use of the :ref:`Lifecycle Service<sec:sim-lifecycle-syncParticipants>`.
To synchronize their virtual time with others, a participant can use the :ref:`Time Synchronization Service<subsubsec:sim-lifecycle-timeSyncService>`.

Users can (re-)configure parts of the |ProductName| participants if the application allows the user to pass in a |ProductName| participant configuration file.
For example, the user may want to configure logging to the standard output or to a file.
See :ref:`(Re-)Configuration<sec:users-config>` for more information.

.. _sec:users-utilities:

Utility Processes
-----------------

The :ref:`Registry<sec:util-registry>` is a utility which enables discovery between |ProductName| participants.
Thus, it is mandatory and must be started prior to the creation of any participants.
Once the registry is created, it listens for participants on a specified URI.
Once a participant is created, if configured properly, it sends a message to the registry URI asking for the list of active participants.
The registry, once receiving the message, sends the requested information back to the new participant.
The new participant, once receiving the message from the registry, attempts to establish a peer-to-peer connection with all participants listed in the message.

The :ref:`System Controller<sec:util-system-controller>` is an application which orchestrates participants in a |ProductName| simulation by specifying a list of required participant names at start up.
A simulation's coordinated participants are dependent on the state of all required participants.
The coordinated participants won't start if any required participant is still missing.
In contrast, autonomous participants do not consider the state of other participants.

The :ref:`Monitor<sec:util-monitor>` utility tracks and displays participant state within a simulation.
It's an optional utility which can be started and stopped at any time.

.. _sec:users-network-example:

Network Topology / Examples
---------------------------

|ProductName| supports TCP/IP and Unix Domain Socket network types.
When joining a simulation, new participants will first open a publically visible listening socket.
Then, it connects to the |ProductName| Registry and tells it its connection information (especially the port number of the just opened listening socket).
If the registry was not configured before it was started, it will open a listening port at localhost:8500.

Once the connection to the registry is established, the registry replies with the connection information of all already connected participants.
The new participant will try to connect to each of the other participants directly.
By default, it will first connect via Unix Domain Sockets and if this does not work, it will try to connect via TCP.
In case both connection attempts fail, the registry will act as a proxy between the participants.

In the most simple case, the registry as well as all participants are running on your local machine (shown in :numref:`Figure %s <label:users-config-setup-local>`).

.. _label:users-config-setup-local:
.. figure:: ../_static/simSetup_local.svg
   :alt: : Setup in which registry and all participants run on a local host with their default configuration.
   :align: center
   :width: 800

   : Setup in which registry and all participants run on a local host with their default configuration.


By default, connections to the simulation are established via TCP on the 'localhost' on port 8500.
:numref:`Figure %s <label:users-config-setup-remote-autonomous>` displays a simulation where the registry runs on a different machine and the participants are reconfigured to connect to it.

.. _label:users-config-setup-remote-autonomous:
.. figure:: ../_static/simSetup_remote_autonomous.svg
   :alt: : A simulation setup in which the registry is running on a remote machine.
   :align: center
   :width: 800

   : A simulation setup in which the registry is running on a remote machine.


A special case are setups that involve containers, as it is common in docker or cloud setups.
As mentioned before, SIL Kit participants open a socket on a (random) port that other participants can connect with.
If you have a setup that involves containers you may need more control regarding the used ports.
For that reason, participants may explicitly configure the listening port (or ports if you have the choice) that the participant opens its socket on.
:numref:`Figure %s <label:users-config-setup-remote-docker>` shows a container setup in which the registry and a participant run inside a docker runtime but on separate containers and one more participant connects from outside of the runtime.

.. _label:users-config-setup-remote-docker:
.. figure:: ../_static/simSetup_remote_docker.svg
   :alt: : Docker setup with a SIL Kit Registry and a participant running in separate containers and another participant connecting from outside the runtime. The runtime is running on the local host.
   :align: center
   :width: 1200

   : Docker setup with a SIL Kit Registry and a participant running in separate containers and another participant connecting from outside the runtime. The runtime is running on the local host.

|

..
  TODO add example for coordinated setup?

.. _sec:users-config:

(Re-)Configuration
------------------

Sometimes, it may be necessary to change the |ProductName| Registry's configuration or the configuration of individual |ProductName| enabled application(s) that are only available as executables in binary form.
The configuration can be changed through the use of a YAML configuration file.
It is passed to a |ProductName| enabled application or Registry and overrides its initial configuration.


.. admonition:: Note

  If you do not want to change the default behavior or your SIL Kit application, it should be unnecessary to provide a configuration file.

Some of the most common :ref:`options<sec:sil-kit-config-yaml>` configured by users include the participant name, logging, bus network specifics, registry location, and health monitoring.
For example, a user may need to modify an already existing participant to use a different name and change the network name that one of its bus services connects to (see :numref:`Figure %s <label:users-config-config-original>`).
To address this, the user can provide a configuration file, which contains the needed changes (see :numref:`Figure %s <label:users-config-config-reconfigured>`).

.. _label:users-config-config-original:
.. figure:: ../_static/configuration_original.svg
   :alt: : Faulty simulation setup with two participants that have the same name and a mismatch in their bus network names.
   :align: center
   :width: 800

   : Faulty simulation setup with two participants that have the same name and a mismatch in their bus network names.

|

.. _label:users-config-config-reconfigured:
.. figure:: ../_static/configuration_reconfigured.svg
   :alt: : Setup from :numref:`Figure %s <label:users-config-config-original>` in which the left participant was reconfigured via a configuration file.
   :align: center
   :width: 800

   : Setup from :numref:`Figure %s <label:users-config-config-original>` in which the left participant was reconfigured via a configuration file.

|

See :ref:`The Participant Configuration File<sec:participant-config>` for more information.

