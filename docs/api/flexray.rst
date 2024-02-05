.. _chap:flexray-service-api:

===================
FlexRay Service API
===================

.. Macros for docs use
.. |IParticipant| replace:: :cpp:class:`IParticipant<SilKit::IParticipant>`
.. |CreateFlexrayController| replace:: :cpp:func:`CreateFlexrayController<SilKit::IParticipant::CreateFlexrayController()>`
.. |IFlexrayController| replace:: :cpp:class:`IFlexrayController<SilKit::Services::Flexray::IFlexrayController>`

.. |FlexrayControllerConfig| replace:: :cpp:class:`FlexrayControllerConfig<SilKit::Services::Flexray::FlexrayControllerConfig>`
.. |Configure| replace:: :cpp:func:`Configure()<SilKit::Services::Flexray::IFlexrayController::Configure>`

.. |FlexrayPocState_Ready| replace:: :cpp:enumerator:`FlexrayPocState::Ready<SilKit::Services::Flexray::FlexrayPocState::Ready>`
.. |FlexrayPocState_Wakeup| replace:: :cpp:enumerator:`FlexrayPocState::Wakeup<SilKit::Services::Flexray::FlexrayPocState::Wakeup>`

.. |FlexrayClusterParameters| replace:: :cpp:class:`FlexrayClusterParameters<SilKit::Services::Flexray::FlexrayClusterParameters>`
.. |FlexrayNodeParameters| replace:: :cpp:class:`FlexrayNodeParameters<SilKit::Services::Flexray::FlexrayNodeParameters>`
.. |FlexrayTxBufferConfig| replace:: :cpp:class:`FlexrayTxBufferConfig<SilKit::Services::Flexray::FlexrayTxBufferConfig>`

.. |FlexrayFrameEvent| replace:: :cpp:class:`FlexrayFrameEvent<SilKit::Services::Flexray::FlexrayFrameEvent>`
.. |FlexrayPocStatusEvent| replace:: :cpp:class:`FlexrayPocStatusEvent<SilKit::Services::Flexray::FlexrayPocStatusEvent>`

.. |Wakeup| replace:: :cpp:func:`Wakeup()<SilKit::Services::Flexray::IFlexrayController::Wakeup>`
.. |AllowColdstart| replace:: :cpp:func:`AllowColdstart()<SilKit::Services::Flexray::IFlexrayController::AllowColdstart>`
.. |Run| replace:: :cpp:func:`Run()<SilKit::Services::Flexray::IFlexrayController::Run>`
.. |UpdateTxBuffer| replace:: :cpp:func:`UpdateTxBuffer()<SilKit::Services::Flexray::IFlexrayController::UpdateTxBuffer>`

.. |AddFrameHandler| replace:: :cpp:func:`AddFrameHandler()<SilKit::Services::Flexray::IFlexrayController::AddFrameHandler>`
.. |AddFrameTransmitHandler| replace:: :cpp:func:`AddFrameTransmitHandler()<SilKit::Services::Flexray::IFlexrayController::AddFrameTransmitHandler>`
.. |AddWakeupHandler| replace:: :cpp:func:`AddWakeupHandler()<SilKit::Services::Flexray::IFlexrayController::AddWakeupHandler>`
.. |AddPocStatusHandler| replace:: :cpp:func:`AddPocStatusHandler()<SilKit::Services::Flexray::IFlexrayController::AddPocStatusHandler>`
.. |AddSymbolHandler| replace:: :cpp:func:`AddSymbolHandler()<SilKit::Services::Flexray::IFlexrayController::AddSymbolHandler>`
.. |AddSymbolTransmitHandler| replace:: :cpp:func:`AddSymbolTransmitHandler()<SilKit::Services::Flexray::IFlexrayController::AddSymbolTransmitHandler>`
.. |AddCycleStartHandler| replace:: :cpp:func:`AddCycleStartHandler()<SilKit::Services::Flexray::IFlexrayController::AddCycleStartHandler>`

