Ethernet Controllers
====================

Any Simulation

* EthMessages are sent using IEthController::SendMessage(...).
* Received Ethessages trigger the callback that was previously registered
  using IEthController::RegisterReceiveMessageHandler(...).

Simple Simulation

* EthMessages are distributed to all other IEthControllers sharing the same link regardless
  of the content. In particular: EthMessages are delievered without checking the
  destination mac address!
* There is no need to activate the controller before sending and receiving EthMessages.
* Only ReceiveMessageHandler and MessageAckHandler are supported.
* No support for StateChangedHandler and BitRateChangedHandler.
* Time stamps must be set manually.


VIBE Simulation

* Message delivery depends message content and switch configuration.
* Messages must match the VLAN configuration of the switch ports.
* Messages are only forwarded to ports that can actually reach the destination MAC.
* Controllers must be activated before sending and receiving messages.
* The network simulator models the startup delay of establishing a link;
  trying to send a message before the link has been established will fail.
* Full support for all callbacks.
* Time stamps are set according to the end time of the message transmission.
