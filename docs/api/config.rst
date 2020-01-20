===============================
The VIB Configuration Mechanism
===============================
This document describes how to programmatically create a VIB configuration.
For a general discussion of the VIB configuration refer to :doc:`../configuration/configuration`.

.. |ConfigBuilder| replace:: :cpp:class:`ConfigBuilder<ib::cfg::ConfigBuilder>`
.. |ToJsonString| replace:: :cpp:func:`ToJsonString()<ib::cfg::Config::ToJsonString>`
.. |FromJsonString| replace:: :cpp:func:`FromJsonString()<ib::cfg::Config::FromJsonString>`
.. |FromJsonFile| replace:: :cpp:func:`FromJsonFile()<ib::cfg::Config::FromJsonFile>`

ConfigBuilder API
-----------------
A configuration can be built incrementally, starting with an instance of |ConfigBuilder|::

       ConfigBuilder configBuilder("simple app configuration");
       auto&& simulationSetup = configBuilder
           .WithActiveMiddleware(Middleware::VAsio)
           .SimulationSetup();
       auto&& pubCfg = simulationSetup
           .AddParticipant("MyParticipant")
           .AsSyncMaster(); 

The builder pattern allows chaining multiple methods to write a configuration in
a declarative style.
When done, a :cpp:class:`Config<ib::cfg::Config>` object can be created
from the |ConfigBuilder| by calling the :cpp::func:`Build()<ib::cfg::ConfigBuilder::Build()>`
method.

A configuration can also be loaded from a JSON file using |FromJsonFile|
or JSON data using |FromJsonString|.
Likewise, it can be exported to a JSON data using |ToJsonString|.


Configuration API
-----------------
    .. doxygenfunction:: ib::cfg::find_by_name

    .. doxygenfunction:: ib::cfg::get_by_name

    .. doxygenstruct:: ib::cfg::Config
       :members:

    .. doxygenclass:: ib::cfg::ConfigBuilder
       :members:

    .. doxygenclass:: ib::cfg::ControllerBuilder
       :members:

    .. doxygenclass:: ib::cfg::GenericPortBuilder
       :members:

    .. doxygenclass:: ib::cfg::IoPortBuilder
       :members:

    .. doxygenclass:: ib::cfg::LinkBuilder
       :members:

    .. doxygenclass:: ib::cfg::NetworkSimulatorBuilder
       :members:

    .. doxygenclass:: ib::cfg::ParentBuilder
       :members:

    .. doxygenclass:: ib::cfg::ParticipantBuilder
       :members:

    .. doxygenclass:: ib::cfg::SimulationSetupBuilder
       :members:

    .. doxygenclass:: ib::cfg::SwitchBuilder
       :members:

    .. doxygenclass:: ib::cfg::TimeSyncBuilder
       :members:

    .. doxygenstruct:: ib::cfg::TimeSync
       :members:
