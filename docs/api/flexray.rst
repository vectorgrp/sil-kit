===================
FlexRay Service API
===================


.. contents::
   :local:
   :depth: 3


.. highlight:: cpp

Using the FlexRay Controller
------------------------------------

Initialization
~~~~~~~~~~~~~~~~~~~~

Before the FlexRay controller can be used and participate in the FlexRay communication cycles,
it must be configured, and then a Startup phase must take place at the beginning of the simulation.

Configuration
_______________________________________

The configuration is performed by setting up a :cpp:class:`ControllerConfig<ib::sim::fr::ControllerConfig>` and passing it to
:cpp:func:`IFrController::Configure()<ib::sim::fr::IFrController::Configure>`. Furthermore,
:cpp:func:`IFrController::Configure()<ib::sim::fr::IFrController::Configure>` switches the controller
to :cpp:enumerator:`PocState::Ready<ib::sim::fr::Ready>` signaling that it is ready for startup.

The :cpp:class:`ControllerConfig<ib::sim::fr::ControllerConfig>` consists of global
:cpp:class:`ClusterParameters<ib::sim::fr::ClusterParameters>` and node-specific
:cpp:class:`NodeParameters<ib::sim::fr::NodeParameters>`, which are both best set
in the JSON config (see config section :ref:`FlexRayControllers<sec:cfg-participant-flexray>`).
Furthermore, the :cpp:class:`ControllerConfig<ib::sim::fr::ControllerConfig>`
contains one or more :cpp:class:`TxBufferConfigs<ib::sim::fr::TxBufferConfig>`,
which can either be specified in the JSON config or added manually at
runtime. TxBuffers are used to initiate a transmission from one FlexRay
controller to another.

The following example configures a FlexRay controller with two
:cpp:class:`TxBufferConfigs<ib::sim::fr::TxBufferConfig>` specifying two
:cpp:class:`FrMessages<ib::sim::fr::FrMessage>`, which will be sent during simulation. The 
:cpp:class:`ClusterParameters<ib::sim::fr::ClusterParameters>` and the
:cpp:class:`NodeParameters<ib::sim::fr::NodeParameters>` are assumed to be set in the JSON config::

    std::vector<TxBufferConfig> bufferConfigs;
    TxBufferConfig txConfig;
    txConfig.channels = Channel::AB;
    txConfig.slotId = 10;
    txConfig.offset = 0;
    txConfig.repetition = 1;
    txConfig.hasPayloadPreambleIndicator = false;
    txConfig.headerCrc = 5;
    txConfig.transmissionMode = TransmissionMode::SingleShot;
    bufferConfigs.push_back(txConfig);

    txConfig.channels = Channel::A;
    txConfig.slotId = 20;
    bufferConfigs.push_back(txConfig);

    ControllerConfig controllerConfig;
    controllerConfig.bufferConfigs = bufferConfigs;
    controllerConfig.clusterParams = participantConfig.flexrayControllers[0].clusterParameters;
    controllerConfig.nodeParams = participantConfig.flexrayControllers[0].nodeParameters;

    flexRayController->Configure(controllerConfig);

Note that :cpp:func:`IFrController::Configure()<ib::sim::fr::IFrController::Configure>`
should be called in the InitHandler of a ParticipantController.

Startup
_______________________________________

At least two FlexRay controllers are always required for a successful startup in a FlexRay cluster.
The two participants responsible for startup are also called coldstart nodes. The "leading"
coldstart node (normally the first node that is in :cpp:enumerator:`PocState::Ready<ib::sim::fr::Ready>`)
has to send the :cpp:func:`IFrController::Wakeup()<ib::sim::fr::IFrController::Wakeup>` command
to the other "following" coldstart node(s)::

  leadingColdStartNode->Wakeup();
  // The leading controllers PocState will change from
  // Ready to Wakeup triggering the PocStatusHandler.

The response of the following cold startnode must be the
:cpp:func:`IFrController::AllowColdstart()<ib::sim::fr::IFrController::AllowColdstart>` and 
:cpp:func:`IFrController::Run()<ib::sim::fr::IFrController::Run>` command
that can be send in the WakeupHandler callback::

  void WakeupHandler(IFrController* controller, const FrSymbol& symbol)
  {
      followingColdStartNode->AllowColdstart();
      followingColdStartNode->Run();
  }

Finally, the leading coldstart node has also to respond by sending the same commands after
the PocState state changed from :cpp:enumerator:`PocState::Wakeup<ib::sim::fr::Wakeup>` to
:cpp:enumerator:`PocState::Ready<ib::sim::fr::Ready>`::
    
  if (oldState == PocState::Wakeup
      && newState == PocState::Ready)
  {
      leadingColdStartNode->AllowColdstart();
      leadingColdStartNode->Run();
  }

Note that the leading coldstart node must send these commands in the next FlexRay cycle and not
directly in a registered handler like the ControllerStateHandler.

