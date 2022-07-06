# C Ethernet Demo

Simple Ethernet communication between two controllers on a single participant named
"EthernetReaderWriter". See the configuration `SilKitConfig_DemoEthernet_SingleParticipant.json` for the simulation
setup.

## Building
For build instructions refer to the parent demo directory.


## Usage
The Demo expects a path to the configuration variable and the participant's name ("EthernetReaderWriter")
as arguments.
> ./SilKitDemoCEthernet SilKitConfig_DemoEthernet_SingleParticipant.json  EthernetReaderWriter

The demo needs a running SilKitRegistry to be functional. It can be started by running the following command.
> ./SilKitRegistry SilKitConfig_DemoEthernet_SingleParticipant.json