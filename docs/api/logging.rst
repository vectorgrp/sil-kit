===================
Logging Service API
===================

SIL Kit provides a Logging service that can be used for writing log messages of specified log levels to files, stdout 
or distribute them with remote logging to other simulation participants. The logger can be obtained by calling 
:cpp:func:`GetLogger()<SilKit::IParticipant::GetLogger>`. Refer to :ref:`Logging Configuration<sec:cfg-logging-configuration-overview>` for 
how to configure the logging service.

.. contents::
   :local:
   :depth: 2

.. _sec:logging-api:

API
---

.. doxygenclass:: SilKit::Services::Logging::ILogger
   :members:

.. _sec:logging-data-types:

Data Type Reference
-------------------

.. doxygenenum:: SilKit::Services::Logging::Level
