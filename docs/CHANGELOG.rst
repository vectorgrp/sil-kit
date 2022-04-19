Changelog
================================

All notable changes to the IntegrationBus project shall be documented in this file.

The format is based on `Keep a Changelog (http://keepachangelog.com/en/1.0.0/) <http://keepachangelog.com/en/1.0.0/>`_.

[3.99.19] - 2022-04-19
--------------------------------

This delivery is the first public delivery since 3.6.1. Since then, development has focused on preparing the public
open source release of the Vector Integration Bus in August 2022. This delivery is not ABI/API/network compatible to
earlier 3.x deliveries. Future 3.99.x deliveries are not expected to be API/ABI/network compatible to each other as we finalize 
the API and network layer for the public open source 4.0 release. These deliveries are intended for evaluation purposes 
and to make the current status of the Vector Integration Bus as transparent as possible.

In this delivery we stabilize the DataMessage API such that it resembles the final API as closely as possible.

Major changes since 3.6.1 (see changelog for details):

 - Introduction of optional and distributed participant configuration
 - DataMessage API as replacement for IoControllers and GenericMessage API
 - Remote Procedure Call API
 - Refactoring of CLI of Utilities to be consistent
 - Tracing and Replay currently not functional

Added
~~~~~

- Participant methods to create DataPublisher, DataSubscriber, RpcClient and RpcServer now have an simplified
  overload with the controller name ('canonicalName') as single argument. In this variant, the controller name is used as topic/rpcChannel.
  The C-Api doesn't provide these simplified methods.

  - ``IntegrationBus/include/ib/mw/IParticipant.hpp``

    + new:

    .. code-block:: c++
      
      IParticipant::CreateDataPublisher(const string& canonicalName) -> ...;
      IParticipant::CreateDataSubscriber(const string& canonicalName) -> ...;
      IParticipant::CreateRpcClient(const string& canonicalName) -> ...;
      IParticipant::CreateRpcServer(const string& canonicalName) -> ...;

Removed
~~~~~~~

- The typedef ``ib::mw::sync::IParticipantController::TaskHandleT`` was removed due to not being used anywhere.

- The header ``IntegrationBus/include/ib/mw/sync/ISyncMaster.hpp`` was removed due to not being used anywhere.

- Replaced the single-member struct ``DataExchangeFormat`` with its sole member, the media type string.
  The following data type and some associated free functions were removed:

  - ``IntegrationBus/include/ib/sim/data/DataMessageDatatypes.hpp``

    .. code-block:: c++

      struct ib::sim::data::DataExchangeFormat;
      bool operator ==(const DataExchangeFormat &, const DataExchangeFormat &);

  - ``IntegrationBus/include/ib/sim/data/string_utils.hpp``

    .. code-block:: c++

      ib::sim::data::to_string(const DataExchangeFormat&) -> std::string;
      ib::sim::data::operator<<(std::ostream& out, const DataExchangeFormat& dataExchangeFormat) -> std::ostream&;

  - ``IntegrationBus/include/ib/capi/DataPubSub.h``

    .. code-block:: c++

      typedef struct { ... } ib_Data_ExchangeFormat;

Changed
~~~~~~~

- Updated documentation of :doc:`./usage/demos`, :doc:`./usage/utilities` and :doc:`./configuration/configuration`


- Participant (formerly 'ComAdapter') methods to create DataPublisher, DataSubscriber, RpcClient and RpcServer now have an additional 
  argument for the controller name. The controller name is used to to reference the controller in the configuration file 
  (formerly, 'topic'/'functionName' was used).

  - ``IntegrationBus/include/ib/mw/IComAdapter.hpp``

    + new:

    .. code-block:: c++
      
      IComAdapter::CreateDataPublisher(const string& canonicalName, ...) -> ...;
      IComAdapter::CreateDataSubscriber(const string& canonicalName, ...) -> ...;
      IComAdapter::CreateRpcClient(const string& canonicalName, ...) -> ...;
      IComAdapter::CreateRpcServer(const string& canonicalName, ...) -> ...;

  - ``IntegrationBus/include/ib/capi/DataPubSub.h``

    + new:

    .. code-block:: c++
  
      typedef ib_ReturnCode (*ib_Data_Publisher_Create_t)(ib_Data_Publisher** outPublisher, ib_Participant* participant,
                                                        const char* controllerName, ...);

      typedef ib_ReturnCode (*ib_Data_Subscriber_Create_t)(ib_Data_Subscriber** outSubscriber, ib_Participant* participant,
                                                              const char* controllerName, ...);

- Renamed Public-API for DataSubscriber:

  - ``IntegrationBus/include/ib/sim/data/IDataSubscriber.hpp``

    + old: 
    
    .. code-block:: c++
    
      IDataSubscriber::SetDefaultReceiveMessageHandler(...);
      IDataSubscriber::RegisterSpecificDataHandler(...);

    + new:

    .. code-block:: c++

      IDataSubscriber::SetDefaultDataMessageHandler(...);
      IDataSubscriber::AddExplicitDataMessageHandler(...);

  - ``IntegrationBus/include/ib/capi/DataPubSub.h``

    + old:

    .. code-block:: c++
    
      typedef ib_ReturnCode (*ib_Data_Subscriber_SetDefaultReceiveDataHandler_t)(...);
      typedef ib_ReturnCode (*ib_Data_Subscriber_RegisterSpecificDataHandler_t)(...);

    + new:

    .. code-block:: c++

      typedef ib_ReturnCode (*ib_Data_Subscriber_SetDefaultDataMessageHandler_t)(...);
      typedef ib_ReturnCode (*ib_Data_Subscriber_AddExplicitDataMessageHandler_t)(...);

- Renamed/wrapped structs and using statements in PubSub-context for the C/Cpp-API:

    + old: 
    
    .. code-block:: c++

      // Cpp-API
      using DataHandlerT =
      std::function<void(ib::sim::data::IDataSubscriber* subscriber, const std::vector<uint8_t>& data)>;
      
      using NewDataSourceHandlerT = std::function<void(ib::sim::data::IDataSubscriber* subscriber,
                                                      const std::string& topic, const std::string& mediaType,
                                                      const std::map<std::string, std::string>& labels)>;
      
      // C-API
      typedef void (*ib_Data_Handler_t)(void* context, ib_Data_Subscriber* subscriber, const ib_ByteVector* data);
      
      typedef void (*ib_Data_NewDataSourceHandler_t)(void* context, ib_Data_Subscriber* subscriber, const char* topic,
                                                      const char* mediaType, const ib_KeyValueList* labels);
    + new:

    The new DataMessageEvent now contains a timestamp with the send time set by the DataPublisher. 
    Formerly, the reception callback only contained the raw data. The information about a new DataPublisher in the 
    NewDataPublisherHandlerT now is bundeled in a stuct called 'NewDataPublisherEvent' also containing a reception
    timestamp.

    .. code-block:: c++

      // Cpp-API
      using DataMessageHandlerT =
      std::function<void(ib::sim::data::IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent)>;
      
      using NewDataPublisherHandlerT =
      std::function<void(ib::sim::data::IDataSubscriber* subscriber, const NewDataPublisherEvent& newDataPublisherEvent)>;
      
      // C-API
      typedef void (*ib_Data_DataMessageHandler_t)(void* context, ib_Data_Subscriber* subscriber, 
                                                  const ib_Data_DataMessageEvent* dataMessageEvent);
      
      typedef void (*ib_Data_NewDataPublisherHandler_t)(void* context, ib_Data_Subscriber* subscriber,
                                                        const ib_Data_NewDataPublisherEvent* newDataPublisherEvent);

- The header ``EndpointAddress.hpp``, ``IReplay.hpp``, ``ITraceMessageSink.hpp``, ``ITraceMessageSource.hpp`` and
  ``TraceMessage.hpp`` are now internal headers.
  The typedef ``ib::mw::ParticipantId`` which used to be defined here was moved into its own public header file.

  - ``IntegrationBus/include/ib/mw/ParticipantId.hpp``

    + new:

    .. code-block:: c++

      using ParticipantId = ...;

  - ``IntegrationBus/include/ib/mw/sync/SyncDatatypes.hpp``

    + old:

    .. code-block:: c++

      #include "ib/mw/EndpointAddress.hpp"

    + new:

    .. code-block:: c++

      #include "ib/mw/ParticipantId.hpp"

- Replaced the single-member struct ``DataExchangeFormat`` with its sole member, the media type string.
  The following method and function type signatures have changed:


    + old:

    .. code-block:: c++

      IComAdapter::CreateDataPublisher(..., const DataExchangeFormat&, ...) -> ...;
      IComAdapter::CreateDataSubscriber(..., const DataExchangeFormat&, ...) -> ...;

    + new:

    .. code-block:: c++

      IComAdapter::CreateDataPublisher(..., const std::string& ...) -> ...;
      IComAdapter::CreateDataSubscriber(..., const std::string& ...) -> ...;

  - ``IntegrationBus/include/ib/sim/data/DataMessageDatatypes.hpp``

    + old:

    .. code-block:: c++

      NewDataSourceHandlerT = std::function<void(..., const DataExchangeFormat&, ...)>;

    + new:

    .. code-block:: c++

      NewDataSourceHandlerT = std::function<void(..., const std::string&, ...)>;

  - ``IntegrationBus/include/ib/sim/data/IDataSubscriber.hpp``

    + old:

    .. code-block:: c++

      IDataSubscriber::RegisterSpecificDataHandler(const DataExchangeFormat&, ...) -> ...;

    + new:

    .. code-block:: c++

      IDataSubscriber::RegisterSpecificDataHandler(const std::string&, ...) -> ...;

  - ``IntegrationBus/include/ib/capi/DataPubSub.h``

    + old:

    .. code-block:: c++

      typedef void (*ib_Data_NewDataSourceHandler_t)(..., const ib_Data_ExchangeFormat* dataExchangeFormat, ...);
      IntegrationBusAPI ib_ReturnCode ib_Data_Publisher_Create(..., ib_Data_ExchangeFormat* dataExchangeFormat, ...);
      typedef ib_ReturnCode (*ib_Data_Publisher_Create_t)(..., ib_Data_ExchangeFormat* dataExchangeFormat, ...);
      IntegrationBusAPI ib_ReturnCode ib_Data_Subscriber_Create(..., ib_Data_ExchangeFormat* dataExchangeFormat, ...);
      typedef ib_ReturnCode (*ib_Data_Subscriber_Create_t)(..., ib_Data_ExchangeFormat* dataExchangeFormat, ...);
      IntegrationBusAPI ib_ReturnCode ib_Data_Subscriber_RegisterSpecificDataHandler(..., ib_Data_ExchangeFormat* dataExchangeFormat, ...);

    + new:

    .. code-block:: c++

      typedef void (*ib_Data_NewDataSourceHandler_t)(..., const char* mediaType, ...);
      IntegrationBusAPI ib_ReturnCode ib_Data_Publisher_Create(..., const char* mediaType, ...);
      typedef ib_ReturnCode (*ib_Data_Publisher_Create_t)(..., const char* mediaType, ...);
      IntegrationBusAPI ib_ReturnCode ib_Data_Subscriber_Create(..., const char* mediaType, ...);
      typedef ib_ReturnCode (*ib_Data_Subscriber_Create_t)(..., const char* mediaType, ...);
      IntegrationBusAPI ib_ReturnCode ib_Data_Subscriber_RegisterSpecificDataHandler(..., const char* mediaType, ...);

