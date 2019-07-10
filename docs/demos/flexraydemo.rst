FlexRay Controllers
===================

Of all vehicle networks, the difference between simple and VIBE simulation is most
prominent for FlexRay. FlexRay controllers transmit data in a fixed manner according
to the configuration of a TX buffer. And applications interact with the FlexRay controllers
by updating the content of the TX buffers. Since all vehicle network controllers are
passive components, FlexRay controllers cannot send messages according to the cycle themselves.

Any Simulation

* The FlexRay controller must be configured using a FlexRay::ControllerConfig struct,
  which combines cluster parameters, node parameters, and the buffer configuration.
* Reception of a FrMessage triggers the callback previously registered by
  IFrController::RegisterMessageHandler(...).
* Reception of a FrMessageAck triggers the callback previously registered by
  IFrController::RegisterMessageAckHandler(...).
* Reception of a WUS or WUDOP symbol triggers the callback previously registered by
  IFrController::RegisterWakupHandler().
* Reception of a FrSymbol triggers the callback previously registered by
  IFrController::RegisterSymbolHandler(...) (primarily intended for diagnostics).
* Reception of FrSymbolAck triggers the callback previously registered by
  IFrController::RegisterSymbolAckHandler(...) (primarily intended for diagnostics).

Simple Simulation

* Messages can be "sent" at any time by calling the UpdateTxBuffer method. This will trigger.
  an immediate transmission of a once corresponding message. Regardless of the repetition.
  configuration, the message will not be automatically sent again.
* Message reception is immediately acknowledged.
* Does not use cluster parameters, largely ignores node parameters.
  (currently, only pWakeupChannel is used).
* Limited support for symbols.

  * IFrController::Run() sends a CASMTS symbol.
  * IFrController::Wakeup() sends the WUS symbol.
  * Symbol reception is immediately acknowledged.

* Limited emulation of the POC (all trigger the registered ControllerStatusHandler).

  * IFrController::Configure(...) causes a switch to PocState::Ready.
  * IFrController::Run() causes a switch to PocState::NormalActive.
  * IFrController::Wakeup() causes a switch to PocState::Wakeup.
  * Reception of a WUS symbol causes a switch to PocState::Ready.

VIBE Simulation

* Requires valid controller configuration with valid cluster and node parameters.
* Models the FlexRay POC and requires usage of the FlexRay controller
  according to the controller host interface (CHI).
* CHI commands must only be issued when the current PocState allows
  (cf. FlexRay Protocol Specification V3.0, Sec. 2.2.1.1 Host Commands).
* IFrController::UpdateTxBuffer() must only be called in PocState::NormalActive.
* Models FlexRay cycle startup and tear down in detail.

  * Requires Wakeup command to startup the FlexRay cycle. Requires at least one other
    FlexRay controller to have cold start enabled (IFrController::AllowColdstart()).

* Provides automated and repeated transmission of FlexRay frames according to configured repetition.
* Time stamps are set according to the end time of the transmission.
