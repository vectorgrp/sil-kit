.. include:: /substitutions.rst
.. include:: ./abstracts.rst

=======================
Communication Protocols
=======================

The demos shown here are build on top of a base implementation that provides general |ProductName| features.
This allows to separate the demo specific use case (e.g. how to use Ethernet with the |ProductName|) from general features (e.g. setup the |ProductName| lifecycle).

.. contents::
    :depth: 1
    :local:

Command line arguments
~~~~~~~~~~~~~~~~~~~~~~

The following arguments are available for all demos described in this chapter:

.. code-block:: console

    -h, --help                   | Get this help.
    -n, --name <name>            | The participant name used to take part in the simulation.
                                   Defaults to '<set by the individual demo>'.
    -u, --registry-uri <uri>     | The registry URI to connect to.
                                   Defaults to 'silkit://localhost:8500'.
    -l, --log <level>            | Log to stdout with level:
                                   'trace', 'debug', 'warn', 'info', 'error', 'critical' or 'off'.
                                   Defaults to 'info'.
                                   Cannot be used together with '--config'.
    -c, --config <filePath>      | Path to the Participant configuration YAML or JSON file.
                                   Cannot be used together with '--log'.
                                   Will always run as fast as possible.
    -a, --async                  | Run without time synchronization mode.
                                   Cannot be used together with '--sim-step-duration'.
    -A, --autonomous             | Start the simulation autonomously.
                                   Without this flag, a coordinated start is performed
                                   which requires the SIL Kit System Controller.
    -d, --sim-step-duration <us> | The duration of a simulation step in microseconds.
                                   Defaults to <set by the individual demo>us.
                                   Cannot be used together with '--async'.
    -f, --fast                   | Run the simulation as fast as possible.
                                   By default, the execution is slowed down to two work cycles per second.
                                   Cannot be used together with '--config'.
    -s, --sleep <ms>             | The sleep duration per work cycle in milliseconds.
                                   Default is no sleeping.
                                   Using this options overrides the default execution slow down.
                                   Cannot be used together with '--fast'.

The default behavior of these options is:

* Participant name is set by the individual demo executable (e.g. ``CanWriter`` for ``SilKitDemoCanWriter``).
* Default registry-uri ``silkit://localhost:8500``.
* Logging to Stdout with Level Info.
* No participant configuration file in use.
* Virtual time synchronization enabled.
* Coordinated start (requires a ``sil-kit-system-controller``).
* Simulation step duration set by the individual demo executable.
* Slowed down execution to two work cycles per second.

Useful execution modes that are made accessible by the general options:

* Spawn multiple demo participants by using collision free participant names, e.g. add a second Can reader with  ``--name CanReader1``.
* Run without time synchronization and start coordination: ``--async --autonomous``, or short ``-aA``.
  This allows to start/stop the participants individually without requiring a ``sil-kit-system-controller``.
* Join a already running simulation with time synchronization: ``--autonomous`` (without ``--async``).
* Perform a coordinated start without time synchronization: ``--async`` (without ``--autonomous``).
  This requires a ``sil-kit-system-controller``.

Some demos extend these options by the following command line arguments:

* For the bus demos (Can, Ethernet, Lin, Flexray): ``--network`` to override the default bus network name.
* For Can and Ethernet: ``--hex`` to print payloads in hexadecimal format.


.. admonition:: Note

   In the following, the system examples are meant to be executed in the repository build of the |ProductName|.
   
   Further, the command line instructions given here are for Linux builds (forward slashes for paths, no filename suffix for executables).
   When using Windows (e.g. Powershell), these have to be replaced with backward slashed and the executable suffix.
   E.g., instead of ``./SilKitDemoCanReader``, use ``.\SilKitDemoCanReader.exe``


.. _sec:can-demo:

Can
~~~

Abstract
    |DemoAbstractCAN|
Executables
    * ``SilKitDemoCanReader``
    * ``SilKitDemoCanWriter``
Sources
    * :repo-link:`CanReaderDemo.cpp <Demos/communication/Can/CanReaderDemo.cpp>`
    * :repo-link:`CanWriterDemo.cpp <Demos/communication/Can/CanWriterDemo.cpp>`
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
    * SIL Kit Network Simulator (optional)
Parameters
    * ``--network <name>``
      Name of the Can network to use. 
      Defaults to 'CAN1'.
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

.. _sec:eth-demo:

Ethernet
~~~~~~~~

Abstract
    |DemoAbstractETH|
Executables
    * ``SilKitDemoEthernetReader``
    * ``SilKitDemoEthernetWriter``
Sources
    * :repo-link:`EthernetReaderDemo.cpp <Demos/communication/Ethernet/EthernetReaderDemo.cpp>`
    * :repo-link:`EthernetWriterDemo.cpp <Demos/communication/Ethernet/EthernetWriterDemo.cpp>`
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
    * SIL Kit Network Simulator (optional)
