===================================================
Middleware Config
===================================================

.. contents:: :local:
   :depth: 3

Overview
--------------------

Since Sprint-31, the Vector Integration Bus offers a second middlewares to choose
from. Besides the established Fast-RTPS middleware, you can now choose the VAsio
middleware, a boost ASIO based middleware that was specifically designed for the Vector
Integration Bus.

By and large, the two middlewares are interchangeable with almost no necessary changes to
the applications using the VIB. In particular, applications that once have been
:ref:`enabled<sec:mwcfg-enable-vasio>` for the new VAsio middleware also work with the old
Fast-RTPS one. From that point on, the :ref:`active middleware<sec:mwcfg-active-middleware>` can
be selected using an IbConfig.json file.

.. admonition:: Note

    If you do not plan on using the new VAsio middleware, there is no need for any changes
    on your application. The Sprint-31 API is fully compatible with previous
    versions. However, it is still recommended to :ref:`enable<sec:mwcfg-enable-vasio>`
    application for the new VAsio middleware to provide the best interoperability.


.. _sec:mwcfg-active-middleware:

Selecting the Active Middleware
----------------------------------------

The active middleware can be configured via the IbConfig.json file. Simply set
"/MiddlewareConfig/ActiveMiddleware" to either "FastRTPS" or "VAsio". E.g., to enable the
VAsio middleware:

.. code-block:: javascript

    {
        ...
        "MiddlewareConfig": {
            "ActiveMiddleware": "VAsio"
        }
    }

The default value for ActiveMiddleware is FastRTPS, so that
:cpp:func:`ib::CreateComAdapter()` can be used as a drop in replacement for
:cpp:func:`ib::CreateFastRtpsComAdapter()`, even when older config files are used.

.. admonition:: Note

    Choosing the active middleware via IbConfig requires that the VIB applications use the
    new generic :cpp:func:`ib::CreateComAdapter()` factory method.

.. admonition:: Note

    It is still possible to create a ComAdapter explicitly for a specific middleware,
    i.e., using :cpp:func:`ib::CreateFastRtpsComAdapter()` and
    :cpp:func:`ib::CreateVAsioComAdapter()`. However this approach is considered
    deprecated and is only intended for backwards compatibility with previous versions of
    the VIB.

    
Configuring the VAsio Middleware
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For the initial discovery of the VIB participants, the VAsio middleware uses a registry
process at a preconfigured hostname and port. By default, the registry is expected to be
running on localhost listening on Port 8500. These values can be changed via the new
*/MiddlewareConfig/VAsio* section, e.g.:


.. code-block:: javascript

    {
        ...
        "MiddlewareConfig": {
            "ActiveMiddleware": "VAsio",
            "VAsio": {
                "Registry": {
                    "Hostname": "remotehost",
                    "Port": 14014
                }
            }
        }
    }

.. list-table:: VAsio Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description

   * - Registry
     - The optional Registry section allows specifying the hostname and a base
       port to be used by participants when connecting to the IbRegistry. By
       default, the registry is expected to be running on "localhost" and is
       listening on port 8500 + *IbDomainId*.

    
Configuring the FastRTPS Middleware
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FastRTPS offers many configuration options. The most common ones can be directly
configured via the IbConfig.json. Detailed, fine grained FastRTPS configuration
can be performed using a FastRTPS XML config file.

The following example shows how to enable unicast discovery with four
participants running on four different hosts. And the history of each FastRTPS
topic instance is set to 100.

