===================================================
Extensions Configuration
===================================================

.. contents:: :local:
   :depth: 3


.. _sec:cfg-extension-configuration-overview:

Overview
========================================

The :doc:`Vector Integration Bus Extensions<../vibes/overview>` are developed by Vector and provided in binary form.
To facilitate the lookup of these binaries, specific search paths can be configured in the extensions configuration.

Configuration
--------------------


.. code-block:: javascript

    "Extensions": {
        "SearchPathHints": [
            "ENV:XYZ"
            "../extensions",
            ...
        ]
    }


.. list-table:: Extension Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - SearchPathHints
     - Optional search path hints which will be considered if an extension is loaded by the VIB,
       additionally to the default ones. A search path hint can contain the prefix "ENV:"
       to refer to an environment variable name. The default search paths are the 
       environment variable "IB_EXTENSION_PATH" and the current working directory.