# IntegrationBus Launcher

This commandline tool launches simulation configurations on Windows and Unix-based systems.

# Prerequisites

* A Python 2.7+ installation must be available on your system
* The IntegrationBus library must be installed
* Basic features get along with the Python standard distribution.
  For additional features, some Python packages that are not part of a standard distribution must be installed.
  Use 'pip install <packagename>' to pull these packages from PyPI, depending on your usecase:
  * jsonschema: Required for JSON validation (optional)
  * jsonmerge: Required for multi-file configurations (optional)

## Getting help

Use the argument '-h' to get help:

On the Windows command prompt, type

```cmd
IbLauncher.py -h
```

Or, on a Linux bash terminal:

```bash
./IbLauncher.py
```

## Basic use

Launching a configuration when there is only one defined is as simple as stating the JSON file:

```py
IbLauncher tests/files/IbConfig_Launch-Example.json
```

For the JSON file used here, this is the same as:

```py
IbLauncher tests/files/IbConfig_Launch-Example.json -c ExampleConfiguration1 -d 42 -r 1 -x setup-run-teardown
```

Use the single launch configuration defined, with a domain ID of 42, no retries in case of failure and perform all steps: First,

* setup environments that are used (e.g. prepare the CANoe installation by adding the IntegrationBus driver), then 
* run simulation, and finally 
* tear down environments (e.g., remove patched driver from CANoe installation).

When there is only one configuration defined, it is automatically chosen by the tool.
The default domain is 42.
The default action is to setup all participants involved.
