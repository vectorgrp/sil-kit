===================
FlexRay Service API
===================

.. Macros for docs use
.. |IParticipant| replace:: :cpp:class:`IParticipant<ib::mw::IParticipant>`
.. |CreateFlexrayController| replace:: :cpp:func:`CreateFlexrayController<ib::mw::IParticipant::CreateFlexrayController()>`
.. |IFlexrayController| replace:: :cpp:class:`IFlexrayController<ib::sim::fr::IFlexrayController>`

.. |FlexrayControllerConfig| replace:: :cpp:class:`FlexrayControllerConfig<ib::sim::fr::FlexrayControllerConfig>`
.. |Configure| replace:: :cpp:func:`Configure()<ib::sim::fr::IFlexrayController::Configure>`

.. |FlexrayPocState_Ready| replace:: :cpp:enumerator:`FlexrayPocState::Ready<ib::sim::fr::FlexrayPocState::Ready>`
.. |FlexrayPocState_Wakeup| replace:: :cpp:enumerator:`FlexrayPocState::Wakeup<ib::sim::fr::FlexrayPocState::Wakeup>`

.. |FlexrayClusterParameters| replace:: :cpp:class:`FlexrayClusterParameters<ib::sim::fr::FlexrayClusterParameters>`
.. |FlexrayNodeParameters| replace:: :cpp:class:`FlexrayNodeParameters<ib::sim::fr::FlexrayNodeParameters>`
.. |FlexrayTxBufferConfig| replace:: :cpp:class:`FlexrayTxBufferConfig<ib::sim::fr::FlexrayTxBufferConfig>`

.. |FlexrayFrameEvent| replace:: :cpp:class:`FlexrayFrameEvent<ib::sim::fr::FlexrayFrameEvent>`
.. |FlexrayPocStatusEvent| replace:: :cpp:class:`FlexrayPocStatusEvent<ib::sim::fr::FlexrayPocStatusEvent>`

.. |Wakeup| replace:: :cpp:func:`Wakeup()<ib::sim::fr::IFlexrayController::Wakeup>`
.. |AllowColdstart| replace:: :cpp:func:`AllowColdstart()<ib::sim::fr::IFlexrayController::AllowColdstart>`
.. |Run| replace:: :cpp:func:`Run()<ib::sim::fr::IFlexrayController::Run>`
.. |UpdateTxBuffer| replace:: :cpp:func:`UpdateTxBuffer()<ib::sim::fr::IFlexrayController::UpdateTxBuffer>`

.. |AddFrameHandler| replace:: :cpp:func:`AddFrameHandler()<ib::sim::fr::IFlexrayController::AddFrameHandler>`
.. |AddFrameTransmitHandler| replace:: :cpp:func:`AddFrameTransmitHandler()<ib::sim::fr::IFlexrayController::AddFrameTransmitHandler>`
.. |AddWakeupHandler| replace:: :cpp:func:`AddWakeupHandler()<ib::sim::fr::IFlexrayController::AddWakeupHandler>`
.. |AddPocStatusHandler| replace:: :cpp:func:`AddPocStatusHandler()<ib::sim::fr::IFlexrayController::AddPocStatusHandler>`
.. |AddSymbolHandler| replace:: :cpp:func:`AddSymbolHandler()<ib::sim::fr::IFlexrayController::AddSymbolHandler>`
.. |AddSymbolTransmitHandler| replace:: :cpp:func:`AddSymbolTransmitHandler()<ib::sim::fr::IFlexrayController::AddSymbolTransmitHandler>`
.. |AddCycleStartHandler| replace:: :cpp:func:`AddCycleStartHandler()<ib::sim::fr::IFlexrayController::AddCycleStartHandler>`

.. |RemoveFrameHandler| replace:: :cpp:func:`RemoveFrameHandler()<ib::sim::fr::IFlexrayController::RemoveFrameHandler>`
.. |RemoveFrameTransmitHandler| replace:: :cpp:func:`RemoveFrameTransmitHandler()<ib::sim::fr::IFlexrayController::RemoveFrameTransmitHandler>`
.. |RemoveWakeupHandler| replace:: :cpp:func:`RemoveWakeupHandler()<ib::sim::fr::IFlexrayController::RemoveWakeupHandler>`
.. |RemovePocStatusHandler| replace:: :cpp:func:`RemovePocStatusHandler()<ib::sim::fr::IFlexrayController::RemovePocStatusHandler>`
.. |RemoveSymbolHandler| replace:: :cpp:func:`RemoveSymbolHandler()<ib::sim::fr::IFlexrayController::RemoveSymbolHandler>`
.. |RemoveSymbolTransmitHandler| replace:: :cpp:func:`RemoveSymbolTransmitHandler()<ib::sim::fr::IFlexrayController::RemoveSymbolTransmitHandler>`
.. |RemoveCycleStartHandler| replace:: :cpp:func:`RemoveCycleStartHandler()<ib::sim::fr::IFlexrayController::RemoveCycleStartHandler>`

.. |HandlerId| replace:: :cpp:class:`HandlerId<ib::sim::HandlerId>`

.. contents::
   :local:
   :depth: 3

.. highlight:: cpp

Using the FlexRay Controller
----------------------------

The FlexRay Service API provides an FlexRay bus abstraction through the |IFlexrayController| interface.
A FlexRay controller is created by calling |CreateFlexrayController| given a controller name and (optional) network 
name::

  auto* flexrayController = participant->CreateFlexrayController("FlexRay1", "PowerTrain1");
  