.. |RemoveFrameHandler| replace:: :cpp:func:`RemoveFrameHandler()<SilKit::Services::Flexray::IFlexrayController::RemoveFrameHandler>`
.. |RemoveFrameTransmitHandler| replace:: :cpp:func:`RemoveFrameTransmitHandler()<SilKit::Services::Flexray::IFlexrayController::RemoveFrameTransmitHandler>`
.. |RemoveWakeupHandler| replace:: :cpp:func:`RemoveWakeupHandler()<SilKit::Services::Flexray::IFlexrayController::RemoveWakeupHandler>`
.. |RemovePocStatusHandler| replace:: :cpp:func:`RemovePocStatusHandler()<SilKit::Services::Flexray::IFlexrayController::RemovePocStatusHandler>`
.. |RemoveSymbolHandler| replace:: :cpp:func:`RemoveSymbolHandler()<SilKit::Services::Flexray::IFlexrayController::RemoveSymbolHandler>`
.. |RemoveSymbolTransmitHandler| replace:: :cpp:func:`RemoveSymbolTransmitHandler()<SilKit::Services::Flexray::IFlexrayController::RemoveSymbolTransmitHandler>`
.. |RemoveCycleStartHandler| replace:: :cpp:func:`RemoveCycleStartHandler()<SilKit::Services::Flexray::IFlexrayController::RemoveCycleStartHandler>`

.. |HandlerId| replace:: :cpp:class:`HandlerId<SilKit::Services::HandlerId>`

.. contents::
   :local:
   :depth: 3

.. highlight:: cpp

Using the FlexRay Controller
----------------------------

The FlexRay Service API provides an FlexRay bus abstraction through the |IFlexrayController| interface.
A FlexRay controller is created by calling |CreateFlexrayController| given a controller name and network 
name::

  auto* flexrayController = participant->CreateFlexrayController("FlexRay1", "PowerTrain1");
  
FlexRay controllers will only communicate within the same network.

.. admonition:: Note

  The FlexRay service needs a detailed simulation based on the network simulator.
  Because of the intrinsic complexity within FlexRay, no trivial simulation exists.

Initialization
~~~~~~~~~~~~~~

Before the FlexRay controller can be used and participate in FlexRay communication cycles,
it must be configured, and a startup phase must take place at the beginning of the simulation.

Configuration
_____________

The configuration is performed by setting up a |FlexrayControllerConfig| and passing it to |Configure|.
Furthermore, |Configure| switches the controller to |FlexrayPocState_Ready| signaling that it is ready for startup.

The |FlexrayControllerConfig| consists of global |FlexrayClusterParameters| and node-specific |FlexrayNodeParameters|,
which are both best set in the participant configuration (see config section 
:ref:`FlexrayControllers<sec:cfg-participant-flexray>`). Furthermore, the |FlexrayControllerConfig| contains one or 
more |FlexrayTxBufferConfig| instances, which can either be specified in the participant configuration or added 
manually at runtime. Tx buffers are used to initiate a transmission from one FlexRay controller to another.

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

Note that |Configure| should be called in the ``CommunicationReadyHandler`` of the ``LifecycleService``.

Startup
_______

At least two FlexRay controllers are always required for a successful startup in a FlexRay cluster.
The two participants responsible for startup are also called cold start nodes. The "leading" cold start node 
(normally the first node that is in |FlexrayPocState_Ready|) has to send the |Wakeup| command to the other 
"following" cold start node(s)::

  leadingColdStartNode->Wakeup();
  // The leading controllers FlexrayPocState will change from
  // Ready to Wakeup triggering the PocStatusHandler.

The response of the following cold start node must be the |AllowColdstart| and |Run| command that can be sent in the 
``WakeupHandler`` callback::

  void WakeupHandler(IFlexrayController* controller, const FlexraySymbolEvent& symbol)
  {
      followingColdStartNode->AllowColdstart();
      followingColdStartNode->Run();
  }

Finally, the leading cold start node has also to respond by sending the same commands after
the ``FlexrayPocState`` changed from |FlexrayPocState_Wakeup| to |FlexrayPocState_Ready|::
    
  if (oldState == FlexrayPocState::Wakeup
      && newState == FlexrayPocState::Ready)
  {
      leadingColdStartNode->AllowColdstart();
      leadingColdStartNode->Run();
  }