Parameters
    * ``--network <name>``
      Name of the Ethernet network to use. 
      Defaults to 'Eth1'.
    * ``--hex``
      Print the Ethernet payloads in hexadecimal format.
      Otherwise, the payloads are interpreted as strings.
System Example
    Run the following commands in separate terminals:

    .. parsed-literal:: 

        # Registry (if not already running):
        |Registry|

        # Ethernet Reader:
        |DemoDir|/SilKitDemoEthernetReader

        # Ethernet Writer:
        |DemoDir|/SilKitDemoEthernetWriter

        # System Controller:
        |SystemController| EthernetReader EthernetWriter

    To run the demo without virtual time synchronization and start coordination, use the following commands in separate terminals:

    .. parsed-literal:: 

        # Registry (if not already running):
        |Registry|

        # Ethernet Reader:
        |DemoDir|/SilKitDemoEthernetReader --async --autonomous

        # Ethernet Writer:
        |DemoDir|/SilKitDemoEthernetWriter --async --autonomous

.. _sec:lin-demo:

Lin
~~~

Abstract
    |DemoAbstractLIN|
Executables
    * ``SilKitDemoLinMaster``
    * ``SilKitDemoLinSlave``
Sources
    * :repo-link:`LinMasterDemo.cpp <Demos/communication/Ethernet/LinMasterDemo.cpp>`
    * :repo-link:`LinSlaveDemo.cpp <Demos/communication/Ethernet/LinSlaveDemo.cpp>`
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
    * SIL Kit Network Simulator (optional)
Parameters
    * ``--network <name>``
      Name of the Lin network to use. 
      Defaults to 'LIN1'.
System Example
    Run the following commands in separate terminals:

    .. parsed-literal:: 

       # Registry (if not already running):
       |Registry|

       # Monitor (optional):
       |Monitor|

       # Lin Master:
       |DemoDir|/SilKitDemoLinMaster

       # Lin Slave:
       |DemoDir|/SilKitDemoLinSlave

       # System Controller:
       |SystemController| LinSlave LinMaster

    To run the demo without virtual time synchronization and start coordination, use the following commands in separate terminals:

    .. parsed-literal:: 

       # Registry (if not already running):
       |Registry|

       # Lin Master:
       |DemoDir|/SilKitDemoLinMaster --async --autonomous

       # Lin Slave:
       |DemoDir|/SilKitDemoLinSlave --async --autonomous

.. _sec:flexray-demo:

FlexRay
~~~~~~~

Abstract
    |DemoAbstractFlexRay|
Executables
    * ``SilKitDemoFlexrayNode0``
    * ``SilKitDemoFlexrayNode1``
