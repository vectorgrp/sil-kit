.. include:: /substitutions.rst
.. include:: ./demo-abstracts.rst

==========================
|ProductName| API features
==========================

.. _sec:simple-can-demo:

Simple Can Demo
~~~~~~~~~~~~~~~

Abstract
    |DemoAbstractSimpleCan|
Executables
    * ``SilKitDemoCanReader``
    * ``SilKitDemoCanWriter``
Sources 
    ``./SilKit-Demos/Can``
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
    * SIL Kit Network Simulator (optional)
Parameters
    * ``--network <name>``
      Name of the Can network to use. 
      Defaults to "CAN1".
    * ``--hex``
      Print the Can payloads in hexadecimal format.
      Otherwise, the payloads are interpreted as strings.
System Example
    Run the following commands in separate terminals:

    .. parsed-literal::

        # Registry (if not already running):
        |Registry|
            
        # Monitor (optional):
        |Monitor|

        # Can Reader:
        |DemoDir|/SilKitDemoCanReader

        # Can Writer:
        |DemoDir|/SilKitDemoCanWriter

        # System Controller:
        |SystemController| CanReader CanWriter 

    To run the demo without virtual time synchronization and start coordination, use the following commands in separate terminals:

    .. parsed-literal:: 

        # Registry (if not already running):
        |Registry|

        # Can Reader:
        |DemoDir|/SilKitDemoCanReader --async --autonomous

        # Can Writer:
        |DemoDir|/SilKitDemoCanWriter --async --autonomous



.. _sec:autonomous-lifecycle-demo:

Autonomous lifecycle Demo
~~~~~~~~~~~~~~~~~~~~~~~~~

TODO

.. _sec:coordinated-lifecycle-demo:

Coordinated lifecycle Demo
~~~~~~~~~~~~~~~~~~~~~~~~~~

TODO

.. _sec:event-based-demo:

Event based participant Demo
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TODO


.. _sec:timesync-demo:

Virtual Time Synchronization Demo
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TODO

.. _sec:integration-demo:

Asynchronous Simulation Step Demo
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TODO

.. _sec:netsim-demo:
         
Network Simulator Demo
~~~~~~~~~~~~~~~~~~~~~~

Abstract
    |DemoAbstractNetSim|
Source location
    ``./SilKit-Demos/NetworkSimulator``
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>`
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
Parameters
    * ``<ParticipantConfiguration.yaml>`` 
      File name of the participant configuration to be used; 
      use ``DemoNetSim.silkit.yaml`` for an example configuration.
    * ``<ParticipantName>`` 
      The name of the participant within the simulation.
    * ``[RegistryUri]`` 
      The silkit:// URI of the registry to connect to; defaults to silkit://localhost:8500 (optional).
Parameter Example
    .. parsed-literal:: 
       
       # Start the Network Simulator Demo with the given configuration file and participant name
       |DemoDir|/SilKitDemoNetSim ./SilKit-Demos/NetworkSimulator/DemoNetSim.silkit.silkit.yaml NetworkSimulator
System Example
    Interplay with the Can Demo:

    .. parsed-literal:: 
    
        # Registry (if not already running):
        |Registry|
        
        # Monitor (optional):
        |Monitor|
    
        # Can Reader:
        |DemoDir|/SilKitDemoCan ./SilKit-Demos/Can/DemoCan.silkit.yaml CanReader
    
        # Can Writer:
        |DemoDir|/SilKitDemoCan ./SilKit-Demos/Can/DemoCan.silkit.yaml CanWriter
    
        # System Controller:
        |SystemController| CanReader CanWriter NetworkSimulator
    
        # Network Simulator Demo:
        |DemoDir|/SilKitDemoNetSim ./SilKit-Demos/NetworkSimulator/DemoNetSim.silkit.silkit.yaml NetworkSimulator
    
Notes
    * The Can Reader and Writer configure their controller on the network "CAN1", which is simulated by the network simulator demo.
    * In the simple bus logic of the network simulation demo (see ``Demos\NetworkSimulator\src\Can\MySimulatedCanController.cpp``), the acknowledgment (CanFrameTransmitEvent) is sent directly to the Can Writer. 
      The frame itself (CanFrameEvent) is sent with a delay of 2ms.

