===================
FlexRay Service API
===================


.. contents::
   :local:
   :depth: 3


.. highlight:: cpp

Using the FlexRay Controller
----------------------------

.. admonition:: Note

  The FlexRay service needs a detailed simulation based on the :ref:`VIBE Network Simulator<chap:VIBE-NetSim>`.
  Because of the intrinsic complexity within FlexRay, no trivial simulation exists.


Initialization
~~~~~~~~~~~~~~

Before the FlexRay controller can be used and participate in the FlexRay communication cycles,
it must be configured, and then a Startup phase must take place at the beginning of the simulation.

Configuration
_____________

The configuration is performed by setting up a :cpp:class:`FlexrayControllerConfig<ib::sim::fr::FlexrayControllerConfig>` and passing it to
:cpp:func:`IFlexrayController::Configure()<ib::sim::fr::IFlexrayController::Configure>`. Furthermore,
:cpp:func:`IFlexrayController::Configure()<ib::sim::fr::IFlexrayController::Configure>` switches the controller
to :cpp:enumerator:`FlexrayPocState::Ready<ib::sim::fr::FlexrayPocState::Ready>` signaling that it is ready for startup.

The :cpp:class:`FlexrayControllerConfig<ib::sim::fr::FlexrayControllerConfig>` consists of global
:cpp:class:`FlexrayClusterParameters<ib::sim::fr::FlexrayClusterParameters>` and node-specific
:cpp:class:`FlexrayNodeParameters<ib::sim::fr::FlexrayNodeParameters>`, which are both best set
in the participant configuration (see config section :ref:`FlexrayControllers<sec:cfg-participant-flexray>`).
Furthermore, the :cpp:class:`FlexrayControllerConfig<ib::sim::fr::FlexrayControllerConfig>`
contains one or more :cpp:class:`FlexrayTxBufferConfig<ib::sim::fr::FlexrayTxBufferConfig>` instances,
which can either be specified in the participant configuration or added manually at
runtime. TxBuffers are used to initiate a transmission from one FlexRay
controller to another.

The following example configures a FlexRay controller with two
:cpp:class:`FlexrayTxBufferConfig<ib::sim::fr::FlexrayTxBufferConfig>` instances specifying two
:cpp:class:`FlexrayFrameEvent<ib::sim::fr::FlexrayFrameEvent>` instances, which will be sent during simulation. The
:cpp:class:`FlexrayClusterParameters<ib::sim::fr::FlexrayClusterParameters>` and the
:cpp:class:`FlexrayNodeParameters<ib::sim::fr::FlexrayNodeParameters>` are assumed to be set in the participant configuration::

    std::vector<FlexrayTxBufferConfig> bufferConfigs;
    FlexrayTxBufferConfig txConfig;
    txConfig.channels = FlexrayChannel::AB;
    txConfig.slotId = 10;
    txConfig.offset = 0;
    txConfig.repetition = 1;
    txConfig.hasPayloadPreambleIndicator = false;
    txConfig.headerCrc = 5;
    txConfig.transmissionMode = FlexrayTransmissionMode::SingleShot;
    bufferConfigs.push_back(txConfig);

    txConfig.channels = FlexrayChannel::A;
    txConfig.slotId = 20;
    bufferConfigs.push_back(txConfig);

    FlexrayControllerConfig controllerConfig;
    controllerConfig.bufferConfigs = bufferConfigs;
    controllerConfig.clusterParams = participantConfig.flexrayControllers[0].clusterParameters;
    controllerConfig.nodeParams = participantConfig.flexrayControllers[0].nodeParameters;

    flexrayController->Configure(controllerConfig);

Note that :cpp:func:`IFlexrayController::Configure()<ib::sim::fr::IFlexrayController::Configure>`
should be called in the InitHandler of a ParticipantController.

Startup
_______

At least two FlexRay controllers are always required for a successful startup in a FlexRay cluster.
The two participants responsible for startup are also called coldstart nodes. The "leading"
coldstart node (normally the first node that is in :cpp:enumerator:`FlexrayPocState::Ready<ib::sim::fr::FlexrayPocState::Ready>`)
has to send the :cpp:func:`IFlexrayController::Wakeup()<ib::sim::fr::IFlexrayController::Wakeup>` command
to the other "following" coldstart node(s)::

  leadingColdStartNode->Wakeup();
  // The leading controllers FlexrayPocState will change from
  // Ready to Wakeup triggering the PocStatusHandler.

The response of the following cold startnode must be the
:cpp:func:`IFlexrayController::AllowColdstart()<ib::sim::fr::IFlexrayController::AllowColdstart>` and
:cpp:func:`IFlexrayController::Run()<ib::sim::fr::IFlexrayController::Run>` command
that can be send in the WakeupHandler callback::

  void WakeupHandler(IFlexrayController* controller, const FlexraySymbolEvent& symbol)
  {
      followingColdStartNode->AllowColdstart();
      followingColdStartNode->Run();
  }

Finally, the leading coldstart node has also to respond by sending the same commands after
the FlexrayPocState state changed from :cpp:enumerator:`FlexrayPocState::Wakeup<ib::sim::fr::FlexrayPocState::Wakeup>` to
:cpp:enumerator:`FlexrayPocState::Ready<ib::sim::fr::FlexrayPocState::Ready>`::
    
  if (oldState == FlexrayPocState::Wakeup
      && newState == FlexrayPocState::Ready)
  {
      leadingColdStartNode->AllowColdstart();
      leadingColdStartNode->Run();
  }

Note that the leading coldstart node must send these commands in the next FlexRay cycle and not
directly in a registered handler like the ControllerStateHandler.