Note that the leading cold start node must send these commands in the next FlexRay cycle and not
directly in a handler like the ``PocStatusHandler``.

Tx Buffer Update (Sending FlexRay Messages)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In each FlexRay cycle, data can be sent by using the |UpdateTxBuffer|. For this, an existing ``txBufferIndex``, 
a ``payload`` and the ``payloadDataValid`` flag must be provided::

  std::string payloadString{"FlexRay message"};

  FlexrayTxBufferUpdate update;
  update.payload.resize(payloadString.size());
  update.payloadDataValid = true;
  update.txBufferIndex = 0;

  std::copy(payloadString.begin(), payloadString.end(), update.payload.begin());

  controller->UpdateTxBuffer(update);

To be notified for the success or failure of the transmission, a ``FrameTransmitHandler`` should
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

  For a successful startup, also the ``PocStatusHandler``, the ``WakeupHandler``, the ``SymbolHandler``
  and the ``SymbolTransmitHandler`` should be added to invoke the different necessary commands.

.. _sec:poc-status-changes:

Receiving POC Status Changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The protocol operation control (POC) status is a structure consisting of status variables, sub-states and flags. It is 
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

Managing the Event Handlers
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Adding a handler will return a |HandlerId| which can be used to remove the handler via:

- |RemoveFrameHandler|
- |RemoveFrameTransmitHandler|
- |RemoveWakeupHandler|
- |RemovePocStatusHandler|
- |RemoveSymbolHandler|
- |RemoveSymbolTransmitHandler|
- |RemoveCycleStartHandler|

API and Data Type Reference
---------------------------

FlexRay Controller API
~~~~~~~~~~~~~~~~~~~~~~
.. doxygenclass:: SilKit::Services::Flexray::IFlexrayController
  :members:

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: SilKit::Services::Flexray::FlexrayFrame
  :members:
.. doxygenstruct:: SilKit::Services::Flexray::FlexrayHeader
  :members:
.. doxygenstruct:: SilKit::Services::Flexray::FlexrayFrameEvent
  :members:
.. doxygenstruct:: SilKit::Services::Flexray::FlexrayFrameTransmitEvent
  :members:
.. doxygenstruct:: SilKit::Services::Flexray::FlexraySymbolEvent
  :members:
.. doxygenstruct:: SilKit::Services::Flexray::FlexraySymbolTransmitEvent
.. doxygenstruct:: SilKit::Services::Flexray::FlexrayWakeupEvent
.. doxygenstruct:: SilKit::Services::Flexray::FlexrayPocStatusEvent
  :members:
.. doxygenstruct:: SilKit::Services::Flexray::FlexrayCycleStartEvent
  :members:
.. doxygenstruct:: SilKit::Services::Flexray::FlexrayControllerConfig
  :members:
.. doxygenstruct:: SilKit::Services::Flexray::FlexrayClusterParameters
  :members:
.. doxygenstruct:: SilKit::Services::Flexray::FlexrayNodeParameters
  :members:
.. doxygenstruct:: SilKit::Services::Flexray::FlexrayTxBufferConfig
  :members:
.. doxygenstruct:: SilKit::Services::Flexray::FlexrayTxBufferUpdate
  :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: SilKit::Services::Flexray::FlexrayMicroTick
.. doxygenenum:: SilKit::Services::Flexray::FlexrayClockPeriod
.. doxygenenum:: SilKit::Services::Flexray::FlexrayChannel
.. doxygenenum:: SilKit::Services::Flexray::FlexraySymbolPattern
.. doxygenenum:: SilKit::Services::Flexray::FlexrayTransmissionMode
.. doxygenenum:: SilKit::Services::Flexray::FlexrayPocState
.. doxygenenum:: SilKit::Services::Flexray::FlexraySlotModeType
.. doxygenenum:: SilKit::Services::Flexray::FlexrayErrorModeType
.. doxygenenum:: SilKit::Services::Flexray::FlexrayStartupStateType
.. doxygenenum:: SilKit::Services::Flexray::FlexrayWakeupStatusType
