===================================================
HealthCheck Configuration
===================================================

.. contents:: :local:
   :depth: 3


.. _sec:cfg-healthcheck-configuration-overview:

Overview
========================================


.. _sec:cfg-participant-healthcheck:
       
In the ``HealthCheck`` section of the participant configuration, it is possible to set soft and hard time limits for the execution of the individual
simulation steps.

Configuration
========================================

.. code-block:: yaml

    HealthCheck:
      SoftResponseTimeout: 500
      HardResponseTimeout: 1000

.. list-table:: HealthCheck Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - SoftResponseTimeout
     - The soft limit for the execution of a simulation step given in
       milliseconds. If the simulation task does not finish within this limit, a warning
       message is logged. This limit is checked for each execution of the simulation
       task. (optional) 
   * - HardResponseTimeout
     - The hard limit for the execution of a simulation task given in
       milliseconds. If the simulation step does not finish within this limit, an
       error message is logged and the participant switches to the Error state,
       which suspends further execution of the simulation. (optional)