- Renamed ``ib::mw::IComAdapter`` to ``ib::mw::IParticipant``, and 
  renamed ``IntegrationBus/include/ib/mw/IComAdapter.hpp`` to ``IntegrationBus/include/ib/mw/IParticipant.hpp``.
  Further renamed method ``ib::CreateSimulationParticipant(...)`` to ``ib::CreateParticipant(...)``:

  - ``IntegrationBus/include/ib/IntegrationBus.hpp``
  
    + old:

    .. code-block:: c++
      
      IntegrationBusAPI auto CreateSimulationParticipant(
          std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig, const std::string& participantName,
          const uint32_t domainId, bool isSynchronized) -> std::unique_ptr<mw::IComAdapter>;
    
    + new:

    .. code-block:: c++
      
      IntegrationBusAPI auto CreateParticipant(
          std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig, const std::string& participantName,
          const uint32_t domainId, bool isSynchronized) -> std::unique_ptr<mw::IParticipant>;

[3.7.18] - 2022-04-05
--------------------------------

Added
~~~~~
- Support for MinGW builds was added.
  The CI builds use the native MinGW builds from winlibs, but a
  cross-compile CMake toolchain file is provided in `IntegrationBus/cmake/mingw-w64-toolchain.cmake`.
  Warnings are not treated as errors for this compilation target, due to some known
  issues in the MinGW-w64 toolchain.

  MinGW libraries are not distributed as part of VIB packages.
  The user must ensure that the following libraries are in PATH:

    .. code-block:: sh

        libgcc_s_seh-1.dll
        libstdc++-6.dll
        libwinpthread-1.dll
   
Fixed
~~~~~
- Fixed typo in IbRegistry command help output.
- Fixed typo in IbSystemMonitor command help output.
- The IbRegistry hostname defined in the configuration is now resolved and
  used to create listening sockets.
  IPv6 addresses are not supported, yet.
- The `lin::ControllerConfig` data type now uses a message history of size 1.
  If the value was set by a `LinController::Init` call, it will be
  retained and automatically transmitted to newly connecting participants.
  
Changed
~~~~~~~
- Demo adaptions
  - Ethernet Demo only uses Set/GetRawFrame calls
  - CAN & Ethernet demo can now run as asynchronous participants (add `--async` as command line argument)

- Updated documentation of :doc:`./vibes/networksimulator` and :doc:`./simulation/simulation`


[3.7.13] - 2022-03-24
--------------------------------
This is an internal build.

Changed
~~~~~~~
- Added a new command line parser for all utilities, to have a consistent user experience.
  Long and short options are supported, invoke the utilities with `--help` to
  see a list of supported options:

    + old: 

    .. code-block:: sh

       IbRegistry Configuration.json 42

    + new: 
    .. code-block:: sh

       IbRegistry --domain 42 Configuration.json
       #the following lists all supported options:
       IbRegistry --help

    + old: 

    .. code-block:: sh

       IbSystemController  42 participant1 participant2 ...

    + new: 
    .. code-block:: sh

       IbSystemController  --domain 42 participant1 participant2 ...

  The VIBE-NetworkSimulator also supports this new command line interface.

Fixed:
~~~~~~
- The IntegrationBus shared library does not log to stdout anymore.
  The internal logger is now used when loading extensions or creating named pipes.
- CAN: send a valid FD frame in CAN Demo by setting hte FD format indicator.


[3.7.8] - 2022-03-09
--------------------------------
This is an internal build.

Removed
~~~~~~~

- Removed deprecated ISystemController functions `Initialize()` and `Reinitialize()` 
  which took a numeric participant id as argument.
  Use participant names instead.

    + old:

    .. code-block:: c++
        
      void ib::mw::sync::ISystemController::Initialize(ParticipantId) const;
      void ib::mw::sync::ISystemController::ReInitialize(ParticipantId participantId) const;

    + new:

    .. code-block:: c++
       
       void ib::mw::sync::ISystemController::Initialize(const std::string& participantName) const;
       void ib::mw::sync::ISystemController::ReInitialize(const std::string& participantName) const;


- Removed the ConfigBuilder from the public API in the `include/ib` directory.
  The following headers were removed:
    
  .. code-block:: sh

    include/ib/cfg/ConfigBuilder.hpp
    include/ib/cfg/ControllerBuilder.hpp
    include/ib/cfg/DataPortBuilder.hpp
    include/ib/cfg/ExtensionConfigBuilder.hpp
    include/ib/cfg/GenericPortBuilder.hpp
    include/ib/cfg/LinkBuilder.hpp
    include/ib/cfg/LoggerBuilder.hpp
    include/ib/cfg/NetworkSimulatorBuilder.hpp
    include/ib/cfg/ParentBuilder.hpp
    include/ib/cfg/ParticipantBuilder.hpp
    include/ib/cfg/ParticipantBuilder_detail.hpp
    include/ib/cfg/ReplayBuilder.hpp
    include/ib/cfg/RpcPortBuilder.hpp
    include/ib/cfg/SimulationSetupBuilder.hpp
    include/ib/cfg/SinkBuilder.hpp
    include/ib/cfg/SwitchBuilder.hpp
    include/ib/cfg/SwitchPortBuilder.hpp
    include/ib/cfg/TimeSyncBuilder.hpp
    include/ib/cfg/TraceSinkBuilder.hpp
    include/ib/cfg/TraceSourceBuilder.hpp
    include/ib/cfg/VAsioConfigBuilder.hpp
    include/ib/cfg/VAsioRegistryBuilder.hpp

- Removed all Configuration headers from `include/ib`. The configuration structure is no longer public.
  Use the opaque :cpp:class:`ib::cfg::IParticipantConfiguration` instead.
  The following headers were removed:

  .. code-block:: sh

     IntegrationBus/include/ib/cfg/OptionalCfg.hpp
     IntegrationBus/include/ib/cfg/all.hpp
     IntegrationBus/include/ib/cfg/fwd_decl.hpp
     IntegrationBus/include/ib/cfg/string_utils.hpp

Changed
~~~~~~~
- Adapted the demos to the new configuration file format.
   - Removed all old configuration files.
   - New configuration files only define a logger.
   - Bus system demos also provide an additional configuration file for VIBE-NetSim.
- Network names on controllers are optional.

Fixed:
~~~~~~
- Reduced a lot of compiler warnings. On CI builds, warnings are treated as errors.
- FlexRay: FlexRayController.Configure(...): values provided in the configuration file were not correctly overriding user-defined values.
- All Bus Systems: Fixed bugs related to configured controllers that had no defined network.
- CAN-Controller: Fixed a bug that prevented users from creating configured CAN controllers if the network was configured and the provided network name did not match the configured one.
- Fixed deadlock in destructor when :cpp:func:`SetSimulationTaskAsync()<ib::mw::sync::IParticipantController::SetSimulationTaskAsync()>`
  is used.
- Fixed LinkUp/LinkDown transmission in VIBE-NetworkSimulator.
- Fixed ethernet frame transmission with multiple switches in VIBE-NetworkSimulator.


[3.7.2] - unreleased
--------------------------------

Fixed
~~~~~
- Controller pointer in FlexRay Controller Wakeup handler

[3.7.1] - unreleased
--------------------------------

Fixed
~~~~~
- New Configuration format Network name override for Can and Ethernet


[3.7.0] - unreleased
--------------------------------

Changed
~~~~~~~

.. admonition:: Note: the IntegrationBus configuration and command line usage changed!

   The old simulation setup based configuration mechanism was replaced by a 
   dynamic configuration:
   The configuration file syntax changed and the utility command arguments changed.
   **Please note, the documentation is not yet updated to reflect all changes.**

- Switched to dynamic configuration concept: a simulation setup description is no
  longer necessary for running a simulation.
  All participants announce and register their services upon connecting to the registry.

    - Removed support of old config format from public includes ``ib/cfg/Config.hpp``

    - Removed CreateComAdapter (use CreateSimulationParticipant instead)

    + old:

    .. code-block:: c++

       auto comAdapter = ib::CreateComAdapter(config, "ParticipantName", domainId);

    + new:

    .. code-block:: c++

       auto participant = ib::CreateSimulationParticipant(config, "ParticipantName", domainId, true);

        
    - CreateSimulationParticipant takes the new configuration as first argument.
      To configure a synchronized participant, set the fourth argument to `true`.
    - Demos: 
        - Removed demos CanDemoNoSync, DynamicConfig, TickTickDone, GenericMessage, SimulationControl, SimulationControlNonBlocking as they are outdated
        - Updated remaining demos to use CreateSimulationParticipant and updated the configuration files.
    - Startup: 
        - IbSystemController (reference implementation) does not take JSON file as first argument anymore and takes all expected synchronized participants as command line arguments.

            .. code-block:: sh

             .\IbSystemController.exe 42 CanWriter CanReader
            
    - SystemController API:
      The system controller now expects a complete list of synchronized participants for proper working:
    
     :cpp:func:`ib::mw::sync::ISystemController::SetRequiredParticipants()`
        


.. admonition:: Note: the VIBE Network Simulator configuration format has changed!

  The format of the Network Simulator configuration has changed. Refer to the
  sample configuration files distributed with the VIBE-NetworkSimulator package.



- The network simulator's configuration was changed.
  Please note, that the documentation does not yet reflect these changes.
  The new configuration format resembles the ``NetworkSimulator`` block of the legacy config format but is stripped down to the minimum:
  it contains blocks for ``Switches`` and ``SimulatedNetworks`` (which were previously called ``SimulatedLinks``):

  + Sample of the new Network Simulator format:
    
    .. code-block:: javascript

        {
            "SchemaVersion": "1",
            "Description": "Small sample config with Link names from the VIB Demos",
            "Switches": [
                {
                    "Name": "FrontSwitch",
                    "Ports": [
                        {
                            "Name": "Port0",
                            "VlanIds": [1],
                            "Network": "FS-Port0"
                        },
                        {
                            "Name": "Port1",
                            "VlanIds": [1],
                            "Network": "FS-Port1"
                        }
                    ]
                }
            ],
            "SimulatedNetworks": [
                {
                    "Name": "CAN1",
                    "Type": "CAN"
                },
                {
                    "Name": "LIN1",
                    "Type": "LIN"
                },
                {
                    "Name": "FlexRay1",
                    "Type": "FlexRay"
                },
                {
                    "Name": "ETH1",
                    "Type": "Ethernet"
                },
                {
                    "Name": "FS-Port0",
                    "Type": "Ethernet"
                },
                {
                    "Name": "FS-Port1",
                    "Type": "Ethernet"
                }
            ]
        }

  The Network Simulator is shipped with two sample configuration files and
  the process invocation is the same: ``VIBE-NetworkSimulator ParticipantName path/to/configFile``

Compatibility with 3.6.3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note: The versions from 3.6.3 up to 3.6.16 changed a lot of key APIs and network compatibilties.

- Application binary interface (ABI): No 
- Application software interface (API): No
- Middleware network protocol (VAsio): No


[3.6.3] - 2022-01
--------------------------------

Removed
~~~~~~~
- removed the deprecated FastRtps middleware:
    - CMake: removed option `IB_MW_ENABLE_FASTRTPS`.
    - Demos: removed the FastRtps specific config files.
    - removed FastRtps specific integration tests.
    - API: removed deprecated `ib::CreateFastRtpsComAdapter` and `ib::CreateVAsioComAdapter`.
      Users should use the generic :cpp:func:`CreateComAdapter<ib::CreateComAdapter()>`,
      refer to :ref:`sec:mwcfg-vasio`.
    - ConfigBuilder: removed FastRtps configuration mechanism: `ib::cfg::ConfigBuilder::ConfigureFastRtps`.

  

[3.6.2] - 2021-12-16
--------------------------------

Fixed
~~~~~
- C-API: fix transmission of flags in CAN SendFrame (VIB-548).
- Links are now checked for unique endpoints (VIB-542).

Changes
~~~~~~~
- Remove FastRTPS submodule in main repository.
- cmake: make Linux detection more portable.
  We now use ``lsb_release`` to determine Linux Standard Base name and version of the distribution.
