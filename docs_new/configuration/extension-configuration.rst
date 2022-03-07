===================================================
!!! Extension Config
===================================================

.. contents:: :local:
   :depth: 3


.. _sec:cfg-extension-configuration-overview:

!!! Overview
========================================

The ExtensionConfig is an optional section in the IbConfig.json containing all extension-related
settings, e.g. for the :doc:`../vibes/vibregistry`.


.. code-block:: javascript

    "ExtensionConfig": {
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