Sources
    * :repo-link:`FlexrayNode0Demo.cpp <Demos/communication/Flexray/FlexrayNode0Demo.cpp>`
    * :repo-link:`FlexrayNode1Demo.cpp <Demos/communication/Flexray/FlexrayNode1Demo.cpp>`
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>`
    * SIL Kit Network Simulator (mandatory)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
Parameters
    * ``--network <name>``
      Name of the FlexRay network to use. 
      Defaults to 'PowerTrain1'.
System Example
    Run the following commands in separate terminals:

    .. parsed-literal:: 

       # Registry (if not already running):
       |Registry|

       # Network Simulator (assumed to be in PATH, necessary):
       sil-kit-network-simulator ./SilKit-Demos/FlexRay/NetworkSimulatorConfig.yaml

       # Monitor (optional):
       |Monitor|

       # Node 0:
       |DemoDir|/SilKitDemoFlexrayNode0

       # Node 1:
       |DemoDir|/SilKitDemoFlexrayNode1

       # System Controller:
       |SystemController| Node0 Node1 NetworkSimulator
Notes
    * The FlexRay demo requires the usage of the SIL Kit Network Simulator and virtual time synchronization.
    * It takes about 65ms (virtual time) until the starting the FlexRay cycle has started and the first FlexRay messages are transmitted.

.. _sec:pubsub-demo:

Publish/Subscribe
~~~~~~~~~~~~~~~~~

Abstract
    |DemoAbstractPubSub|
Executables
    * ``SilKitDemoPublisher``
    * ``SilKitDemoSubscriber``
Sources
    * :repo-link:`PublisherDemo.cpp <Demos/communication/PubSub/PublisherDemo.cpp>`
    * :repo-link:`SubscriberDemo.cpp <Demos/communication/PubSub/SubscriberDemo.cpp>`
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
Parameters
    * No demo specific command line arguments
System Example
    Run the following commands in separate terminals:

    .. parsed-literal:: 
    
       # Registry (if not already running):
       |Registry|
    
       # Monitor (optional):
       |Monitor|
    
       # Publisher:
       |DemoDir|/SilKitDemoPublisher 
    
       # Subscriber:
       |DemoDir|/SilKitDemoSubscriber
    
       # System Controller:
       |SystemController| Publisher Subscriber
    
    To run the demo without virtual time synchronization and start coordination, use the following commands in separate terminals:
    
    .. parsed-literal::
    
       # Registry (if not already running):
       |Registry|
    
       # Publisher:
       |DemoDir|/SilKitDemoPublisher --async --autonomous
    
       # Subscriber:
       |DemoDir|/SilKitDemoSubscriber --async --autonomous

.. _sec:rpc-demo:

Rpc
~~~

Abstract
    |DemoAbstractRPC|
Executables
    * ``SilKitDemoRpcClient``
    * ``SilKitDemoRpcServer``
Sources
    * :repo-link:`RpcClientDemo.cpp <Demos/communication/Rpc/RpcClientDemo.cpp>`
    * :repo-link:`RpcServerDemo.cpp <Demos/communication/Rpc/RpcServerDemo.cpp>`
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
Parameters
    * No demo specific command line arguments
System Example
    Run the following commands in separate terminals:

    .. parsed-literal:: 
    
       # Registry (if not already running):
       |Registry|
    
       # Monitor (optional):
       |Monitor|
    
       # Server:
       |DemoDir|/SilKitDemoRpcServer
    
       # Client:
       |DemoDir|/SilKitDemoRpcClient
    
       # System Controller:
       |SystemController| RpcServer RpcClient
    
    To run the demo without virtual time synchronization and start coordination, use the following commands in separate terminals:
    
    .. parsed-literal::
    
       # Registry (if not already running):
       |Registry|
    
       # Server:
       |DemoDir|/SilKitDemoRpcServer --async --autonomous
    
       # Client:
       |DemoDir|/SilKitDemoRpcClient --async --autonomous
    

Implementation details
~~~~~~~~~~~~~~~~~~~~~~

The base implementation of the demos is located in ``Demos\include\ApplicationBase.hpp``.

Further, the participant pairs of the demos share some common behavior like printing frames, status handling or small helper classes.
This is located in the header ``<DemoName>Common.hpp`` of the respective demo folders.

To constructed your own demo, use the following template:

.. code-block:: c++

    #include "ApplicationBase.hpp"

    // Inherit from ApplicationBase that provides common SIL Kit features
    class MyDemoParticipant: public ApplicationBase
    {
    public:
        // Inherit constructors
        using ApplicationBase::ApplicationBase;

    private:
        
        // Member variables like SIL Kit controller pointers, Demo state, etc
        std::string _myOption;
        bool _myFlag;

        // The following overrides are invoked in the right order by the ApplicationBase
        // This enables:
        // - General and demo specific command line arguments
        // - Setup of the SIL Kit lifecycle
        // - Controller creation and initialization
        // - SimulationStepHandler vs. thread based execution (--async)
        // - Signal handling to CTRL-C at any time
        // - Basic logging

        // Extend the command line argument list
        void AddCommandLineArgs() override
        {
            GetCommandLineParser()->Add<CommandlineParser::Option>(
                "myOption", "o", "DefaultValue", "-o, --myOption <value>",
                std::vector<std::string>{"Description Line 1.", "Description Line 2"});

            GetCommandLineParser()->Add<CommandlineParser::Flag>(
                "myFlag", "f", "-f, --myFlag",
                std::vector<std::string>{"Description Line 1.", "Description Line 2"});
        }

        // Evaluate the command line argument list
        void EvaluateCommandLineArgs() override
        {
            _myOption = GetCommandLineParser()->Get<CommandlineParser::Option>("myOption").Value();
            _myFlag = GetCommandLineParser()->Get<CommandlineParser::Flag>("myFlag").Value();
        }

        // Create all SIL Kit controllers here
        void CreateControllers() override
        {
            // All SIL Kit features can be accessed via GetParticipant()
            // _myController = GetParticipant()->CreateXYZController(...);
        }

        // Controller initialization goes here
        void InitControllers() override
        {
        }
    
        // Called in each simulation step when running with time synchronization
        void DoWorkSync(std::chrono::nanoseconds now) override
        {
            // _myController->Send(...)   
        }

        // Called in a worker thread when running without time synchronization
        void DoWorkAsync() override
        {
            // _myController->Send(...)   
        }
    };

    int main(int argc, char** argv)
    {
        Arguments args;
        args.participantName = "MyDemoParticipant"; // Always specify a meaningful default participant name 

        MyDemoParticipant app{args};

        // This will trigger AddCommandLineArgs() and EvaluateCommandLineArgs()
        // Optionally, a set of excluded default command line arguments can be specified
        app.SetupCommandLineArgs(argc, argv, "Description for the command line help");
    
        // This will trigger CreateControllers(), InitControllers() and then cyclically DoWorkSync() or DoWorkAsync()
        return app.Run();
    }