- C-API: refactoring as described in VIB-529.
  The experimental C API has been refactored to make use of a unified naming scheme.
  The following changes have been introduced:

  - All identifiers related to a specific object (or topic) now use the prefix ``ib_Topic_``.
    For instance, ``ib_CanController_Stop`` has been renamed to ``ib_Can_Controller_Stop``.
    The names are now unified for all functions and types, and for the 'topics'
    SimulationParticipant, Can, Ethernet, FlexRay, Lin.

  - The functions ending with ``_create`` or ``_destroy`` have been renamed to ``_Create`` or ``_Destroy``.


  
Added
~~~~~
- Added SerDes support library for easy serialization /deserialization of user data.
  Refer to `Demos/GenericMessage/GenericMessageDemoSerDes.cpp` for an example (VIB-514).

Compatibility with 3.6.1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): No (build is disabled)
- Middleware network protocol (VAsio): Yes


[3.6.1-QA] - 2021-12-10
--------------------------------
This is a Quality Assured Release.

Added
~~~~~
- The launcher now supports YAML formatted configuration files based on their file
  name extension ``.yaml`` or ``.yml``.
  This requires an installation of PyYAML (e.g., via ``pip install PyYAML``).
  Please note, that JSON schema validation with YAML files is not supported (VIB-526).
- Added the option ``--exclude-registry`` to the launcher.
  This will prevent the launcher from starting an IbRegistry process on its own (VIB-527).

Changed
~~~~~~~
.. admonition:: Note: the VIB-CANoe Add-on is now discontinued.

  The CANoe Add-on is affected by a known regression.
  Upcoming releases of Vector CANoe will support the IntegrationBus directly.

- The VIB CANoe addon is now discontinued.
  It is affected by **a known regression** and will not be part of this release.
  Users of the VIB-CANoe add-on are encouraged to remain on the previous quality
  assured release.
  The upcoming release of Vector CANoe 16 will support the IntegrationBus directly.

Fixed
~~~~~
- Fixed a regression in the ``Replay`` feature and the VIBE-NetworkSimulator support (VIB-544).

Compatibility with 3.6.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): Yes 
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): No (build is disabled)
- Middleware network protocol (VAsio): Yes

[3.6.0] - 2021-12-01
--------------------------------
Please note, the version jump from v3.4.x to v3.6.x was necessary due to internal
releases already having the v3.5.x tag.

Added
~~~~~
- Add experimental C-API for life-cycle and time synchronization (VIB-492).
- Add experimental C-API for FlexRay (VIB-505).
- Add experimental C-API for LIN (VIB-516).
- Internal VIB threads now set their name for easier debugging on WIN32 and posix
  platforms (VIB-524).

Changed
~~~~~~~

.. admonition:: Note: Public API changed
  
  Some rarely used public headers and `IComAdapter` methods were removed.
  User code should re-compile cleanly with these changes.
  
- Some header files were removed from the public ``include/ib`` directories.
  These `IIbTo*` headers are only for internal use.
  This change should not affect any users of the public API directly (VIB-511).

Complete list of removed header files from ``include/ib``:

  .. code-block:: sh

     include/ib/mw/IIbEndpoint.hpp
     include/ib/mw/IIbMessageReceiver.hpp
     include/ib/mw/IIbReceiver.hpp
     include/ib/mw/IIbSender.hpp
     include/ib/mw/sync/IIbToParticipantController.hpp
     include/ib/mw/sync/IIbToSyncMaster.hpp
     include/ib/mw/sync/IIbToSystemController.hpp
     include/ib/mw/sync/IIbToSystemMonitor.hpp
     include/ib/sim/can/IIbToCanController.hpp
     include/ib/sim/can/IIbToCanControllerProxy.hpp
     include/ib/sim/can/IIbToCanSimulator.hpp
     include/ib/sim/eth/IIbToEthController.hpp
     include/ib/sim/eth/IIbToEthControllerProxy.hpp
     include/ib/sim/eth/IIbToEthSimulator.hpp
     include/ib/sim/fr/IIbToFrBusSimulator.hpp
     include/ib/sim/fr/IIbToFrController.hpp
     include/ib/sim/fr/IIbToFrControllerProxy.hpp
     include/ib/sim/generic/IIbToGenericPublisher.hpp
     include/ib/sim/generic/IIbToGenericSubscriber.hpp
     include/ib/sim/io/IIbToInPort.hpp
     include/ib/sim/io/IIbToOutPort.hpp
     include/ib/sim/lin/IIbToLinController.hpp
     include/ib/sim/lin/IIbToLinControllerProxy.hpp
     include/ib/sim/lin/IIbToLinSimulator.hpp

- Directly sending messages on the :cpp:class:`IComAdapter<ib::mw::IComAdapter>` via `SendIbMessage(...)`
  is not possible anymore.
  Sending messages is now only supported via the specific service controllers,
  see :ref:`VIB API<sec:api-services>` for an overview (VIB-511).

  For example, code that relies on `IComAdapter::SendIbMessage(EndpointAddress, const T&)` should
  use an appropriate controller for the given type T:

  + old:
    
    .. code-block:: c++

       // Example for message type T = CanMessage
       CanMessage msg{};
       comAdapter->SendIbMessage(EndpointAddress{/*implementation detail*/}, msg);

 + new:
   
   .. code-block:: c++

      // Example for message type T = CanMessage
      auto* controller = comAdapter->CreateCanController();
      CanMessage msg{};
      controller->SendMessage(msg);
 

Complete list of methods removed from the interface ``class IComAdapter`` in ``include/ib/mw/IComAdapter.hpp``:

  .. code-block:: c++

     virtual void SendIbMessage(EndpointAddress from, const sim::can::CanMessage& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, sim::can::CanMessage&& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::can::CanTransmitAcknowledge& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::can::CanControllerStatus& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::can::CanConfigureBaudrate& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::can::CanSetControllerMode& msg) = 0;
    
     virtual void SendIbMessage(EndpointAddress from, const sim::eth::EthMessage& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, sim::eth::EthMessage&& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::eth::EthTransmitAcknowledge& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::eth::EthStatus& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::eth::EthSetMode& msg) = 0;
    
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::FrMessage& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, sim::fr::FrMessage&& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::FrMessageAck& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, sim::fr::FrMessageAck&& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::FrSymbol& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::FrSymbolAck& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::CycleStart& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::HostCommand& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::ControllerConfig& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::TxBufferConfigUpdate& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::TxBufferUpdate& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::ControllerStatus& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::fr::PocStatus& msg) = 0;
    
     virtual void SendIbMessage(EndpointAddress from, const sim::lin::SendFrameRequest& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::lin::SendFrameHeaderRequest& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::lin::Transmission& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::lin::WakeupPulse& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::lin::ControllerConfig& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::lin::ControllerStatusUpdate& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::lin::FrameResponseUpdate& msg) = 0;
    
     virtual void SendIbMessage(EndpointAddress from, const sim::io::AnalogIoMessage& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::io::DigitalIoMessage& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::io::PatternIoMessage& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, sim::io::PatternIoMessage&& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sim::io::PwmIoMessage& msg) = 0;
    
     virtual void SendIbMessage(EndpointAddress from, const sim::generic::GenericMessage& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, sim::generic::GenericMessage&& msg) = 0;
    
     virtual void SendIbMessage(EndpointAddress from, const sync::NextSimTask& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sync::Tick& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sync::TickDone& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sync::QuantumRequest& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sync::QuantumGrant& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sync::ParticipantStatus& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sync::ParticipantCommand& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, const sync::SystemCommand& msg) = 0;
    
     virtual void SendIbMessage(EndpointAddress from, const logging::LogMsg& msg) = 0;
     virtual void SendIbMessage(EndpointAddress from, logging::LogMsg&& msg) = 0;
    
     virtual void OnAllMessagesDelivered(std::function<void(void)> callback) = 0;
     virtual void FlushSendBuffers() = 0;

.. admonition:: Note: FastRTPS build is disabled

  The FastRTPs middleware build is now disabled for the official Vector packages.

- The binary packages are built with the CMake flag ``IB_MW_ENABLE_FASTRTPS=OFF``.
  Calling a ``CreateComAdapter`` with an active middleware of ``FastRTPS`` will result
  in a runtime exception.
- MSVC: our CMakeSettings.json now directly supports building with Ninja.
- The transmit acknowledges in Ethernet were changed to work with a single participant.
  That is, in a trivial simulation sending an Ethernet message will be immediately
  acknowledged (VIB-490).

Fixed
~~~~~
- Config: prevent multiple statements of the same keyword, e.g. multiple ``"Links"``
  blocks now raise an error (VIB-528).
- Fix building with ``IB_BUILD_TESTS=OFF`` (VIB-536).

Compatibility with 3.4.6
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The API changes consist of internal headers which were removed from ``include/ib`` and
the removal of ``ComAdapter::SendIbMessage`` methods.

- Application binary interface (ABI): No 
- Application software interface (API): No
- Middleware network protocol (FastRTPS): No (build is disabled)
- Middleware network protocol (VAsio): Yes

[3.4.6] - 2021-11-16
--------------------------------

Changed
~~~~~~~

.. admonition:: Note: the FastRTPS middleware is now deprecated
  
   See :ref:`sec:mwcfg` for migration instructions.

- The middleware `FastRTPS` is now marked as deprecated.
  This middleware will be removed in the future.
  The middleware specific `CreateFastRtpsComAdapter` API has been
  marked as deprecated for a long time.
  Users should adopt the generic :cpp:func:`CreateComAdapter<ib::CreateComAdapter()>`,
  refer to :ref:`sec:mwcfg-enable-vasio` for instructions.
- C-API for ethernet was refactored and improved (VIB-489).
- C-API for CAN was refactored and internal integrity checks and unit tests
  added (VIB-464 VIB-465 VIB-466 VIB-460 VIB-462).
- The transmit acknowledges in CAN were changed to work with a single participant.
  That is, in a trivial simulation sending a CAN message will be immediately acknowledged (VIB-473 VIB-478).
- Updated gtest to version 1.11. This version supports a simplified
  MOCK_METHOD macro with specifiers like `const, override`. (VIB-474).

Added
~~~~~
- Added new functional tests for CAN and Ethernet without simulation control
  flow / synchronization (VIB-491 VIB-494).
- A new C-API for a DataPublisher / Subscriber (VIB-432).

Fixed
~~~~~
- Fixed regression in ``Pause()/Continue()`` calls of ParticipantController.
  The `SimulationControl` Demo was added and used as a stress test (AFTMAGT-330 VIB-499).

Compatibility with 3.4.5
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No (interface change due to transmit acknowledges)
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.4.5] - 2021-11-03
--------------------------------

Added
~~~~~
- C-API: added initial ethernet bindings for the C language.
  Refer to :ref:`sec:capi` for documentation.
  Currently, the Ethernet controllers work without time-synchronization.
  A Demo is provided in ``Demos/CEthernet`` (VIB-431).

Fixed
~~~~~
- Clarify the documentation of synchronization types and the usage of
  ``SetPeriod()`` (AFTMAGT-329).
- Fix YAML conversions with the ``vib-config-tool``.
  The YAML parser was emitting JSON formatted input back in JSON format.
  Ensure that default config values are properly serialized, or if unchanged are
  omitted from the output (AFTMAGT-331).
- Fix VAsio config values in the JSON schema.
  Update the ConfigBuilder to support the latest config additions.
  Clarified the documentation, which indicated misplaced config items nested in the
  ``Registry`` config item (AFTMAGT-328).
- Ensure user input (in different encodings) is not used verbatim in the
  domain socket creation (VIB-459).


Compatibility with 3.4.4
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.4.4] - 2021-10-20
--------------------------------

Added
~~~~~
- Added initial C language bindings. Please note, the C API is currently
  work in progress and as such might change in the future.
  The documentation is in :ref:`sec:capi`.
  C language bindings exist for CAN controllers without time-synchronzition
  and creating simulation participants from config files.

Compatibility with 3.4.3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.4.3-QA] - 2021-09-15
--------------------------------
This is a Quality Assured Release.

