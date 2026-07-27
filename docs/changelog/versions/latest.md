# [5.0.7] - UNRELEASED

## Added

- `config`: experimental `Experimental.TimeSynchronization.DynamicSimulationStep` to enable dynamic
  simulation step sizes, aligning each simulation step to the minimal step among all synchronized
  participants. Tri-state: `true` requests it for the whole simulation, `false` opts out, and leaving
  it unset follows the network. Off by default.

## Changed

- `config`: default format of file logging is set to JSON. Set "Format: Simple" in the sink to get previous behaviour.
- `docs`: added description of the logging "Format".
- `orchestration`: setting a simulation step duration of zero now raises a `SilKitError` instead of being silently accepted.