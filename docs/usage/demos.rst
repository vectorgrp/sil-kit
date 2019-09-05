======================
VIB Demos
======================

This document describe the usage of the demo projects that are
included with the Vector Integration Bus project and what their
expected output and or results are. All demo source code is located in
the GIT repository in the folder Demos.

.. admonition:: Note

   All paths on the following pages are relative to the top level of
   the GIT repository. Build artifacts are assumed to be located in a
   'build' subdirectory.


CAN Demo
~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  CAN Reader/Writer with or without Network Simulator VIBE
   *  -  Source location
      -  Demos/Can
   *  -  Requirements
      -  * SystemController
         * SystemMonitor (optional)
         * NetworkSimulator (optional)
   *  -  Parameters
      -  There are up to 3 positional arguments:
         
         #. Filename of the IB Configuration to be used; must be either of the two provided DemoCan configs.
         #. Name of the participant in the configuration; must be either CanWriter or CanReader.
         #. FastRTPS domain ID (optional); defaults to 42.
   *  -  Parameter Example
      -  .. code-block:: powershell
            
            # Creates a CAN Writer Process in the default domain 42:
            build/Demos/Can/IbDemoCan Demos/Can/IbConfig_DemoCan.json CanWriter
   *  -  System Example
      -  .. code-block:: powershell

            # Network Simulator VIBE (assumed to be in PATH, optional):
            NetworkSimulator BusSimulator Demos/Can/IbConfig_DemoCan_NetSim.json

            # System Monitor (optional):
            build/Demos/SystemMonitor/IbDemoPassiveSystemMonitor Demos/Can/IbConfig_DemoCan.json

            # CAN Reader:
            build/Demos/Can/IbDemoCan Demos/Can/IbConfig_DemoCan.json CanReader

            # CAN Writer:
            build/Demos/Can/IbDemoCan Demos/Can/IbConfig_DemoCan.json CanWriter

            # System Controller:
            build/Demos/SystemController/IbDemoSystemController Demos/Can/IbConfig_DemoCan.json


Ethernet Demo
~~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Ethernet Reader / Writer with or without Network Simulator VIBE
   *  -  Source location
      -  Demos/Ethernet
   *  -  Requirements
      -  * SystemController
         * SystemMonitor (optional)
         * NetworkSimulator (optional)
   *  -  Parameters
      -  There are up to 3 positional arguments:
         
         #. Filename of the IB Configuration to be used; must be either of the two provided DemoEthernet configs.
         #. Name of the participant in the configuration; must be either EthernetWriter or EthernetReader.
         #. FastRTPS domain ID (optional); defaults to 42.
   *  -  Parameter Example
      -  .. code-block:: powershell

            # Creates an Ethernet Writer Process in the default domain 42:
            build/Demos/Ethernet/IbDemoEthernet Demos/Ethernet/IbConfig_DemoEthernet.json EthernetWriter
   *  -  System Example
      -  .. code-block:: powershell

            # Network Simulator VIBE (assumed to be in PATH, optional):
            NetworkSimulator BusSimulator Demos/Ethernet/IbConfig_DemoEthernet_NetSim.json

            # System Monitor (optional):
            build/Demos/SystemMonitor/IbDemoPassiveSystemMonitor Demos/Ethernet/IbConfig_DemoEthernet.json

            # Ethernet Reader:
            build/Demos/Ethernet/IbDemoEthernet Demos/Ethernet/IbConfig_DemoEthernet.json EthernetReader

            # Ethernet Writer:
            build/Demos/Ethernet/IbDemoEthernet Demos/Ethernet/IbConfig_DemoEthernet.json EthernetWriter

            # System Controller:
            build/Demos/SystemController/IbDemoSystemController Demos/Ethernet/IbConfig_DemoEthernet.json
   *  -  Notes
      -  | \- The writer sends Ethernet messages at a fixed rate of one message per quantum.
         | \- Both reader and writer sleep for 1 second per quantum to slow down execution.


LIN Demo
~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  LIN Master and Slave demo. The master sends and requests messages from a LIN slave.
   *  -  Source location
      -  Demos/Lin
   *  -  Requirements
      -  * SystemController
         * SystemMonitor (optional)
         * NetworkSimulator (optional)
   *  -  Parameters
      -  There are up to 3 positional arguments:
         
         #. Filename of the IB Configuration to be used; must be either of the two provided DemoLin configs.
         #. Name of the participant in the configuration; must be either LinMaster or LinSlave.
         #. FastRTPS domain ID (optional); defaults to 42.
   *  -  Parameter Example
      -  .. code-block:: powershell

            # Creates a LIN Master Process in the default domain 42:
            build/Demos/Lin/IbDemoLin Demos/Lin/IbConfig_DemoLin.json LinMaster
   *  -  System Example
      -  .. code-block:: powershell

            # Network Simulator VIBE (assumed to be in PATH, optional):
            NetworkSimulator BusSimulator Demos/Lin/IbConfig_DemoLin_NetSim.json

            # System Monitor (optional):
            build/Demos/SystemMonitor/IbDemoPassiveSystemMonitor Demos/Lin/IbConfig_DemoLin.json

            # LIN Master:
            build/Demos/Lin/IbDemoLin Demos/Lin/IbConfig_DemoLin.json LinMaster

            # LIN Slave:
            build/Demos/Lin/IbDemoLin Demos/Lin/IbConfig_DemoLin.json LinSlave

            # System Controller:
            build/Demos/SystemController/IbDemoSystemController Demos/Lin/IbConfig_DemoLin.json
   *  -  Notes
      -  | \- Both Master and Slave sleep for 1 second per quantum to slow down execution.
         | \- The master alternatively sends and requests LIN messages. It sends a message for LIN ID 17 and requests a message for LIN ID 34.
         | \- The slave is configured to trigger a callback on LIN ID 17 and replies with the String "Hello!" on LIN ID 34.


