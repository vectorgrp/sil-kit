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

Configuring the FastRTPS Middleware
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TBD

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
