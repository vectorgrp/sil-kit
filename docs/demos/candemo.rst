Can Demo
========

The CAN demo shows a simple use case with a CAN simulation. In the "IbConfig_DemoCan.json" - configuration four participants are specified:

* The "CanWriter" will continously send a CAN message over the link "CAN1"
* The "CanReader" is receiving the CAN messages sent over the link "CAN1"
* The "SystemController" handles the different SystemStates of the simulation
* The "SystemMonitor" is monitoring everything that happens in the simulation


Setup & Usage
-------------

The CAN Demo requires the IntegrationBus.dll for launching. Furthermore, its configuration depends on the described four participants,
which run in different processes. The two CAN participants are started with the IbCanDemo.exe / IbCanDemo.so, which needs two arguments
and one optional argument:

#. The \*.json config
#. The ParticipantName (CanWriter / CanReader)
#. (optional) The domainId

There are two suitable ways to start the CAN demo and all of the four participants:

1. The IbLauncher
~~~~~~~~~~~~~~~~~

The IbLauncher requires: `Python <https://www.python.org/downloads/>`_ v2.7+ / v3.x+

You can start the IbLauncher with the "IbLauncher.bat" under Windows or the "IbLauncher.sh" under Linux, which also sets the 
PATH environment variable to the bin folder of the IntegrationBus.dll. The IbLauncher-Script needs two arguments:

#. The \*.json config
#. A LaunchConfiguration (specified in the \*.json configs)

An example CAN demo execution with the IbLauncher looks like this::

    IbLauncher.bat ..\..\IntegrationBus-Demos\Can\IbConfig_DemoCan.json -c Installation


2. Commandline Execution
~~~~~~~~~~~~~~~~~~~~~~~~

TBD


Code Explanation
----------------

The CAN acknowledges and CAN messages are received over the following Callbacks:

.. code-block:: cpp

    auto comAdapter = ib::CreateFastRtpsComAdapter(ibConfig, participantName, domainId);
    auto* canController = comAdapter->CreateCanController("CAN1");

    canController->RegisterTransmitStatusHandler(&AckCallback);
    canController->RegisterReceiveMessageHandler(&ReceiveMessage);