Added
~~~~~
- CMake: the options IB_MW_ENABLE_VASIO and IB_MW_ENABLE_FASTRTPS can be used to
  disable the middlewares at compile time.
- Config: introduced a 'SchemaVersion' field in the config formats, to allow for future
  changes.

Fixed
~~~~~
- VAsio: fix a bug affecting distributed (networked) setups. Participants did
  only listen on localhost addresses and the registry did not distribute proper
  endpoint addresses to other participants.

.. admonition:: Note: this bug affects users of VAsio with networked setups

  Users with VIB setups across multiple hosts (with non-localhost addresses)
  should upgrade to this VIB version.

- VAsio: improve performance. The `TraceTx()`/`TraceRx()` invocations were expanding
  their arguments and applying a `to_string()` operation on the VIB message
  payloads. Now, this is code path is only executed when `Trace` log level is active.

- UB sanitizer: fixed undefined behavior in MessageBuffer (unaliased memory
  accesses) and Watchdog (integer overflow).

- Fixed missing 'ActiveMiddleware' statements in Can/FastRTPS config files and
  ensure it is properly parsed with the new YamlConfigParser (VIB-375).

- Fixed out of bounds exception when configuring tracing and replaying.
  Also ensure that configuring a replay block with a 'Receive' direction is able
  to replay messages (VIB-372).

- Ensure that 'SearchPathHints' are passed on to the VIBE extension implementing
  the IReplayDataProvider interface (VIB-378).

- Fix accidentally setting the badbit on the ostream when formatting Ethernet
  MAC addresses and LIN frames.

Changed
~~~~~~~
- ``Config::FromJsonString`` now uses a Yaml-parser internally. To update legacy
  JSON configs to the current valid format we provide the ``vib-config-tool``
  utility. It supports deprecated controller and port declarations (cf `[3.1.0] - 2020-06-15`_)
  and also network simulator configurations at `SimulationSetup` scope (cf `[3.3.0] - 2020-08-12`_).
  It can output in `JSON` or `YAML` format. For example, to ensure a JSON config
  is up to date:

  .. code-block:: sh

     vib-config-tool --convert oldFile.json updated.json --format json

.. admonition:: Note: the IbRegistry executable was moved again from the `IntegrationBus-NonRedistributable` directory to the `IntegrationBus/bin`

    The relocation of the registry to the NonRedistributable caused so much
    irritation and breakage that we undid this change.

- To reduce the fallout of moving the IbRegistry we put it back into
  `IntegrationBus/bin/`. For compatibility the IbLauncher searches the registry
  executable in both directories.

Removed
~~~~~~~

- Removed the json11 from the source-tree. We now use yaml-cpp to parse JSON and YAML
  configuration files. Please note, that the yaml-based parser still accepts
  JSON formatted inputs.

Compatibility with 3.4.2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No (json11 symbols removed)
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.4.2] - 2021-08-04
--------------------------------

Added
~~~~~

- VAsio now supports UNIX domain sockets as transport. By default, all participants
  open domain sockets locally and advertise the TCP/IP and local endpoints to other participants.
  If a participant is unable to connect to another participant locally, the TCP/IP
  endpoint is used for connecting without raising an error.
  As such, the local domain connectivity is optional and fully transparent to the users.
  Because of the path limitations of domain sockets the socket files are stored
  in the system temporary directory (e.g. %TEMPDIR% or $TMP).
  To allow for simulation isolation, the socket files have the participant IDs,
  domain ID, and the simulation's working directory encoded in the file name.

- The CMake build system is now better equipped for reproducible builds on Linux platforms.
  CI builds now set the TZ, LANG and SOURCE_DATE_EPOCH environment variables to known values.
  The time of the last commit on the master branch is used to initialize SOURCE_DATE_EPOCH.
  Packaged source-trees now contain a pre-processed version_macros.hpp recording the commit
  hash.

Fixed
~~~~~

- Various fixes for our scripted Jenkins pipeline to improve stability on our CI environment.

Changed
~~~~~~~

- VAsio: we imported the Asio library to version 1.18.2.
  This includes many bugfixes and also domain socket support for Windows 10.
  The FastRTPS packages used for CI/CD builds had to be rebuilt using this new version of
  Asio.

- CMake: the third party handling code was refactored. Instead of using
  variables we now have targets to link against the third party components.

Compatibility with 3.4.1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No (additions to `struct ib::cfg::VAsio::Config`)
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.4.1] - 2021-06-25
--------------------------------

Added
~~~~~
- Added native YAML parser implementation.
  This provides better error messages and warnings for parsing YAML and JSON config files
  via the ``Config::FromYamlString``/``Config::FromYamlFile`` procedures.
  When the parser encounters an unknown element it emits a warning.
  However, if the element uses a reserved element name of the config 
  schema the warning is turned into an error (AFTMAGT-314).
  Please note, that schema versioning for backward compatibility is
  not yet implemented and will be addressed in the future.

Fixed
~~~~~
- Fixed warnings during 32-bit builds: an int64_t was truncated to int.
- Build: relax some warning levels for gtest and remove useless compile
  flags on windows.

Changed
~~~~~~~
- When shutting down, do not print an error message that we're already
  shutting down.

.. admonition:: Note: the IbRegistry executable was moved to the NonRedistributable directory.

  The location of the IbRegistry executable now matches that of the vib-registry shared library,
  cf. :ref:`sec:util-registry`.

- Similar to the vib-registry shared library, the executable was moved to the
  IntegrationBus-NonRedistributable directory.



Compatibility with 3.4.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.4.0] - 2021-06-01
--------------------------------

Added
~~~~~
- Added submodule yaml-cpp. This will be the base of our native
  YAML configuration parser.
- Added ``ib::cfg::Config::FromYamlString`` and ``ib::cfg::Config::FromYamlFile`` to load
  configuration from YAML formatted input. This currently transforms
  the YAML input into JSON and re-uses the JSON config parser.

Compatibility with 3.3.10
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): No (additions to ``ib::cfg::Config``)
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.3.10-QA] - 2021-05-10
--------------------------------

This is a Quality Assured Release.

Added
~~~~~
- Added MdfChannel identification to replaying config.
  This allows replaying MDF4 trace files that do not originate from VIB
  simulation runs, cf. :ref:`sec:replay-foreign`.

Fixed
~~~~~
- Allow tracing while replaying on I/O InPorts (VIB-159).
- Allow tracing while replaying on GenericSubscribers (VIB-159).
- Fix trivial FlexRay simulation. State transitions when a wakeup command was issued were not properly computed (VIB-154).
- Ensure that only active replay controllers are configured.
  This fixes a crash when multiple controllers were defined, but only one was active (VIB-160).
- Allow tracing messages on a LIN master when replaying is active (VIB-158).
- Fix null pointer derference in PcapReader when input file name was missing in configuration.
  Also ensure that the config has non-empty input and output file paths (VIB-156).
- The Launcher will attempt to clean up the CANoe environment several times when shutting down.
  This ensures a clean CANoe installation when CANoe is slow to shutdown, e.g. when launcher is interrupted
  by a user (VIB-153).
- Fix LIN Sleep frames when using the VIBE Network Simulator.
  When the ``ILinController::GoToSleep()`` was invoked, a `sleep` frame was transmitted and
  then the controller's internal state was set to `Sleeping`. This caused the Network Simulator
  to abort the running sleep-frame transmission and an erroneous  LIN_RX_NO_RESPONSE frame was generated.
  An additional `sleep pending` state is introduced which allows completion of pending transmissions before entering
  the `sleep` state. (VIB-155)

  .. admonition:: VIBE Network Simulator compatibility 

     To ensure interoperability you should use VIBE Network Simulator v3.3.10
     in all setups involving detailed LIN simulations.
     See compatibility below for details.

  
Changed
~~~~~~~
- Print acknowledgement on std::cout when an extension is loaded.
- Throw IB exceptions instead of runtime_error where applicable.
- Update the Demo configs to newer ``NetworkSimulator`` configuration scheme (VIB-156).
- Updated JSON config schema to include ``TraceSource`` and ``Replay`` config blocks (VIB-156).


Compatibility with 3.3.9
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Due to changes in the config API we are not ABI compatible.

Please note that the detailed simulation of LIN requires a matching VIBE Network Simulator v3.3.10.
The addition of a new internal state makes the current VIB release incompatible with older Network Simulators for the detailed LIN simulation.


- Application binary interface (ABI): No (Due to Config)
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.3.9] - 2021-04-09
--------------------------------

Added
~~~~~
- Add documentation for :doc:`Replaying and Tracing<usage/replay>` (AFTMAGT-308).
- Added support for replaying FlexRay messages (AFTMAGT-289).
  Replaying FlexRay requires the use of the VIBE-NetworkSimulator.
  The NetworkSimulator is attached as a replay controller to the replay scheduler, cf. :ref:`sec:replay-architecture` for an overview.
- Added support for replaying FlexRay messages in the VIBE-NetworkSimulator (AFTMAGT-290).
  Please note, that the startup/synchronization sequence is not part of a trace file  and time stamps of the
  replay might deviate from the original traced messages.
  The logical order of messages is kept after the synchronization has been established.
  Refer to section :ref:`sec:replay` for a summary of supported features and limitations.
- Add a :ref:`Replay<sec:cfg-participant-replay>` configuration block to the NetworkSimulator
  configuration.

Fixed
~~~~~
- The config parser no longer uses asserts when validating a Config.
  A misconfiguration exception is now thrown, which can be handled by the user (AFTMAGT-309).

Compatibility with 3.3.8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Due to changes in the config API we are not ABI compatible.

- Application binary interface (ABI): No (Due to Config)
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.3.8] - 2021-02-18
--------------------------------

Added
~~~~~
- The CI build pipeline now includes Ubuntu 20.04 as a build target, which is used
  to run automated unit tests and integration tests (AFTMAGT-300).
- Added support for replaying LIN frames using the VIBE-MDF4Tracing extension.
  This is exclusively supported on LIN master controllers.
  Replaying on LIN slaves is not supported, even though tracing is possible
  on these controllers.
  Invoking transmission API calls during replay is also not supported on a master (AFTMAGT-288).
- Setting VAsio TCP flags is now supported from a JSON config file, cf. :ref:`sec:mwcfg-vasio` (AFTMAGT-305).


Fixed
~~~~~
- A graceful connection shutdown does no longer result in an error message when
  using the VAsio  middleware (AFTMAGT-299).
- Starting multiple VAsio registries on the same TCP/IP port resulted in
  empty error messages. Now, useful error messages are reported.
  On windows no error was reported at all. The registry listening socket is now
  created with SO_EXCLUSIVEADDR on windows, which prevents mulitple registries
  to share the same listening port (AFTMAGT-303).


Compatibility with 3.3.7
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.3.7] - 2021-01-27
--------------------------------

Added
~~~~~
- Added replaying functionality for Ethernet, Can, GenericMessages and IO ports.
  This utilizes a new ReplayScheduler and replay controllers to inject replay
  messages from a trace file. The PCAP file format  is supported natively, and
  the MDF4 format is supported through the VIBE-MDF4Tracing extension.

Changed
~~~~~~~
- The semantics of the :cpp:func:`ComAdapter::Create*<ib::mw::IComAdapter::CreateCanController>`
  methods to create services has changed.
  Previously, invoking them multiple times would result in an exception.
  Now, the same pointer is always returned.
  The ReplayScheduler uses these methods to create and configure replay
  controllers for services internally.

Fixed
~~~~~
- When setting the ``struct ib::cfg::VAsio::Config::tcpQuickAck`` option, the
  TCP_QUICKACK socket option is now activated after each successful read()/recvmsg on
  Linux. This ensures that the default 40ms delayed acknowledge timeout is not
  used.

Compatibility with 3.3.6
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.3.6-QA] - 2020-12-10
--------------------------------

This is a Quality Assured Release.

Fixed
~~~~~
- Utility executables are now built with appropriate RPATHs on Linux.
- Remove -Wabi flags from gcc/clang builds, as they were not correctly used
  and they break the build on Ubuntu 20.04.
