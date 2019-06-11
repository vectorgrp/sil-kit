.. IntegrationBus documentation master file, created by
   sphinx-quickstart on Fri Jun  7 10:39:25 2019.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to IntegrationBus's documentation!
==========================================

Welcome to the documentation for the Vector Integration Bus (IB), a library for a 
virtual protection platform for HAF/AF functions written in C++.


.. toctree::
   :maxdepth: 2
   :caption: Contents:



Indices and tables
==================

* :ref:`genindex`
* :ref:`search`

Docs
====

Important stuff
===============

	.. doxygenstruct:: ib::mw::EndpointAddress
	   :members:

	.. doxygenclass:: ib::mw::IComAdapter
	   :members:
	
	.. doxygenstruct:: ib::cfg::Config
	   :members:
	   
	.. doxygenstruct:: ib::cfg::GenericPort
	   :members:	

   
All possible Controllers
========================

	.. doxygenclass:: ib::sim::can::ICanController
	   :members:
	   
	.. doxygenclass:: ib::sim::eth::IEthController
	   :members:

	.. doxygenclass:: ib::sim::fr::IFrController
	   :members:
	   
	.. doxygenclass:: ib::sim::lin::ILinController
	   :members:

	   
Generic Publish-Subscribe
=========================

	.. doxygenclass:: ib::sim::generic::IGenericPublisher
	   :members:
	   
	.. doxygenclass:: ib::sim::generic::IGenericSubscriber
	   :members:

	.. doxygenstruct:: ib::sim::generic::GenericMessage
	   :members: 
	   
	   
Sync Interfaces
========================
	   
	.. doxygenclass:: ib::mw::sync::ISystemMonitor
	   :members:
	   
	.. doxygenclass:: ib::mw::sync::ISystemController
	   :members:

	      