FlexRay Demo
~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  FlexRay Demo for a FlexRay cluster containing two nodes
   *  -  Source location
      -  Demos/FlexRay
   *  -  Requirements
      -  * SystemController
         * SystemMonitor (optional)
         * NetworkSimulator (optional)
   *  -  Parameters
      -  There are up to 3 positional arguments:
         
         #. Filename of the IB Configuration to be used; must be either of the two provided DemoFlexray configs.
         #. Name of the participant in the configuration; must be either Node0 or Node1.
         #. FastRTPS domain ID (optional); defaults to 42.
   *  -  Parameter Example
      -  .. code-block:: powershell

            # Creates a FlexRay Process for Node 0 in the default domain 42:
            build/Demos/FlexRay/IbDemoFlexray Demos/FlexRay/IbConfig_DemoFlexray.json Node0
   *  -  System Example
      -  .. code-block:: powershell

            # Network Simulator VIBE (assumed to be in PATH, optional):
            NetworkSimulator BusSimulator Demos/FlexRay/IbConfig_DemoFlexray_NetSim.json

            # System Monitor (optional):
            build/Demos/SystemMonitor/IbDemoPassiveSystemMonitor Demos/FlexRay/IbConfig_DemoFlexray.json

            # Node 0:
            build/Demos/FlexRay/IbDemoFlexray Demos/FlexRay/IbConfig_DemoFlexray.json Node0

            # Node 1:
            build/Demos/FlexRay/IbDemoFlexray Demos/FlexRay/IbConfig_DemoFlexray.json Node1

            # System Controller:
            build/Demos/SystemController/IbDemoSystemController Demos/FlexRay/IbConfig_DemoFlexray.json
   *  -  Notes
      -  Starting the FlexRay cycle takes quite some time, which is accurately modeled by the NetworkSimulator. 
         It takes somewhat between 50 and 100 ms until the first FlexRay messages are transmitted.


Generic Message Demo
~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Untimed demo to send and receive Generic Messages
   *  -  Source location
      -  Demos/GenericMessage
   *  -  Requirements
      -  * SystemController
         * SystemMonitor (optional)
   *  -  Parameters
      -  There are up to 3 positional arguments:
         
         #. Filename of the IB Configuration to be used; must be the provided IbConfig_DemoGenericMessage.json config. 
         #. Name of the participant in the configuration; must be either Publisher or Subscriber.
         #. FastRTPS domain ID (optional); defaults to 42.
   *  -  Parameter Example
      -  .. code-block:: powershell

            # Creates a generic message process with for the participant Publisher:
            build/Demos/GenericMessage/IbDemoGenericMessage Demos/GenericMessage/IbConfig_DemoGenericMessage.json Publisher
   *  -  System Example
      -  .. code-block:: powershell

            # System Monitor (optional):
            build/Demos/SystemMonitor/IbDemoPassiveSystemMonitor Demos/GenericMessage/IbConfig_DemoGenericMessage.json

            # Generic Message Subscriber:
            build/Demos/GenericMessage/IbDemoGenericMessage Demos/GenericMessage/IbConfig_DemoGenericMessage.json Subscriber

            # Generic Message Publisher:
            build/Demos/GenericMessage/IbDemoGenericMessage Demos/GenericMessage/IbConfig_DemoGenericMessage.json Publisher

            # System Controller:
            build/Demos/SystemController/IbDemoSystemController Demos/GenericMessage/IbConfig_DemoGenericMessage.json
   *  -  Notes
      -  | \- Both Publisher and Subscriber sleep for 1 second per quantum to slow down execution.
         | \- The Publisher sends two topics "GroundTruth" and "VehicleModelOut" to show 
           that multiple generic message topics are created and separated from each other.


IO Port Demo
~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Sender / Receiver demo for IO values. The demo uses Analog IO, Digital IO, PWM IO, and Pattern IO.
   *  -  Source location
      -  Demos/IO
   *  -  Requirements
      -  * SystemController
         * SystemMonitor (optional)
   *  -  Parameters
      -  There are up to 3 positional arguments:
         
         #. Filename of the IB Configuration to be used; must be the provided IbConfig_DemoIo.json config. 
         #. Name of the participant in the configuration; must be either IoWriter or IoReader.
         #. FastRTPS domain ID (optional); defaults to 42.
   *  -  Parameter Example
      -  .. code-block:: powershell

            # Creates an IO-Writer process:
            build/Demos/Io/IbDemoIo Demos/Io/IbConfig_DemoIo.json IoWriter
   *  -  System Example
      -  .. code-block:: powershell

            # System Monitor (optional):
            build/Demos/SystemMonitor/IbDemoPassiveSystemMonitor Demos/Io/IbConfig_DemoIo.json

            # IO Value Writer:
            build/Demos/Io/IbDemoIo Demos/Io/IbConfig_DemoIo.json IoWriter

            # IO Value Reader:
            build/Demos/Io/IbDemoIo Demos/Io/IbConfig_DemoIo.json IoReader

            # System Controller:
            build/Demos/SystemController/IbDemoSystemController Demos/Io/IbConfig_DemoIo.json
   *  -  Notes
      -  | \- Both Writer and Reader sleep for 1 second per quantum to slow down execution.
         | \- In each quantum, all four types of IO are used in each quantum: analog, digital, PWM, and pattern IO.
