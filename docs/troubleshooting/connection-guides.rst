.. include:: /substitutions.rst

Connection Guides
=================

This chapter provides instructions for specific connection scenarios between various host.

.. contents::
   :local:
   :depth: 2


Windows and WSL
~~~~~~~~~~~~~~~

Registry running in WSL
+++++++++++++++++++++++

#. If needed, get the precompiled registry from a Ubuntu build of the |ProductName| at https://github.com/vectorgrp/sil-kit/releases.
#. On WSL, use ``ip addr show eth0`` and note down the IP address displayed at ``inet``, e.g., ``172.19.71.101``.
   You WSL network should be reachable under this IP address.
#. Start the registry on the WSL side with this IP address and a port::

      ./sil-kit-registry --listen-uri silkit://172.19.71.101:8500

#. On Windows, launch a |ProductName| participant with the same connect URI.
   For testing purposes, the SIL Kit Monitor utility can be used::

      .\sil-kit-monitor.exe --connect-uri silkit://172.19.71.101:8500

#. The registry should report the successful connection with a log output like::

     [SilKitRegistry] [info] Sending known participant message to SystemMonitor, protocol version 3.1

Registry running in Windows
+++++++++++++++++++++++++++

This setup is possible but requires either 

- network mirroring mode for WSL (available for WSL2) or
- reconfiguration of the WSL and Windows network adapters and possible adaption of firewall settings.

However, the configuration is error-prone and out of scope for this documentation.
The more convenient setup is to run the registry in the WSL instance, as described in the previous section.

.. TODO
   Windows and Linux VM
   Windows and Docker
   Linux and Docker
   Windows and OpenSSH-Server 