Tx Buffer Update (Sending FlexRay Messages)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In each FlexRay cycle, data can be sent by using the
:cpp:func:`IFrController::UpdateTxBuffer()<ib::sim::fr::IFrController::UpdateTxBuffer>`.
For this, an existing txBufferIndex, a payload and the
payloadDataValid flag must be provided::

  std::string payloadString{"FlexRay message"};

  TxBufferUpdate update;
  update.payload.resize(payloadString.size());
  update.payloadDataValid = true;
  update.txBufferIndex = 0;

  std::copy(payloadString.begin(), payloadString.end(), update.payload.begin());

  controller->UpdateTxBuffer(update);

To be notified for the success or failure of the transmission, a MessageAckHandler should
be registered::
  
  // Register MessageAckHandler to receive FlexRay acknowledges from other FlexRay controller.
  auto messageAckHandler =
      [](IFrController*, const FrMessageAck& ack) {};
  frController->RegisterMessageAckHandler(messageAckHandler);

Receiving FlexRay Messages
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To receive data from other FlexRay controller, a MessageHandler must be registered,
which is called by the FlexRay controller whenever a :cpp:class:`FrMessage<ib::sim::fr::FrMessage>`
is received::

  // Register MessageHandler to receive FlexRay messages from other FlexRay controller.
  auto messageHandler =
      [](IFrController*, const FrMessage& msg) {};
  frController->RegisterMessageHandler(messageHandler);

.. admonition:: Note

  For a successful Startup, also the PocStatusHandler, the WakeupHandler, the SymbolHandler
  and the SymbolAckHandler should be registered to invoke the different necessary commands.

.. _sec:poc-status-changes:

Receiving POC status changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The protocol operation control (POC) status is a structure consisting of
status variables, substates and flags. It is modelled by the
:cpp:class:`PocStatus<ib::sim::fr::PocStatus>` structure.
Updates to the controller's PocStatus can be monitored using handlers
registered with a call to
:cpp:func:`IFrController::RegisterPocStatusHandler()<ib::sim::fr::IFrController::RegisterPocStatusHandler>`::
    
    //Register a PocStatus handler, and handle status changes
    frController->RegisterPocStatusHandler([&oldPoc](IFrController* ctrl, const PocStatus& poc) {
        // we might get called even if poc.state was not changed
        if (poc.state != oldPoc.state)
        {
            switch (poc.state)
            {
            case PocState::Halt:
                //handle halt
                break;
            case PocState::Config:
                // etc.
                break;
            //case PocState::...
                //...
            }
        }

        if (poc.freeze)
        {
          //handle freeze
        }

        if (poc.chiHaltRequest)
        {
          //deferred halt was requested ...
        }

        //if(poc....) handle other status changes

        // retain state for next handler invocation
        oldPoc = poc
    });

The handler will be invoked whenever the controller's PocStatus is updated.

.. admonition:: Note

    POC members beside PocStatus::state are updated when using an accurate simulation with
    the VIBE network simulator.


API and Data Type Reference
--------------------------------------------------
FlexRay Controller API
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygenclass:: ib::sim::fr::IFrController
  :members:

Data Structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygenstruct:: ib::sim::fr::FrMessage
  :members:
.. doxygenstruct:: ib::sim::fr::Frame
  :members:
.. doxygenstruct:: ib::sim::fr::Header
  :members:
.. doxygenstruct:: ib::sim::fr::FrMessageAck
  :members:
.. doxygenstruct:: ib::sim::fr::FrSymbol
  :members:
.. doxygenstruct:: ib::sim::fr::FrSymbolAck
  :members:
.. doxygenstruct:: ib::sim::fr::ControllerStatus
  :members:
.. doxygenstruct:: ib::sim::fr::PocStatus
  :members:
.. doxygenstruct:: ib::sim::fr::CycleStart
  :members:
.. doxygenstruct:: ib::sim::fr::ControllerConfig
  :members:
.. doxygenstruct:: ib::sim::fr::ClusterParameters
  :members:
.. doxygenstruct:: ib::sim::fr::NodeParameters
  :members:
.. doxygenstruct:: ib::sim::fr::TxBufferConfig
  :members:
.. doxygenstruct:: ib::sim::fr::TxBufferUpdate
  :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: ib::sim::fr::FrMacroTick
.. doxygentypedef:: ib::sim::fr::FrMicroTick
.. doxygenenum:: ib::sim::fr::ClockPeriod
.. doxygenenum:: ib::sim::fr::Channel
.. doxygenenum:: ib::sim::fr::SymbolPattern
.. doxygenenum:: ib::sim::fr::ChiCommand
.. doxygenenum:: ib::sim::fr::TransmissionMode
.. doxygenenum:: ib::sim::fr::PocState
.. doxygenenum:: ib::sim::fr::SlotModeType
.. doxygenenum:: ib::sim::fr::ErrorModeType
.. doxygenenum:: ib::sim::fr::StartupStateType
.. doxygenenum:: ib::sim::fr::WakeupStatusType
