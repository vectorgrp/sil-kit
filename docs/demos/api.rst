.. include:: /substitutions.rst
.. include:: ./abstracts.rst

===
API
===

These demos focus on basic systems or single topics of the |ProductName| API.

.. _sec:simple-can-demo:

Simple Can
~~~~~~~~~~

Abstract
    |DemoAbstractSimpleCan|
Executables
    * ``SilKitDemoSimpleCan``
Sources
    * :repo-link:`SimpleCan.cpp <Demos/api/SimpleCan/SimpleCan.cpp>`
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>`
Parameters
    * ``<ParticipantName>``
      Name of the SIL Kit participant. 
System Example
    Run the following commands in separate terminals:


.. _sec:netsim-demo:

Network Simulator API
~~~~~~~~~~~~~~~~~~~~~

Abstract
    |DemoAbstractNetSim|
Sources
    * :repo-link:`NetSimDemo.cpp <Demos/api/NetworkSimulator/NetSimDemo.cpp>`
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>`
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
        |DemoDir|/SilKitDemoCanReader
    
        # Can Writer:
        |DemoDir|/SilKitDemoCanWriter
    
        # System Controller:
        |SystemController| CanReader CanWriter NetworkSimulator
    
        # Network Simulator Demo:
        |DemoDir|/SilKitDemoNetSim ./SilKit-Demos/NetworkSimulator/DemoNetSim.silkit.silkit.yaml NetworkSimulator
    
Notes
    * The Can Reader and Writer configure their controller on the network "CAN1", which is simulated by the network simulator demo.
    * In the simple bus logic of the network simulation demo (see ``Demos\NetworkSimulator\src\Can\MySimulatedCanController.cpp``), the acknowledgment (CanFrameTransmitEvent) is sent directly to the Can Writer. 
      The frame itself (CanFrameEvent) is sent with a delay of 2ms.

    .. parsed-literal::

        # Registry (if not already running):
        |Registry|

        # Can Reader:
        |DemoDir|/SilKitDemoSimpleCan CanReader

        # Can Writer:
        |DemoDir|/SilKitDemoSimpleCan CanWriter

        # System Controller:
        |SystemController| CanReader CanWriter 

..
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


      