- Update the config schema to include the IbRegistry logger.

Compatibility with 3.3.5
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.3.5] - 2020-11-26
--------------------------------

Added
~~~~~~
- The number of connection attempts can now be configured in the VAsio registry
  configuration, see the "ConnectAttempts" field in :ref:`sec:mwcfg-vasio`.
- Added preliminary TCP/IP tuning settings to ``struct ib::cfg::VAsio::Config``. 
  The following settings are available:

  * tcpNoDelay: enable the TCP_NODELAY flag, which disables Nagle's algorithm (default off).
  * tcpQuickAck: enable the Linux specific TCP_QUICKACK, which disables delayed acknowledgements (default off).
  * tcpSendBufferSize and tcpReceiveBufferSize: set the TCP buffer sizes.

Changed
~~~~~~~
- The TCP_NODELAY is now off by default again. It can be enabled using the VAsio
  config.

Fixed
~~~~~
- The IbLauncher now considers debug libraries when searching for the VIB
  installation, this makes it usable with Debug builds.

Compatibility with 3.3.4
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No (due to Config changes)
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.3.4] - 2020-11-04
--------------------------------

Added
~~~~~~
- The CMake build system has new options to build with sanitizers:
  IB_ENABLE_ASAN and IB_ENABLE_UBSAN, to enable 'Address Sanitizer' and
  'Undefined Behavior Sanitizer', respectively.
  When active, the -fsanitize=... compile options and link options are enabled
  globally.
  This is currently only supported on GCC and Clang.
- Added replay message provider interfaces for the upcoming MDF replaying
  extension.

Fixed
~~~~~~
- Fixed undeclared variable use in IbLauncher (AFTMAGT 294).

Changed
~~~~~~~
- Added the TCP_NODELAY option to VAsio (AFTMAGT 297).
  This reduces latencies when sending a lot of small VIB messages.

Compatibility with 3.3.3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.3.3] - 2020-10-15
--------------------------------
Added
~~~~~~
- Added a new configuration format for replaying traces,
  refer to :ref:`sec:cfg-participant-replaying` for details.
  Please note that the replaying mechanism is still under development.

Changed
~~~~~~~
- The IbRegistry command line utility now supports a ``--use-signal-handler`` flag
  that prevents it from listening on stdin. It can be safely shut down with
  Control-C when started with this flag.

Compatibility with 3.3.2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Due to changes in the config API we are not ABI compatible.

- Application binary interface (ABI): No (due to Config)
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.3.2] - 2020-09-24
--------------------------------

Changed
~~~~~~~
- The duplicate IbConfig schema was removed from the Launcher subdirectory.
  There is now only one instance of IbConfig.schema.json under
  IntegrationBus/source/cfg/.
- Integration tests were refactored and stabilized (AFTMAGT 271).
  Networked integration tests now use a synchronized simulation, with a new test
  harness that simplifies test case setup.

Compatibility with 3.3.1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.3.1-QA] - 2020-08-27
--------------------------------

This is a Quality Assured Release.

Fixed
~~~~~
- Fixed config JSON schema and updated Demos to latest configuration syntax.
- Allow 'UseTraceSinks' when parsing configuration files in backward-compatibility
  mode.
- Make trace sink attachment deterministic per participant. This
  changes an internal interface used to load the VIBE mdf4tracing extension.
- Fix launcher when there is a trailing separator in the environment PATH
  variable.

Changed
~~~~~~~
- Building the documentation now requires Sphinx version >= 3.0 (cf. :doc:`../development/build`).
  
Compatibility with 3.3.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.3.0] - 2020-08-12
--------------------------------

Added
~~~~~
- Added support for tracing bus messages on the VIBE-NetworkSimulator (AFTMAGT 277).
- Enable tracing for GenericMessages (AFTMAGT 233).
- Enable tracing for IoPorts (AFTMAGT 276).
- Added support for tracing CAN/LIN/Ethernet bus messages on controller proxies (AFTMAGT 278).

Changed
~~~~~~~
- The NetworkSimulator configuration syntax was changed. The network simulator
  definition was moved from the SimulationSetup level, down to the participant
  that previously only referred to the network simulator by name.
  For backward compatibility the old configuration syntax is still supported,
  refer to :ref:`sec:cfg-network-simulators`.
  (AFTMAGT-277).

  + old:
    
    .. code-block:: javascript

       "SimulationSetup": {
           "Participants": [
                {
                    "Name": "NetworkSimulator",
                    "NetworkSimulators": [ "NetSim1" ]
                }
           ],
           "NetworkSimulators": [
                {
                    "Name" : "NetSim1",
                    "SimulatedLinks": [...]
                }
           ]
        } 

  + new:
    
    .. code-block:: javascript


       "SimulationSetup": {
           "Participants": [
                {
                    "Name": "NetworkSimulator",
                    "NetworkSimulators": [
                        {
                            "Name" : "NetSim1",
                            "SimulatedLinks": [...]
                        }
                    ]
                }
           ]
        } 
  + This change also affects the config builder API and ib::cfg::Config.



Compatibility with 3.2.1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The config builder API for network simulators was changed, and the struct
ib::cfg::Config was also modified.

- Application binary interface (ABI): No  (changes in Config)
- Application software interface (API): No (changes in Config)
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.2.1] - 2020-07-23
--------------------------------

Added
~~~~~

- The startup delay of a FlexRay node in the FlexRay demo can now be set programmatically.
- Added a message tracing section to :ref:`Participant Configuration<sec:cfg-participant-tracing>`.
  It briefly discusses how to configure the trace sink mechanism and how the 
  :ref:`mdf4tracing` extension is used.
  The controllers now also reflect the recently updated tracing support.

Changed
~~~~~~~

- Enabled message tracing on the following controllers: LIN, CAN, FlexRay.
- The Ethernet demo now uses the IEthController::SendFrame API.
- Updated version of the third-party library `fmt` to version `6.1.0`.

Fixed
~~~~~

- Fixed a bug in the FlexRay demo which caused unreachable code in the POC Handler.
- TraceSinkBuilder was missing API exports for three methods, resulting in
  missing symbols when linking on Windows.
- Fixed shared library loading incompatibilities.

Compatibility with 3.2.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No 
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.2.0] - 2020-07-06
--------------------------------

Added
~~~~~

- Logger of the VAsio Registry can now be configured via the middleware configuration,
  cf. :ref:`sec:mwcfg-vasio`. The corresponding :cpp:class:`RegistryBuilder<ib::cfg::VAsio::RegistryBuilder>`
  also gained the :cpp:func:`ConfigureLogger()<ib::cfg::VAsio::RegistryBuilder::ConfigureLogger>` method to configure
  the logger of the VAsio Registry.
- Added benchmark demo, cf. :ref:`sec:util-benchmark-demo`.

.. _sec:vib320-changed:

Changed
~~~~~~~

- :cpp:class:`ib::type_conversion_error<ib::type_conversion_error>` inherits now from 
  :cpp:class:`std::runtime_error<std::runtime_error>` instead of :cpp:class:`std::exception` directly.
- :cpp:class:`ib::cfg::LoggerBuilder<ib::cfg::LoggerBuilder>` doesn't inherit from
  :cpp:class:`ib::cfg::ParentBuilder<ib::cfg::ParentBuilder>` anymore.

Fixed
~~~~~
- Fixed a bug that prevented legacy IbConfigs (pre VIB 3.1.0) using the PcapFile
  setting from being converted to new IbConfigs (VIB 3.1.0) using trace sinks.


Compatibility with 3.1.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No (See :ref:`Changed<sec:vib320-changed>`)
- Application software interface (API): No (:ref:`LoggerBuilder API changed<sec:vib320-changed>`)

- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.1.0] - 2020-06-15
--------------------------------

Added
~~~~~
- New optional configuration section for extension-related settings,
  cf. :doc:`../configuration/extension-configuration`. Its only property
  is the list of extension search path hints, which allows to configure
  the additional search paths for shared library extensions loaded by the VIB,
  e.g. the :doc:`vibes/vibregistry`.

- New configuration mechanism for IB message tracing.
  It supersedes the previous Ethernet and PCAP specicic configuration, please
  refer to the Deprecated section.

- The ParticipantBuilder gained a new AddTraceSink() method, which returns
  a TraceSinkBuilder. A TraceSink consists of a unique, non-empty name, an output 
  path and the format type (PcapFile, PcapPipe, Mdf4File).

- Services, Controllers and Ports can be configured to use a trace sink by name.
  For example, by invoking the
  :cpp:func:`WithTraceSink(name)<ib::cfg::GenericPortBuilder::WithTraceSink()>`
  on the appropriate builder.
  This will populate the 'UseTraceSinks' field of the JSON serialization of the
  builder's configuration type.

Changed
~~~~~~~
- For FastRTPS, the default participant lease duration is now 2h to avoid
  connection losses when debugging. (AFTMAGT-267)

- To enable the newly added 'UseTraceSinks' fields, the JSON serialization
  format of the Controllers, Services and Ports were adjusted.
  In particular, the JSON type of DigitalIoPort, AnalogIoPort, PwmPort,
  PatternPort, and GenericSubscriber were changed:

  + old:

    .. code-block:: javascript

       "Port-Type": [ "PortName", "OtherPort", ...]

  + new:

    .. code-block:: javascript

       "Port-Type": [
            {
                "Name":  "PortName"
            },
            {
                "Name":  "OtherPort"
            }
       ]




Fixed
~~~~~
- :cpp:func:`ILinController::SendFrameHeader(LinIdT)<void
  ib::sim::lin::ILinController::SendFrameHeader(LinIdT)>` now correctly sets the
  current simulation time in the LinTransmission. Previously, the timestamp was
  always 0s.

- GenericSubscriber was missing the ITimeProvider interface.

.. _sec:api-withpcap-removed:

Removed
~~~~~~~
- The ControllerBuilder<EthernetController> no longer supports the
  WithPcapFile() and WithPcapPipe() methods.
  This usage has been superseded by the new configuration mechanism:

  + old:

    .. code-block:: c++

        simulationSetup
            .AddParticipant("P1")
            .AddEthernet("ETH1")
            .WithPcapFile("output filename");

  + new:

    .. code-block:: c++

        auto&& participant = simulationSetup.AddParticipant("P1");
        participant->AddEthernet("ETH1").WithTraceSink("EthSink");
        participant->AddTraceSink("EthSink")
            .WithType(TraceSink::Type::PcapFile)
            .WithOutputPath("output filename.pcap");



Deprecated
~~~~~~~~~~
- The 'pcapFile' and 'pcapPipe' fields in the EthernetController configuration 
  are deprecated. Please use the newly added 'UseTraceSinks' and 'TraceSinks' 
  fields. These fields will be removed from the JSON format and the Config
  Builder API in the future.

  + old:
    
    .. code-block:: javascript

       "EthernetControllers": [
           {
               "Name": "ETH0",
               "PcapFile": [ "EthernetReader.pcap" ]
           }
        ] 

  + new:
    
    .. code-block:: javascript

       "EthernetControllers": [
           {
               "Name": "ETH0",
               "UseTraceSinks": "EthernetSink"
           }
        ] 
        "TraceSinks": [
            {
                "Name" : "EthernetSink",
                "OutputPath": "EthernetReader.pcap",
                "Type": "PcapFile"
            }
        ]

- Loading a JSON file which contains the deprecated "PcapFile" or "PcapPipe"
  fields will cause a runtime warning. Internally the data structures  are updated
  as if a "TraceSinks" and "UseTraceSinks" was supplied with a TraceSink name 
  that is derived from the Participant's and EthernetController's names.

Compatibility with 3.0.7
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): No
- Application software interface (API): No (:ref:`ConfigBuilder API changed<sec:api-withpcap-removed>`)
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.0.7] - 2020-05-25
--------------------------------