FlexRay controllers will only communicate within the same network. If no network name is provided, the controller name
will be used as the network name.

.. admonition:: Note

  The FlexRay service needs a detailed simulation based on the :ref:`VIBE Network Simulator<chap:VIBE-NetSim>`.
  Because of the intrinsic complexity within FlexRay, no trivial simulation exists.

Initialization
~~~~~~~~~~~~~~

Before the FlexRay controller can be used and participate in the FlexRay communication cycles,
it must be configured, and then a Startup phase must take place at the beginning of the simulation.

Configuration
_____________

The configuration is performed by setting up a |FlexrayControllerConfig| and passing it to |Configure|.
Furthermore, |Configure| switches the controller to |FlexrayPocState_Ready| signaling that it is ready for startup.

The |FlexrayControllerConfig| consists of global |FlexrayClusterParameters| and node-specific |FlexrayNodeParameters|,
which are both best set in the participant configuration (see config section 
:ref:`FlexrayControllers<sec:cfg-participant-flexray>`). Furthermore, the |FlexrayControllerConfig| contains one or 
more |FlexrayTxBufferConfig| instances, which can either be specified in the participant configuration or added 
manually at runtime. TxBuffers are used to initiate a transmission from one FlexRay controller to another.

The following example configures a FlexRay controller with two |FlexrayTxBufferConfig| instances specifying two
|FlexrayFrameEvent| instances, which will be sent during simulation. The |FlexrayClusterParameters| and the
|FlexrayNodeParameters| are assumed to be set in the participant configuration::

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

Note that |Configure| should be called in the InitHandler of a ParticipantController.

Startup
_______

At least two FlexRay controllers are always required for a successful startup in a FlexRay cluster.
The two participants responsible for startup are also called coldstart nodes. The "leading" coldstart node 
(normally the first node that is in |FlexrayPocState_Ready|) has to send the |Wakeup| command to the other 
"following" coldstart node(s)::

  leadingColdStartNode->Wakeup();
  // The leading controllers FlexrayPocState will change from
  // Ready to Wakeup triggering the PocStatusHandler.

The response of the following cold startnode must be the |AllowColdstart| and |Run| command that can be send in the 
WakeupHandler callback::

  void WakeupHandler(IFlexrayController* controller, const FlexraySymbolEvent& symbol)
  {
      followingColdStartNode->AllowColdstart();
      followingColdStartNode->Run();
  }

Finally, the leading coldstart node has also to respond by sending the same commands after
the FlexrayPocState state changed from |FlexrayPocState_Wakeup| to |FlexrayPocState_Ready|::
    
  if (oldState == FlexrayPocState::Wakeup
      && newState == FlexrayPocState::Ready)
  {
      leadingColdStartNode->AllowColdstart();
      leadingColdStartNode->Run();
  }

Note that the leading coldstart node must send these commands in the next FlexRay cycle and not
directly in a handler like the PocStatusHandler.

Tx Buffer Update (Sending FlexRay Messages)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In each FlexRay cycle, data can be sent by using the |UpdateTxBuffer|. For this, an existing txBufferIndex, 
a payload and the payloadDataValid flag must be provided::

  std::string payloadString{"FlexRay message"};

  FlexrayTxBufferUpdate update;
  update.payload.resize(payloadString.size());
  update.payloadDataValid = true;
  update.txBufferIndex = 0;

  std::copy(payloadString.begin(), payloadString.end(), update.payload.begin());

  controller->UpdateTxBuffer(update);

To be notified for the success or failure of the transmission, a FrameTransmitHandler should
be added::
  
  // Add FrameTransmitHandler to receive FlexRay transmit events from other FlexRay controllers.
  auto frameTransmitHandler =
      [](IFlexrayController*, const FlexrayFrameTransmitEvent& ack) {};
  flexrayController->AddFrameTransmitHandler(frameTransmitHandler);

Receiving FlexRay Messages
~~~~~~~~~~~~~~~~~~~~~~~~~~

To receive data from other FlexRay controller, a ``FrameHandler`` must be added via |AddFrameHandler|, which is called 
by the FlexRay controller whenever a |FlexrayFrameEvent| is received::

  // Add FrameHandler to receive FlexRay messages from other FlexRay controller.
  auto frameHandler =
      [](IFlexrayController*, const FlexrayFrameEvent& msg) {};
  flexrayController->AddFrameHandler(frameHandler);

.. admonition:: Note

  For a successful Startup, also the ``PocStatusHandler``, the ``WakeupHandler``, the ``SymbolHandler``
  and the ``SymbolTransmitHandler`` should be added to invoke the different necessary commands.

.. _sec:poc-status-changes:

Receiving POC status changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The protocol operation control (POC) status is a structure consisting of status variables, substates and flags. It is 
modelled by the |FlexrayPocStatusEvent| structure. Updates to the controller's POC status can be monitored using 
handlers added with a call to |AddPocStatusHandler|::
    
    // Add a FlexrayPocStatusEvent handler, and handle status changes
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

The handler will be invoked whenever the controller's POC status is updated.

Managing the event handlers
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Adding a handler will return a |HandlerId| which can be used to remove the handler via:

- |RemoveFrameHandler|
- |RemoveFrameTransmitHandler|
- |RemoveWakeupHandler|
- |RemovePocStatusHandler|
- |RemoveSymbolHandler|
- |RemoveSymbolTransmitHandler|
- |RemoveCycleStartHandler|

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