.. code-block:: javascript

    {
        ...
        "MiddlewareConfig": {
            "FastRTPS": {
                "DiscoveryType": "Unicast",
                "UnicastLocators": {
                    "CanWriter": "192.168.190.1",
                    "CanReader": "192.168.190.2",
                    "SystemController": "192.168.190.3",
                    "SystemMonitor": "192.168.190.4"
                },
                "HistoryDepth": 100
            }
    }


.. list-table:: FastRTPS Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description

   * - DiscoveryType
     - The optional DiscoveryType determines how discovery between the
       individual participants is performed. The options are *Local*, *Unicast*,
       *Multicast*, and *ConfigFile*. *Local* performs a unicast discovery on
       localhost only, *Unicast* performs unicast discovery with explicit IP
       configurations per participant (cf. *UnicastLocators* list below),
       *Multicast* performs multicast discovery on all network interfaces
       (**warning** Multicast discovery can result in multiple IB instances
       interfering with each other!), *ConfigFile* will perform discovery
       according to the FastRTPS XML config file given by *ConfigFileName*
       below.

   * - UnicastLocators
     - A list of "ParticipantName": "IP-Address" pairs, one for each
       participant. All participants in the configuration must be
       specified. Mandatory if *DiscoveryType* is set to *Unicast*.

   * - ConfigFileName
     - An optional FastRTPS XML configuration file. Paths are relative to the
       IbConfig file.

   * - SendSocketBufferSize
     - The optional buffer size of the FastRTPS send socket. If not specified,
       FastRTPS will use it's internal default value.
   * - ListenSocketBufferSize
     - The buffer size of the FastRTPS listen socket. If not specified,
       FastRTPS will use it's internal default value.
   * - HistoryDepth
     - The optional HistoryDepth specifies the number of items FastRTPS keeps
       for each send and receive history of each topic instance. By default a
       depth of 5 items is used. If you are sending many items per SimTask
       execution, you might need to set a larger history depth to avoid items
       being overwritten before they are transmitted.


.. _sec:mwcfg-enable-vasio:

Enabling VIB Applications for the VAsio Middleware
------------------------------------------------------------

To make your VIB application work with the VAsio middleware, two changes are necessary:
    1. Replace the Fast-RTPS ComAdapter creation with the generic ComAdapter creation,
        i.e., replace :cpp:func:`ib::CreateFastRtpsComAdapter()` with
        :cpp:func:`ib::CreateComAdapter()`.
    2. Controller initialization must be moved to the
       :cpp:func:`IParticipantController::SetInitHandler()<ib::mw::sync::IParticipantController::SetInitHandler()>` callback.

The first change is to enable middleware configuration via the IbConfig. The second change
is necessary as the VAsio does not use histories of published values as FastRTPS
does. Thus, configuring a controller earlier than the InitHandler can result in lost
configuration data.

For example, the necessary changes for the CAN demo are as follows


**Old CAN Demo (Only works with FastRTPS)**

.. code-block:: cpp
    :emphasize-lines: 1,6,7

    auto comAdapter = ib::CreateFastRtpsComAdapter(ibConfig, participantName, domainId);
    auto* canController = comAdapter->CreateCanController("CAN1");
    
    canController->RegisterTransmitStatusHandler(&AckCallback);
    canController->RegisterReceiveMessageHandler(&ReceiveMessage);
    canController->SetBaudRate(10000, 1000000);
    canController->Start();
    
    // Set an Init Handler
    participantController->SetInitHandler([&participantName](auto initCmd) {
    
        std::cout << "Initializing " << participantName << std::endl;
    
    });
                
**New CAN Demo (works with VAsio and FastRTPS)**

.. code-block:: cpp
    :emphasize-lines: 1,11,12

    auto comAdapter = ib::CreateComAdapter(ibConfig, participantName, domainId);
    auto* canController = comAdapter->CreateCanController("CAN1");
    
    canController->RegisterTransmitStatusHandler(&AckCallback);
    canController->RegisterReceiveMessageHandler(&ReceiveMessage);
    
    // Set an Init Handler
    participantController->SetInitHandler([canController, &participantName](auto initCmd) {
    
        std::cout << "Initializing " << participantName << std::endl;
        canController->SetBaudRate(10000, 1000000);
        canController->Start();
    
    });


ComAdapter Factory Methods
----------------------------------------

CreateComAdapter
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygenfunction:: ib::CreateComAdapter


CreateFastRtpsComAdapter
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygenfunction:: ib::CreateFastRtpsComAdapter

CreateVAsioComAdapter
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ib::CreateVAsioComAdapter