Added
~~~~~
- Add a time provider interface internal to the service controllers.
  By default the wallclock time is used as the source for the current time.
  When a participant controller is present, its virtual simulation time is used.
- IEthController gained a new API for sending Ethernet frames with explicit and
  implicit timestamps:
  :cpp:func:`IEthController::SendFrame(EthFrame, nanoseconds)<EthTxId ib::sim::eth::IEthController::SendFrame(EthFrame, std::chrono::nanoseconds)>`
  and 
  :cpp:func:`IEthController::SendFrame(EthFrame)<EthTxId ib::sim::eth::IEthController::SendFrame(EthFrame)>`.

  These methods will support MDF4 tracing in the future.
  The controller's time provider will be queried if no user supplied timestamp
  is present.

- ParticipantControllers can now forcefully exit the run loop in case of an
  error:
  :cpp:func:`IParticipantController::ForceShutdown()<ib::mw::sync::IParticipantController::ForceShutdown()>`.
  Note that this method is only intended for use cases where a regular shut down
  is not possible!

Fixed
~~~~~
- Fixed a memory leak: there was a shared pointer cycle in SyncMaster's
  DiscreteTimeClient when attaching a lambda to itself.

Deprecated
~~~~~~~~~~
- Please note, that the :cpp:func:`IEthController::SendMessage(EthMessage)<EthTxId ib::sim::eth::IEthController::SendMessage(EthMessage)>`
  method is deprecated in favor of the new SendFrame() methods.
  It will be removed in the future.
  The EthMessage struct contains a user-settable timestamp, which is not a good
  fit for the new time provider based API.

Changed
~~~~~~~
- CMake build: the binaries are now all built in the
  ${CMAKE_BINARY_DIR}/$<CONFIG> directory. This allows running Demos and Tests
  directly from the build directory, which eases debugging.

Compatibility with 3.0.6
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.0.6] - 2020-04-30
--------------------------------

Fixed
~~~~~
- Disable remote logging when shutting down (AFTMAGT252)
  This fixes issues when remote logging is enabled with log levels of debug and
  higher. The FastRTPS middleware uses debug log messages internally, and during
  shutdown the LogMsg FastRTPS topic is unmatched and destroyed -- which leads
  to invalid accesses when remote logging is enabled.

- PCAP tracing now includes the ingress data on EthControllers (AFTMAGT265).

Changed
~~~~~~~

- We no longer bundle FastRTPS binaries in the official VIB packages.
  Users had issues using the exported cmake targets from FastRTPS binaries when
  building from source.
  The VIB cmake build system fetches FastRTPS using git when the FastRTPS
  depdendencies are missing from the local source tree.

Compatibility with 3.0.5
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.0.5] - 2020-04-08
--------------------------------

Added
~~~~~
- It is now possible to provide a time stamp for LIN transmissions. For this,
  overloads have been added to
  :cpp:func:`ILinController::SendFrame()<void ib::sim::lin::ILinController::SendFrame(Frame, FrameResponseType, std::chrono::nanoseconds)>`
  and
  :cpp:func:`ILinController::SendFrameHeader()<void ib::sim::lin::ILinController::SendFrameHeader(LinIdT, std::chrono::nanoseconds)>`.
  Note that this timestamp will be overwritten when using the VIBE NetworkSimulator.

- The VAsio registry can now be used as a shared library. Please note that the
  shared library is non-redistributable.
  The extension mechanism will load the shared library and construct an instance
  of the :cpp:class:`IIbRegistry` interface for the user to consume.
  The API entry point is the
  :cpp:func:`CreateIbRegistry()<ib::extensions::CreateIbRegistry>` function.
  The vib-registry shared library needs to reside in the current process's 
  working directory.
  Initially, it is located in the ``IntegrationBus-NonRedistributable``
  subdirectory of the VIB package.
  In case of error a std::runtime_error is thrown.

Changed
~~~~~~~
- Don't format Logger messages if the messages aren't going to be
  logged anyways.

Compatibility with 3.0.4
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.0.4] - 2020-03-19
--------------------------------

Added
~~~~~
- A new FlexRay controller API is introduced for monitoring protocol operation
  control (POC) status changes, cf. :ref:`sec:poc-status-changes`.
  This new API supersedes the  ControllerStatusHandler mechanism, as current use
  cases and new ones are covered by the PocStatus handler.
  This new API exposes more status variables of the POC when using the VIBE
  Network Simulator for FlexRay simulation. (AFTMAGT-253)

  .. admonition:: Note

     To ensure interoperability you should use VIBE Network Simulator v3.0.4
     in all setups involving different, but compatible versions of VIB.
     See compatibility below for details.

Changed
~~~~~~~
- Clarify error messages on connection loss for VAsio. The previous term
  "Shutdown" was ambiguous, the error reason now states "Connection
  lost" (AFTMAGT-260).
- Disable problematic FastRTPS connection loss detection (AFTMAGT-259).


Deprecated
~~~~~~~~~~
- :cpp:func:`IFrController::RegisterControllerStatusHandler()<ib::sim::fr::IFrController::RegisterControllerStatusHandler()>`
  is now deprecated in favor of
  :cpp:func:`RegisterPocStatusHandler()<ib::sim::fr::IFrController::RegisterPocStatusHandler()>`.
  ControllerStatusHandler will be removed in a future release.
  The usage of RegisterControllerStatusHandler will result in a warning
  at runtime and compile time.


Fixed
~~~~~
- The IbRegistry can now be used for multiple simulation runs without the need
  to terminate and restart it (AFTMAGT-249).
- The internally used spdlog is now build with compiler flags that prevent
  creation of weak symbols on Linux / GCC (AFTMAGT-256).


Compatibility with 3.0.3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Please note that the new FlexRay controller model (VIB v3.0.4) requires the
matching VIBE Network Simulator v3.0.4, even when not using the new PocStatus
API. However, the VIBE Network Simulator v3.0.4 is fully compatible with
previous FlexRay controller models and enables interoperability between VIB
v3.0.3 and v3.0.4 participants.

- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.0.3] - 2020-02-26
--------------------------------

Added
~~~~~
- New integration test ensuring that VAsio is fully deterministic and delivers messages strictly in-order.
  This test replaces the demo GenericMessageITest, which is now obsolete and has been removed for this reason.

Fixed
~~~~~
- ib::version::Patch() was set to a wrong value.

Removed
~~~~~~~
- Removed demo GenericMessageITest.

Compatibility with 3.0.2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.0.2-QA] - 2020-02-10
--------------------------------

This is a Quality Assured Release.

Added
~~~~~
- Documentation for the CAN controller API: :doc:`CAN Vehicle Network Controllers <api/can>`.
- Documentation for the Participant Controller API: :doc:`api/participantcontroller` (AFTMAGT-206).
- Documentation for the IO Port services: :doc:`api/io` (AFTMAGT-201).
- Documented Generic Messages API: :doc:`api/genericmessage` (AFTMAGT-204).
- Documented the simulation state machine and synchronization types: :doc:`simulation/simulation`
- Added docs for the ComAdapter: :doc:`api/comadapter`
- Added quick start guide: :doc:`usage/quickstart`
- Elaborate the user APIs and overview pages: :doc:`api/api`
- Add docs for :doc:`api/systemcontroller` (AFTMAGT-242).
- Add docs for :doc:`api/systemmonitor` (AFTMAGT-242).
- Add docs for :doc:`api/ethernet` (AFTMAGT-239).

Changed
~~~~~~~
- Removed the upper limit of the VAsio send Queue, to avoid that critical
  IbMessages are thrown away. (AFTMAGT-240)
- VIB Utilities are now distributed in Release build configuration (AFTMAGT-245)
- The interactive system controller has been promoted from demo to
  utility. I.e., it is now part of the binary delivery.

Fixed
~~~~~
- The LIN controller now only calls the goToSleepHandler if the frame id and
  also the data field of a received LIN frame matches the id and data of a valid
  "GoToSleep" frame (AFTMAGT-244).
- VIB applications built in RelWithDebInfo or MinSizeRel will no longer link
  against the VIB Debug installation, which crashes under windows. (AFTMAGT-246)
- Fixed compilation errors caused by windows.h, which broke std::min/max calls
  in VIB headers. (AFTMAGT-248)
- Fixed a crash when move assigning the ConfigBuilder. (AFTMAGT-24)
- Fixed the cmake exported targets. CMake users should be able to use
  find_package(CONFIG) to integrate with the IntegrationBus library.
- Fixed a bug that prevented the CAN controller state callback from being called
  when using VIBE NetworkSimulator.
- Removed a false warning for VAsio with syncType::DistributedTimeQuantum, which
  incorrectly informed a user that IParticipantController::setPeriod() has no
  effect for this syncType.

Compatibility with 3.0.1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes


[3.0.1] - 2020-01-08
--------------------------------

Added
~~~~~
- New config parameter "HistoryDepth" added to FastRTPS config section. This value is used to
  set the history size for all FastRTPS topics.
- New WithHistoryDepth method for FastRtpsConfigBuilder. When using the builder pattern to
  generate an Ib Config, the new FastRTPS HistoryDepth can be configured this way.
- New documentation for the configuration mechanism, cf. :doc:`../configuration/configuration`
- New documentation for FastRTPS configuration, cf. :doc:`../configuration/middleware-configuration`
- Extend the simulation setup documentation, cf. :doc:`../configuration/simulation-setup`

Changed
~~~~~~~
- The IbLauncher CANoe environment module was adapted to the new CANoe Extensions packaging format.

Fixed
~~~~~
- A check was added to prevent participants from using ParticipantID 0, since this ID is reserved for the Registry when VAsio is used.

Compatibility with 3.0.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes

[3.0.0] - 2019-12-03
--------------------------------
Added
~~~~~
- The logging mechanism now includes the most important elements of sent and received IB messages (AFTMAGT-217).

Changed
~~~~~~~
- The VIB distribution uses a new directory layout for packages:

  IntegrationBus
    Contains the pre-built binary distribution of the IntegrationBus, including the C++ header files and CMake export targets.
    
  IntegrationBus-Demos
    The VIB demos are now distributed in source form.
    They can be easily compiled against the distributed VIB binaries: e.g, on Windows just right-click and "open in visual studio"
    
  IntegrationBus-Documentation
    Contains the documentation in HTML and text format.
    
  IntegrationBus-Source
     The VIB source tree, with notable changes:
     
     - The SystemController and SystemMonitor demos were moved to the Utilities/ directory. 
       They are also distributed in binary form.
     - Fast-RTPS is no longer distributed in source form. 
       The CMake build infrastructure automatically downloads a git snapshot if needed (requires git).

- CMake packaging was simplified (AFTMAGT-195).
- A notification is shown when writing to a PCAP pipe is enabled, as the default behavior is to block until the pipe is read by another process (AFTMAGT-221).
- Logging output to std::cerr and std::cout is replaced by calls to the internal logging mechanism (AFTMAGT-210).
- Warn user when a PCAP pipe is opened (AFTMAGT-221).
       
  
Fixed
~~~~~
- Fixed the Participant subscription in VASio (AFTMAGT-216). Creating an IB service will now block until all the necessary subscriptions have been acknowledged by all known participants.

Compatibility with 2.0.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No
- Application software interface (API): No
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): No

[2.0.0] - 2019-11-06
--------------------------------
Added
~~~~~
- New watchdog functionality for SimTasks with soft and hard limits. Whenever a
  SimTask runs longer than the soft limit, a warning is logged. If it runs
  longer than the hard limit, the participant switches to the error state.
- New config section for ParticipantControllers. Here, you can specify the execution time limits for SimTasks and specify the SyncType. E.g.,
  
    .. code-block:: javascript
                    
       "ParticipantController": {
           "SyncType": "DiscreteTime",
           "ExecTimeLimitSoftMs": 1010,
           "ExecTimeLimitHardMs": 1500
       }
   
