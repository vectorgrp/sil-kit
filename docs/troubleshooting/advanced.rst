Advanced 
========

This section shows how to use advanced tools like the `Vector SIL Kit Dashboard <https://vector.com/sil-kit-dashboard>`_ to visualize simulations, in order to identitfy participant misconfigurations, detect system misbehavior or analyze participant performance.

Getting Started with the dashboard
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The dashboard is a freeware application that collects and visualizes SIL Kit simulation data.
It can be used to show static attributes of participants and also show their simulation properties like active peer connections and simulated, active networks.
In addition, the collection of performance metrics can be enabled in the configuration files of the SIL Kit participants.

This section shows how to enable the data collection of simulations for the Vector SIL Kit Dashboard.
Please refer to the SIL Kit Dashboard documentation for updated instructions.

#. The SIL Kit registry will forward simulation data to a running Vector SIL Kit Dashboard instance, if configured properly.
   The ``sil-kit-registry`` executable supports the ``--dashboard-uri https://1.2.3.4`` command line flag and the registry's configuration supports the ``DashboardUri: https://1.2.3.4`` declaration.
   Ensure that the registry is able to connect to the Dashboard when started, e.g. by verifying the log output.
   The registry's configuration of ``CollectFromRemotes: true`` defaults to true -- it can be disabled explicitly.
#. Each participant that should be included in the dashboard visualization, must add a remot metric sink to its conifugration file:

   .. code-block:: yaml

      Experimental:
        Metrics:
          Sinks:
            - Name: RemoteSink1
              Type: Remote

   This will enable the data updates from the participant to the registry, which will forward it to the dashboard.
#. Start a simulation run, the dashboard will show the simulation's state, including the participant states, attributes and simulated network details. If the metrics sinks are enabled, detailed performance counters will be available for analysis.
