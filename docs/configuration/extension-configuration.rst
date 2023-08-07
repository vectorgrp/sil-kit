===================================================
Extensions Configuration
===================================================

.. contents:: :local:
   :depth: 3


.. _sec:cfg-extension-configuration-overview:

Overview
========================================

The Vector SIL Kit Extensions are developed by Vector and provided in binary form.
To facilitate the lookup of these binaries, specific search paths can be configured as explained below.

Configuration
=============


.. code-block:: yaml

    Extensions:
      SearchPathHints:
      - ENV:XYZ
      - "../extensions"


.. list-table:: Extension Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - SearchPathHints
     - Optional search path hints which will be considered if an extension is loaded by the SIL Kit,
       additionally to the default ones. A search path hint can contain the prefix ``ENV:``
       to refer to an environment variable name. The default search paths are the 
       environment variable ``SILKIT_EXTENSION_PATH`` and the current working directory.