- New ParticipantControllerBuilder. When using the builder pattern to generate
  an Ib Config, the ParticipantController can be configured via
  ParticipantBuilder::AddParticipantController().

- New Ethernet Trace Logging in the PCAP Format. Trace logs can be written to files
  or named pipes.
- New config parameters are added for EthernetControllers. Here, you can specify
  the PCAP trace filename or the name of the pipe. E.g.:
  
    .. code-block:: javascript

       "EthernetControllers": [
           {
               "Name": "ETH0",
               "MacAddr": "F6:04:68:71:AA:C2",
               "PcapFile": "EthernetReader.pcap",
               "PcapPipe": "EthernetReaderPipe"
           }
       ]
- Added WithPcapFile and WithPcapPipe methods for ControllerBuilder<EthernetController>.
  When using the builder pattern to generate an Ib Config, PCAP tracing can be configured
  with the new methods.

Changed
~~~~~~~
- Transitions from the shutdown state to the error state are no longer allowed.
  With the recently added connection loss detection, participants could also
  enter the error state after a normal shutdown, which is now prevented.

- Implemented new versioning schema. As of now, the following semantic
  versioning schema is applied:
  
  + major number changes indicate breaks on a network layer
  + minor number changes indicate API of config format breaks
  + patch number changes indicate any other non-breaking changes.

- Added a canId field to the CanTransmitAcknowledge data type. This was required
  for a bug fix and is a breaking change on the network layer.
  
- Added a sourceMac field to the EthTransmitAcknowledge data type. This was
  required for a bug fix and is breaking change on the network layer.
  
Fixed
~~~~~
- CAN controllers now only call the TransmitStatusHandler if they did send the
  corresponding CAN message. Previously, in a simulation with more than two CAN
  controllers, the callback could be triggered without having sent a message.

- Ethernet controllers now only call the MessageAckHandler if they did send the
  corresponding ethernet message. Previously, in a simulation with more than two
  ethernet controllers, the callback could be triggered without having sent a
  message.

Deprecated
~~~~~~~~~~~~~~
- The Participant config setting SyncType has been deprecated. The SyncType is
  now configured in the ParticipantController section.

  + old:
    
    .. code-block:: javascript
                    
       "SyncType": "DiscreteTime"

  + new:
  
    .. code-block:: javascript
                    
       "ParticipantController": {
           "SyncType": "DiscreteTime"
       }

- The SyncType::Unsynchronized is no longer used. Only participants with a
  ParticipantController configuration are synchronized. I.e., you can simple
  remove the "SyncType": "Unsynchronized" from SystemMonitors.
   
- The ParticipantBuilder::WithSyncType() has been deprecated. Use
  ParticipantConfigBuilder::WithSyncType() instead.

  + old:
    
    .. code-block:: c++
                    
      simulationSetup.AddParticipant("P1")
          .WithSyncType(SyncType::DiscreteTime);
    
  + new:
    
    .. code-block:: c++
                    
      simulationSetup.AddParticipant("P1")
          .AddParticipantController().WithSyncType(SyncType::DiscreteTime);


Compatibility with 1.1.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Partially (Everything except for CAN and Ethernet is compatible)
- Middleware network protocol (VAsio): Partially (Everything except CAN and Ethernet is compatible)


[1.1.0] - 2019-09-16
--------------------------------
Added
~~~~~
- Added a connection loss mechanism in FastRTPS and VAsio middleware. Lossing the
  connection of one participant will lead the system to go into Error state.
- When logging at trace level, a log entry is now written for each incoming and
  outgoing IbMessage.
- When logging at trace level, the wait times and execution times per tick are
  now logged.
- The creation of a ComAdapter is now logged. The log entry includes the used
  VIB version.
- Connection losses during the simulation are now detected. The are reported as
  an updated ParticipantStatus with State Error and a note that the connection
  was lost.

Fixed
~~~~~
- Fixed FlexRay parameter validation, which could cause valid parameter sets to
  be rejected. E.g., gdSymbolWindows has a valid range from 0 to 162, but we
  check for 1 to 139.
- Fixed a racecondition when starting up a VAsio simulation. As VAsio does not
  have a history, it could happen that some participants did not receive all
  ParticipantStatus values.

.. _changelog:1.0.0_removed:

Removed
~~~~~~~
- IComAdapter::RegisterNewPeerCallback() was removed. This method was only
  intended as an IB-internal helper method and never officially announced as
  part of the public API.
  
Compatibility with 1.0.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No
- Application software interface (API): No (cf. :ref:`Removed<changelog:1.0.0_removed>`)
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): Yes
  

[1.0.0-QA] - 2019-09-25
--------------------------------

This is a Quality Assured Release.

Added
~~~~~
- The logging facilities can now be configured per participant using the IB
  config. E.g., different sinks and log levels can be configured.

Changed
~~~~~~~
- The LIN API was redesigned to provide a clearer and simpler interface. To make
  the transition to the new API as simple as possible, we provided extensive
  documentation on the new API itself including usage examples and information
  about what changed in the new API: :doc:`../api/lin`
- Removed spdlog from the public IB API. Spdlog is still used internally but it
  has been removed from the public API to avoid conflicts with user specific
  spdlog installations.
- The CMake build options BUILD_TESTS and BUILD_DOCS were renamed to
  IB_BUILD_TESTS and IB_BUILD_DOCS.
- The Tools folder has been renamed to Utilities to differentiate it more
  clearly from build tools. For the time being, the IbRegistry is the only
  utility.

Fixed
~~~~~
- Fixed a crash in the IbLauncher when the IbConfig did not specify a
  MiddlewareConfig or an ActiveMiddleware.
- Fixed a crash when creating a ComAdapter with the same participant name as a
  previously destroyed one. The crash originated in spdlog.
- Fixed a crash in the VAsioConnection destructor due to a wrong member order.
- Fixed the Fast-RTPS submodule from v1.7.0 to v1.8.1, which got broken during a
  merge.
- The old, unmaintained CHANGELOG.md is no longer installed. Instead, the
  CHANGELOG.rst is installed in addition to the HTML documentation.


Compatibility with Sprint-31
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No
- Application software interface (API): No
- Middleware network protocol (FastRTPS): Partially (Everything except LIN is compatible)
- Middleware network protocol (VAsio): Partially (Everything except LIN is compatible)


[Sprint-31] - 2019-08-14
------------------------
Added
~~~~~
- New VAsio middleware as an alternative to Fast-RTPS, the VAsio middleware was
  specifically developped for the integration bus to provide high performance and
  stability. Cf. :doc:`../configuration/middleware-configuration`.

Changed
~~~~~~~
- Upgrade Fast-RTPS to version v1.8.1. This improves stability on Linux.

Fixed
~~~~~
- Fixed a crash in the IbLauncher when the IbConfig did not specify a
  MiddlewareConfig or an ActiveMiddleware.


Compatibility with Sprint-30
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No
- Application software interface (API): Yes
- Middleware network protocol (FastRTPS): Yes
- Middleware network protocol (VAsio): No


[Sprint-30] - 2019-07-31
------------------------
Fixed
~~~~~
- Attempting to create a ComAdapter with an empty name will now throw
  a misconfiguration exception with a proper error message.


Interface compatibility with Sprint-29
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Application middleware interface: Yes


[Sprint-29] - 2019-07-17
------------------------
Added
~~~~~
- It is now possible to reconfigure FlexRay TX-Buffers during the simulation, e.g., to change offset
  and repetition. Cf. :cpp:func:`IFrController::ReconfigureTxBuffer()<ib::sim::fr::IFrController::ReconfigureTxBuffer()>`

Changed
~~~~~~~
- This is the last entry to CHANGELOG.md. From now on, the changelog
  will be maintained in docs/CHANGELOG.rst.
- The IB API Headers are no longer added to every project. Instead, a dedicated
  header project IbApi has been added.
- The IbLauncher now prefers Python 3 if available

Fixed
~~~~~
- Fix logger nullptr bug in SystemMonitor
- Fast-RTPS ComAdapter creation is now thread safe

Interface compatibility with Sprint-28
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No
- Application software interface (API): Yes
- Application middleware interface: Yes


[Sprint-28] - 2019-07-03
-------------------------

Added
~~~~~
- New demo that shows how integration tests can be written for the Vector Integration Bus.

Fixed
~~~~~
- IbLauncher can now be started from every directory location on Linux and Windows. The global
  IntegrationBus-BinPath and IntegrationBus-LibPath are now set to absolute paths inside the
  IbLauncher project. Furthermore, the IbLauncher shell script now sets absolute paths for the bin
  and lib path.
- FastRTPS socket buffer sizes now use default values when not set in IbConfig. This could lead to
  random socket buffer sizes in release builds.
- VIB integration tests now can be launched directly from the Visual Studio test runner.

Interface compatibility with Sprint-27:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): Yes
- Application software interface (API): Yes
- Application middleware interface: Yes


[Sprint-27] - 2019-06-19
------------------------

Changed
~~~~~~~
- NetworkSimulator VIBE is now only used for configured links. For all other links,
  the trivial simulation is used.

Fixed
~~~~~
- The IB Launcher will now work if installed in a path containing spaces.
- The FlexRay configuration will now use strings to represent the enumeration values of pChannels,
  pWakeupChannel, and pdMicrotick, as well as channels and transmissionMode for the TxBuffers. The
  new valid values are:

  * Channels: "A", B", or "AB"
  * pdMicrotick: "12.5ns", "25ns", or "50ns"
  * transmissionMode: "Continuous" or "SingleShot"

Interface compatibility with previous version:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Application binary interface (ABI): No
- Application software interface (API): Yes
- Application middleware interface: Yes


[Sprint-26] - 2019-05-29
------------------------

Added
~~~~~

- New FlexRay controller callback
  :cpp:type:`IFrController::CycleStartHandler()<ib::sim::fr::IFrController::CycleStartHandler>`,
  which is called at the start of each FlexRay cycle. Only available
  VIBE simulation.
- New config option for FastRTPS middleware to configure SocketBuffer sizes
- New config options to configure FlexRay TxBuffers

Fixed
~~~~~

- Fixed broken CMake target for installed IntegrationBus target:
  With the introduction of spdlog, the IntegrationBus cmake target depends on spdlog::spdlog target.
  However, the spdlog::spdlog target was not installed (only header files were copied, but no cmake
  config file was generated). Thus, the installed IntegrationBus target could not be used from cmake.

[Sprint-25] - 2019-05-14
------------------------

Added
~~~~~

- The FlexRay cluster and node configuration can now be specified in the IbConfig.json.
- It is now possible to use
  :cpp:func:`SetResponse()<ib::sim::lin::ILinController::SetResponse()>`
  and
  :cpp:func:`RequestMessage()<ib::sim::lin::ILinController::RequestMessage()>`
  on LIN :cpp:class:`Controllers<ib::sim::lin::ILinController>` configured as master. This can be used to send a
  LIN message from a master to slaves instead of the
  :cpp:func:`SendMessage()<ib::sim::lin::ILinController::SendMessage>`.

Fixed
~~~~~

- LIN Controller models are now robust to out-of-order configuration. It is no longer necessary to
  configure the master node before the slaves. And the master can handle an out-of-order
  configuration, e.g., if a response reaches the master before the response configuration.


[Sprint-24] - 2019-03-13
------------------------

Added
~~~~~

- Support to swap out participants between simulation runs. A participant can activate the so called
  coldswap feature by calling
  :cpp:func:`IParticipantController::EnableColdswap()<ib::mw::sync::IParticipantController::EnableColdswap()>`.
  The coldswap process can be initiated by a system controller once the system is in state stopped.
- Participants can now signal that they are alive by refreshing the participant status. This can be
  done by calling
  :cpp:func:`IParticipantController::RefreshStatus()<ib::mw::sync::IParticipantController::RefreshStatus()>` and
  is reflected in the new field ParticipantStatus::refreshTime.
