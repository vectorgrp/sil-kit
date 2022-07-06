# C Data Demo

Asynchronous publish-subscribe communication between two participants named
"Publisher1" and "Subscriber1". The publisher uses a history length of 1, the subscriber
registers a specific data handler. See the configuration `SilKitConfig_DemoData.json` for 
the simulation setup.

## Building
For build instructions refer to the parent demo directory.

## Usage
The Demo expects a path to the configuration file and the participant's name as arguments.
Note that the setup is asynchronous, so the subscriber has to be started first.

> ./SilKitDemoCData SilKitConfig_DemoData.json Subscriber1
> ./SilKitDemoCData SilKitConfig_DemoData.json Publisher1
