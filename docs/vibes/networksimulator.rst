========================
VIBE Network Simulator
========================

.. _chap:VIBE-NetSim:

Overview
------------------

The VIBE Network Simulator supports simulation of CAN, LIN, Ethernet,
and FlexRay networks. Compared to the simple, functional simulation,
the network simulator adds the following details:

CAN / CAN FD
~~~~~~~~~~~~~~~~~~~

Delay Model Features:

* Priority based arbitration of frames according to CAN identifiers.
* Delayed transmission due to a busy bus.
* Transmission duration according to configured baud rate and frame length including stuff bits.

Transmission Acknowledgement Features:

* Models transmission errors due to a missing ACK when there is only one active CAN controller.
  This allows reproducing effects that arise when starting the communication, e.g.,
  a CAN controller entering error passive state.

Ethernet
~~~~~~~~~~~~~~~~~~~

The VIBE Network Simulator can simulate bus topologies and switched topologies.
It adds the following features to an Ethernet connection:

* Transmission delays according to full-duplex operation.
* Priority ordered transmission queues according to the priority values encoded in the VLAN tag.
* Delayed connection establishment after controller activation.

Ethernet switches are modelled according to the store-and-forward technique and provide the
following features:

* Ethernet connections between ports or controllers implement the above mentioned features of ethernet connections.
* Port based filtering according to MAC addresses and VLAN IDs (dynamically learned ports).
* Frame dropping with respect to VLAN tags, i.e., frames with drop eligible indicator and
  lower priority will be dropped first in case of congestion.

FlexRay
~~~~~~~~~~~~~~~~~~~

The FlexRay simulation models the synchronous transmission of frames in a FlexRay cluster.

* Entire synchronous transmission cycle is modelled.
* Detailed simulation of the startup phase from the point of powering controllers on until
  a stable synchronous cycle has been established.
* Erroneous shutdown due to a loss of synchronicity can be modeled.
* Supports periodic transmission according to repetition configuration of TX buffers.
* Supports sending and receiving Wake-Up Symbols.
* Requires valid FlexRay cluster and node parameters.

LIN
~~~~~~~~~~~~~~~~~~~

The LIN simulation has the following features:

* Simulated data transmission according to the LIN master/slave concept where all transmissions
  are initiated by the master.
* Calculation of the transmission delay according to payload length and configured baud rate.
* Allows reproducing communication errors that arise at a master due to missing slave responses.


Command Line Interface
----------------------

.. code-block:: 

    Usage: VIBE-NetworkSimulator.exe [--version] [--help] [--name <participantName>] [--domain <domainId>] [--log <level>] [--participant-configuration <configuration>] <configuration>
    Arguments:
    -v, --version: Get version info.
    -h, --help: Get this help.
    -n, --name <participantName>: The participant name used to take part in the simulation. Defaults to 'NetworkSimulator'.
    -d, --domain <domainId>: The domain ID which is used by the Integration Bus. Defaults to 42.
    -l, --log <level>: Log to stdout with level 'trace', 'debug', 'warn', 'info', 'error', 'critical' or 'off'. Defaults to 'info'.
    -c, --participant-configuration <configuration>: Path and filename of the Participant configuration YAML or JSON file. Note that the format was changed in v3.6.11.
    <configuration>: Path and filename of the Network Simulator configuration YAML or JSON file. Note that the format was changed in v3.6.11.


.. _sec:networksimulator-configuration:

Configuration
------------------

The VIBE Network Simulator can be configured via two YAML/JSON config files: the 
:doc:`participant configuration <../configuration/configuration>`, as used for every simulation participant, and the 
network simulator configuration as described in the following. The network simulator configuration tells the 
network simulator which networks to  simulate and with which properties.

The outline of a network simulation configuration file (in YAML format) is as follows:

.. code-block:: yaml
                
    ---
    "$schema": "./ParticipantConfiguration.schema.json"
    SchemaVersion: 1
    Description: Sample configuration for VIBE Network Simulator
    SimulatedNetworks:
    - Name: CAN1
      Type: CAN
    - Name: Ethernet1
      Type: Ethernet
    - Name: Ethernet2
      Type: Ethernet
    - Name: FlexRay1
      Type: FlexRay
    - Name: LIN1
      Type: LIN
    Switches:
    - Name: Switch1
      Ports:
      - Name: Port1
        VlanIds:
        - 1
        Network: ETH1_Link1
      - Name: Port2
        VlanIds:
        - 1
        Network: ETH1_Link2



Configuration Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 15 85
   :header-rows: 1

   * - Setting Name
     - Description

   * - $schema
     - The location of the network simulator configuration schema file. The NetworkSimulatorConfiguration.schema.json is
       part of the VIBE Netsim delivery.
       
   * - SchemaVersion
     - The version of the schema of this config file. Current Version number is 1.

   * - Description
     - A verbatim description of the configuration intended to help a test engineer identifying a particular 
       configuration. (optional)

   * - SimulatedNetworks
     - This section describes which networks should be simulated by the network simulator

   * - Switches
     - This section can be used to configure a switched network topology for Ethernet networks. It contains a list of 
       switches.

Simulated Networks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 15 85
   :header-rows: 1

   * - Setting Name
     - Description

   * - Name
     - The name of the network as referenced by the controllers of participants.
       
   * - Type
     - The type of the simulated network. Can either be "CAN", "Ethernet", "FlexRay" or "LIN".

Switches
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table:: Switch Configuration
   :widths: 15 85
   :header-rows: 1

   * - Setting Name
     - Description

   * - Name
     - The name of the Ethernet switch.
       
   * - Ports
     - A list of the ports of the switch.

.. list-table:: Port Configuration
   :widths: 15 85
   :header-rows: 1

   * - Setting Name
     - Description

   * - Name
     - The name of the Ethernet switch port.
       
   * - Network
     - The network to which this port is connected.
     
   * - VlanIds
     - A list of the VLAN IDs of this port.