- Logging is finally here. We've integrated spdlog and enabled distributed logging with a new spdlog
  sink. The FastRtpsComAdapter automatically creates an spdlogger with this sink. You can access
  this logger via :cpp:func:`IComAdapter::GetLogger()<ib::mw::IComAdapter::GetLogger()>` and add
  any further sinks, e.g., to print logging messages to std out. Examples for this can be found in
  the CAN demo and in the PassiveSystemMonitor.

Changed
~~~~~~~

- The signature of simulation tasks has changed from void(std::chrono::nanoseconds now) to
  void(std::chrono::nanoseconds now, std::chrono::nanoseconds duration). The guaranteed simulation
  time that can be processed is [now, now+duration). The old signature is still available but is now
  considered deprecated and will be removed in a future sprint.


[Sprint-23] - 2019-02-20
------------------------

[Sprint-22] - 2019-02-06
------------------------

Added
~~~~~

- The IbConfig is now validated before creating a ComAdapter. NB: ib::CreateFastRtpsComAdapter()
  will now also throw Misconfiguration exceptions!

Fixed
~~~~~

- The IbLauncher now correctly shows both stdout and stderr. Previously, only stdout was shown.


[Sprint-21] - 2019-01-23
------------------------

Added
~~~~~

- Strict sync, i.e., calling wait_for_all_acked() between ticks, is now
  configurable via the IbConfig.json: SimulationSetup/TimeSync/SyncPolicy.
- If a participant is configured as SyncMaster, the corresponding
  ComAdapter will now automatically create the SyncMaster instance.

Removed
~~~~~~~

- SimulationSetup/TimeSync/SyncType has been removed
  from the IbConfig.json as the SyncType can now be configured per participant.
- IParticipantController::EnableStrictSync() has been removed.
  This is now handled automatically according to the configured SyncPolicy.

Changed
~~~~~~~

- The TimeSyncConfigBuilder is now accessed ib::cfg::SimulationSetupBuilder::ConfigureTimeSync().
  Old: ib::cfg::SimulationSetupBuilder::SetSyncType(SyncType).
- ComAdapter is now configured automatically according to SyncPolicy. Only in
  strict mode, wait_for_all_acked() is used and a short heartbeat period is used.
- IComAdapter::CreateSyncMaster() was renamed to IComAdapter::GetSyncMaster() since the
  SyncMaster is automatically instantiated by the FastRtpsComAdapter if configured.

Fixed
~~~~~

- SystemMonitor was made more robust to race conditions that could lead to a IB
  Startup Failure (SystemState stuck in SystemState::Invalid)


[Sprint-20] - 2018-12-19
------------------------

Added
~~~~~

- LIN: new method ILinController::SetResponseWithChecksum() to override the
  configured checksum model. This can be used to facilitate fault injection and
  simulation.
- LIN: support for LIN network management (sleep / wakeup). See new methods
  ILinController methods: SetSleepMode(), SetOperationalMode(), SendGoToSleep(),
  SendWakeupRequest() and related callbacks.

Removed
~~~~~~~

- Demo projects ExecutionController and ExecutionControllerProxy were based on
  the deprecated synchronization API and were Removed

Changed
~~~~~~~

- LIN: IMPORTANT you must now specify a LIN ID for each lin::SlaveResponseConfig.
  lin::SlaveConfig now longer identifies the lin::SlaveResponses by positions.
- All demo projects Can, Lin, Ethernet, FlexRay, GenericMessage and Io now use
  the new synchronization API.
- Revised public API:

  - Renamed files: IoDataTypes.hpp -> IoDatatypes.hpp
  - Fixed inconsistent naming:

    - IComAdapter.hpp: RegisterCanBusSimulator -> IComAdapter::RegisterCanSimulator
    - CanDatatypes.hpp: Removed 'e' prefix from enum classes CanControllerState, CanErrorState, CanTransmitStatus
    - EthDatatypes.hpp: Removed 'e' prefix from enum classes EthTransmitStatus, EthState, EthMode
    - IEthController.hpp: Renamed 'acticate()', 'deacticate()' -> 'Acticate()', 'Deacticate()'
    - ISyncAdapterTtd.hpp: Renamed SetOnTickCallback -> SetTickHandler
    - IGenericSubscriber.hpp: Renamed RegisterCallback -> SetReceiveMessageHandler
    - ISyncMasterDt.hpp: Renamed RegisterShutdownHandler -> SetShutdownHandler

  - Moved generic messages into subnamespace: ib::sim -> ib::sim::generic
  - Made include namespaces reflect folder names:
    ib::Simulation::Can|Ethernet|Flexray|Generic|Io|Lin|Kernels -> ib::sim::can|eth|fr|generic|io|lin|kernels


[Sprint-19] - 2018-12-05
------------------------

Added
~~~~~

- New unified SyncMaster that replaces the SyncMasterDt and
  SyncMasterTtd. Requires new state handling with ParticipantController
- The participant discovery mechanism can now be configured in the
  IbConfig.json, section "MiddlewareConfig/FastRTPS/DiscoveryType". The default
  is Local, which limits communication to the localhost.
- The build number of the master branches CD build is now available as
  ib::version::BuildNumber()
- On windows, version information is now available as metadata of the
  IntegrationBus.dll

Removed
~~~~~~~

- "MiddlewareConfig/FastRTPS/CommunicationMaster" has been removed
  and replaced with the new DiscoveryType options.

Changed
~~~~~~~

- SyncMasterDt and SyncMasterTtd are now considered deprecated.

Fixed
~~~~~

- Unicast discovery is now working with DiscoveryType Unicast and a list of the
  participants' IP-Addresses.


[Sprint-18] - 2018-11-21
------------------------

Added
~~~~~

- New IbLauncher to startup a whole IB System. See /Launcher/README.md for infos
  on how to use (AFTMAGT-120)
- Version information is now available in the IB library via the following API
  calls: ib::version::Major(), ib::version::Minor(), ib::version::Patch(),
  ib::version::String(), ib::version::SprintNumber(), ib::version::SprintName(),
  ib::version::GitHash() (AFTMAGT-154)

Removed
~~~~~~~

- LinkId has been removed from CAN, LIN, FlexRay and Ethernet data types, as
  they are no longer needed (see below).

Changed
~~~~~~~

- LIN masters now directly store slave responses to answer any request without
  delay. Instead of emulating LIN communication over the IB, LIN slaves now send
  newly configured response data to masters. (AFTMAGT-155) NB: calling
  ILinController::RequestMessage() will now trigger a callback to the registered
  MessageHandler before RequestMessage() returns!
- There is now one FastRTPS topic per link. I.e., if there are two CAN busses
  CAN1 and CAN2 in your configuration, they will now use separate
  topics. Previously, traffic of different links (busses) was separated by a
  linkid field in the message data types and controllers had to filter out
  messages accordingly. This is no longer necessary. (AFTMAGT-140)


[Sprint-17] - 2018-11-07
------------------------

Added
~~~~~

- New state handling, which is provided by the following classes:

-- sync::IParticipantController (cf. IComAdapter::GetParticipantController())
   allows registering callbacks for the different phases of a participant's life
   time (e.g., Initialization, Running, Stop, Shutdown) and replaces the old
   sync::ISyncAdapterTtd and sync::ISyncAdapterDt.

-- sync::ISystemMonitor (cf. IComAdapter::GetSystemMonitor()) is a passive
   component, which never sends data, it allows registering callbacks to observe
   the states of the other participants as well as the global system state.

-- sync::ISystemController (cf. IComAdapter::GetSystemController()) is the
   counterpart to the system monitor and allows manipulating the system state,
   e.g., by initializing individual participants.

-- sync::ISyncMaster (cf. IComAdapter::CreateSyncMaster()) a unified synchronization master
   (currently only supports simple Tick/TickDone synchronization).

-- Participants can now inidividually specify one of the following synchronization mechanisms:

  - DiscreteEvent (not implemented yet)
  - TimeQuantum (Quantum Request and Grant with variable quantum lengths)
  - DiscreteTime (Fixed interval synchronization with participant acknowledgement (Tick/TickDone))
  - DiscreteTimePassive (the participant receives Ticks but does not generate
    TickDone messages and thus does not actively participate in the system
    synchronization), Unsychronized (for participants that are only intended to
    monitor or control the simulation but not participante as an active client)

- GenericMessage configuration is now available at both IGenericPublisher and
  IGenericSubscriber (AFTMAGT-137)

Changed
~~~~~~~

- The old state handling including the old SyncAdapters is now considered deprecated!
  This affects the following classes: sync::ISyncAdapterDt, sync::ISyncAdapterTtd,
  sync::ISyncMasterTtd, sync::ISyncMasterDt, sync::IExecutionControllerProxy.
  And the matching factory methods: IComAdapter::CreateSyncAdapterDt(),
  IComAdapter::CreateSyncAdapterTtd(), IComAdapter::CreateSyncMasterTtd(),
  IComAdapter::CreateSyncMasterDt(), IComAdapter::CreateExecutionControllerProxy().
- The SyncType can now be configured per participant (Currently only DiscreteTime supported)


[Sprint-15] - 2018-10-10
------------------------

Added
~~~~~

- Documentation for throwing behavior at API level (AFTMAGT-50)
- Doxygen documentation for vehicle network controller APIs (AFTMAGT-126)
- Automated CI build system for Jenkins CI, cf. folder /IntegrationBus/ci/ (AFTMAGT-55)
- Support for custom FastRTPS XML configurations; the file name can be
  specified in the ib config (Config.middlewareConfig.fastRtps) (AFTMAGT-138)

Fixed
~~~~~

- Fixed a bug that caused the FastRTPS communication to sporadically hang (AFTMAGT-126)
- The number of links for IO ports is no longer limited (AFTMAGT-134)


[Sprint-14] - 2018-09-27
------------------------

Added
~~~~~

- GenericPublisher::Config() and GenericSubscriber::Config() accessors for corresponding config items. This allows retrieving the name of a Publisher or Subscriber (AFTMAGT-125)
- ib::cfg::Config::ToJsonString() converts an integration bus config to a parsable json string.
- CMake install target for the IntegrationBus
- The generic message demo now uses time synchronization, i.e., an ExecutionController is required to run the demo

Changed
~~~~~~~

- ExecutionController demo now terminates automatically unless started with --waitForKeyPress

Fixed
~~~~~

- The byte order of mac addresses in ethernet frames was fixed


[Sprint-13] - 2018-09-12
------------------------

Added
~~~~~

- Changelog :)
- ib::cfg::ConfigBuilder to create IbConfigs programatically, cf. example in Demo/ConfigBuilder/ConfigBuilderDemo.cpp.
- Support for Continous Integration (CI) with Jenkins, cf. /ci/
- IO ports can now be specified and initialized via the IntegrationBus config.
- Support for multiple GenericMessage instances (specified via IbConfig.json)
- Added new top-level section 'MiddlewareConfig' to JSON file for settings FastRTPS/CommunicationMaster and FastRTPS/ConfigFileName; moved existing configuration tree into top-level section 'SimulationSetup'.
- FastRTPS version bump from v1.5.0 to v1.6.0. FastRTPS is now included as a sub module

Removed
~~~~~~~

- ib::sim::IGenericMessageController()
- ib::mw::IComAdapter::CreateGenericMessageController()

Changed
~~~~~~~

- Moved config headers to /include/ib/cfg to match namespace, i.e.,
  /include/ib/Config.hpp moved to /include/ib/cfg/Config.hpp.
- IoPorts are now type specific with direction. IComAdapter::CreateIoPort() has been replaced with: IComAdapter::CreateAnalog{In,Out}(), IComAdapter::CreateDigital{In,Out}(), IComAdapter::CreatePattern{In,Out}(), IComAdapter::CreatePwm{In,Out}()
- IComAdapter::CreateGenericMessageController() has been replaced by IComAdapter::CreateGenericPublisher() and IComAdapter::CreateGenericSubscriber(), cf. updated demo Demo/GenericMessage/