Tx Buffer Update (Sending FlexRay Messages)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In each FlexRay cycle, data can be sent by using the
:cpp:func:`IFlexrayController::UpdateTxBuffer()<ib::sim::fr::IFlexrayController::UpdateTxBuffer>`.
For this, an existing txBufferIndex, a payload and the
payloadDataValid flag must be provided::

  std::string payloadString{"FlexRay message"};

  FlexrayTxBufferUpdate update;
  update.payload.resize(payloadString.size());
  update.payloadDataValid = true;
  update.txBufferIndex = 0;

  std::copy(payloadString.begin(), payloadString.end(), update.payload.begin());

  controller->UpdateTxBuffer(update);

To be notified for the success or failure of the transmission, a FrameTransmitHandler should
be registered::
  
  // Register FrameTransmitHandler to receive FlexRay transmit events from other FlexRay controllers.
  auto frameTransmitHandler =
      [](IFlexrayController*, const FlexrayFrameTransmitEvent& ack) {};
  flexrayController->AddFrameTransmitHandler(frameTransmitHandler);

Receiving FlexRay Messages
~~~~~~~~~~~~~~~~~~~~~~~~~~

To receive data from other FlexRay controller, a FrameHandler must be registered,
which is called by the FlexRay controller whenever a :cpp:class:`FlexrayFrameEvent<ib::sim::fr::FlexrayFrameEvent>`
is received::

  // Register FrameHandler to receive FlexRay messages from other FlexRay controller.
  auto frameHandler =
      [](IFlexrayController*, const FlexrayFrameEvent& msg) {};
  flexrayController->AddFrameHandler(frameHandler);

.. admonition:: Note

  For a successful Startup, also the PocStatusHandler, the WakeupHandler, the SymbolHandler
  and the SymbolTransmitHandler should be registered to invoke the different necessary commands.

.. _sec:poc-status-changes:

Receiving POC status changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The protocol operation control (POC) status is a structure consisting of
status variables, substates and flags. It is modelled by the
:cpp:class:`FlexrayPocStatusEvent<ib::sim::fr::FlexrayPocStatusEvent>` structure.
Updates to the controller's POC status can be monitored using handlers
registered with a call to
:cpp:func:`IFlexrayController::RegisterPocStatusHandler()<ib::sim::fr::IFlexrayController::AddPocStatusHandler>`::
    
    //Register a FlexrayPocStatusEvent handler, and handle status changes
    flexrayController->AddPocStatusHandler([&oldPoc](IFlexrayController* ctrl, const FlexrayPocStatusEvent& poc) {
        // we might get called even if poc.state was not changed
        if (poc.state != oldPoc.state)
        {
            switch (poc.state)
            {
            case FlexrayPocState::Halt:
                //handle halt
                break;
            case FlexrayPocState::Config:
                // etc.
                break;
            //case FlexrayPocState::...
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

The handler will be invoked whenever the controller's FlexrayPocStatusEvent is updated.

Message Tracing
~~~~~~~~~~~~~~~

.. admonition:: Note

  Currently the Message Tracing functionality is not available, but it will be reintegrated in the future.


The FrController supports message tracing in MDF4 format.
This is provided by the :ref:`VIBE MDF4Tracing<mdf4tracing>` extension.
Refer to the :ref:`sec:cfg-participant-tracing` configuration section for usage instructions.

API and Data Type Reference
---------------------------

FlexRay Controller API
~~~~~~~~~~~~~~~~~~~~~~
.. doxygenclass:: ib::sim::fr::IFlexrayController
  :members:

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: ib::sim::fr::FlexrayFrame
  :members:
.. doxygenstruct:: ib::sim::fr::FlexrayHeader
  :members:
.. doxygenstruct:: ib::sim::fr::FlexrayFrameEvent
  :members:
.. doxygenstruct:: ib::sim::fr::FlexrayFrameTransmitEvent
  :members:
.. doxygenstruct:: ib::sim::fr::FlexraySymbolEvent
  :members:
.. doxygenstruct:: ib::sim::fr::FlexraySymbolTransmitEvent
.. doxygenstruct:: ib::sim::fr::FlexrayWakeupEvent
.. doxygenstruct:: ib::sim::fr::FlexrayPocStatusEvent
  :members:
.. doxygenstruct:: ib::sim::fr::FlexrayCycleStartEvent
  :members:
.. doxygenstruct:: ib::sim::fr::FlexrayControllerConfig
  :members:
.. doxygenstruct:: ib::sim::fr::FlexrayClusterParameters
  :members:
.. doxygenstruct:: ib::sim::fr::FlexrayNodeParameters
  :members:
.. doxygenstruct:: ib::sim::fr::FlexrayTxBufferConfig
  :members:
.. doxygenstruct:: ib::sim::fr::FlexrayTxBufferUpdate
  :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: ib::sim::fr::FlexrayMacroTick
.. doxygentypedef:: ib::sim::fr::FlexrayMicroTick
.. doxygenenum:: ib::sim::fr::FlexrayClockPeriod
.. doxygenenum:: ib::sim::fr::FlexrayChannel
.. doxygenenum:: ib::sim::fr::FlexraySymbolPattern
.. doxygenenum:: ib::sim::fr::FlexrayChiCommand
.. doxygenenum:: ib::sim::fr::FlexrayTransmissionMode
.. doxygenenum:: ib::sim::fr::FlexrayPocState
.. doxygenenum:: ib::sim::fr::FlexraySlotModeType
.. doxygenenum:: ib::sim::fr::FlexrayErrorModeType
.. doxygenenum:: ib::sim::fr::FlexrayStartupStateType
.. doxygenenum:: ib::sim::fr::FlexrayWakeupStatusType
