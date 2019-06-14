.. IntegrationBus documentation master file, created by
   sphinx-quickstart on Fri Jun  7 10:39:25 2019.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to IntegrationBus's documentation!
==========================================

Welcome to the documentation for the Vector Integration Bus (IB), a library for a 
virtual protection platform for HAF/AF functions written in C++.

If you want to improve or extend the documentation, have a look at :doc:`How to write reStructuredText?<documentation>`.

Indices and tables
------------------

* :ref:`genindex`
* :ref:`search`


.. _getting-started-index:

Getting Started
---------------

This is an introduction.

.. toctree::
  :maxdepth: 1
  
  introduction
  architecture
  documentation


Requirements
------------

* `Python`_ 2.x.x

.. _Python: http://docs.python.org/reference/

These are the specific versions IntegrationBus is tested and built against. 
Use other versions at your own risk.
	

IntegrationBusAPI
-----------------

    .. doxygenfunction:: ib::CreateFastRtpsComAdapter

    .. doxygenclass:: ib::mw::IComAdapter
       :members:


Simulation Namespace
--------------------

    .. doxygenclass:: ib::sim::can::ICanController
       :members:

    .. doxygenclass:: ib::sim::eth::IEthController
       :members:

    .. doxygenclass:: ib::sim::eth::EthFrame
       :members:

    .. doxygenclass:: ib::sim::io::IInPort
       :members:

    .. doxygenclass:: ib::sim::io::IOutPort
       :members:

    .. doxygenclass:: ib::sim::fr::IFrController
       :members:

    .. doxygenclass:: ib::sim::lin::ILinController
       :members:

    .. doxygenclass:: ib::sim::generic::IGenericPublisher
       :members:

    .. doxygenclass:: ib::sim::generic::IGenericSubscriber
       :members:

    .. doxygenstruct:: ib::sim::generic::GenericMessage
       :members: 


Middleware Namespace
--------------------

    .. doxygenclass:: ib::mw::sync::IParticipantController
       :members:

    .. doxygenclass:: ib::mw::sync::ISystemMonitor
       :members:

    .. doxygenclass:: ib::mw::sync::ISystemController
       :members:


Config Namespace
----------------

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
