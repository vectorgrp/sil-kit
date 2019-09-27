==============
VIB Utilities
==============

Launcher
~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  The Launcher batch script implements scripting in order to start up an Integration Bus system.
         Nevertheless, all Demos may be executed manually as described in the corresponding section.
   *  -  Source location
      -  Launcher
   *  -  Requirements
      -  * `Python <https://www.python.org/downloads/>`_ v2.7+ / v3.x+
         * Adaptation of executable directories in section "Developer-Linux" or "Developer-Windows" in Demos/Can/IbConfig_DemoCan.json
         * Adaptation of INTEGRATIONBUS_BINPATH & INTEGRATIONBUS_LIBPATH in IbInstallation.json of Launcher/iblauncher/data
   *  -  Parameters
      -  There are eight arguments:

         #. Filename of the IB Configuration to be used (IB config file).
         #. Launch configuration [-c] (e.g. Installation/Developer-WinDebug/Developer-WinRelease/Developer-Linux)
         #. Network node [-n], optional
         #. FastRTPS domain ID (optional); defaults to 42.
         #. Command [-x] (e.g. setup/run/teardown/setup-run-teardown(default)), optional
         #. Logfile [-l], optional
         #. Retries [-r], optional
         #. Quiet execution [-q], optional
   *  -  Parameter Example
      -  .. code-block:: powershell

            # Launch CAN demo w/o Network Simulator VIBE:
            Launcher/IbLauncher.sh Demos/Can/IbConfig_DemoCan.json -c Developer-Linux
   *  -  Notes
      -  INTEGRATIONBUS_BINPATH & INTEGRATIONBUS_LIBPATH may be defined as environment variables.


VAsio Registry
~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  - Abstract
      - The Registry enables discovery between IB participants when using the
        VAsio middleware. It is mandatory, when using the VAsio middleware.

   *  - Source location
      - Utilities/IbRegistry
   *  - Requirements
      - None
   *  - Parameters
      - There are up to two positional arguments:

        #. Filename of the IB Configuration to be used (IB config file).
        #. IntegrationBus domain ID (optional); defaults to 42.

   *  - Parameter Example
      - .. code-block:: powershell

            # Start the IbRegistry using the CAN demo configuration
            build/Utilities/bin/IbRegistry Demos/Can/IbConfig_DemoCan.json 42

   *  - Notes
      - The IbRegistry must be started before the IB participants. When using
        the Launcher, the IbRegistry is automatically started if the IbConfig
        specifies VAsio as the :doc:`active
        middleware<../configuration/middleware-configuration>`.


SystemController
~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  The SystemController implements state handling for the participants of a Integration Bus system.
         Examples for state change commands called by the SystemController are 'Run','Stop','Shutdown' etc.
   *  -  Source location
      -  Demos/SystemController
   *  -  Requirements
      -  The SystemController requires an established Integration Bus System.
         Thus, it has to be started after other (active) participants.
   *  -  Parameters
      -  There are up to two positional argument:

         #. Filename of the IB Configuration to be used (IB config file).
         #. FastRTPS domain ID (optional); defaults to 42.
   *  -  Parameter Example
      -  .. code-block:: powershell

            # Start SystemController for CAN Demo w/o Network Simulator VIBE:
            build/Demos/SystemController/IbDemoSystemController Demos/Can/IbConfig_DemoCan.json
   *  -  Notes
      -  | For RTPS: The above command will not be successful, unless the reader and writer participant of the CAN Demo are established upfront (see below).
         | For VAsio: The above command will not be successful, unless the reader and writer participant of the CAN Demo are established afterwards.


SystemMonitor
~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  The SystemMonitor visualizes the states of the participants of a Integration Bus system.
   *  -  Source location
      -  Demos/SysteMonitor
   *  -  Requirements
      -  None
   *  -  Parameters
      -  There are up to two positional arguments:
          
         #. Filename of the IB Configuration to be used (IB config file).
         #. FastRTPS domain ID (optional); defaults to 42.
   *  -  Parameter Example
      -  .. code-block:: powershell
            
            # Start SystemMonitor for CAN Demo w/o Network Simulator VIBE:
            build/Demos/SystemMonitor/IbDemoSystemMonitor Demos/Can/IbConfig_DemoCan.json
   *  -  Notes
      -  The SystemMonitor represents a passive participant in an Integration Bus system. Thus, it can be (re)started at any time.
