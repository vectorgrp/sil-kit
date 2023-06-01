======================================
Data Serialization/Deserialization API
======================================

.. Macros for docs use
.. |Serializer| replace:: :cpp:class:`Serializer<SilKit::Util::SerDes::v1::Serializer>`
.. |Deserializer| replace:: :cpp:class:`Deserializer<SilKit::Util::SerDes::v1::Deserializer>`
.. contents::
   :local:
   :depth: 3

Using the Data Serialization/Deserialization API
------------------------------------------------

The `Data Serialization/Deserialization (SerDes) API` provides serialization and deserialization of data using a simple serialization scheme.
Both `Data Publish/Subscribe API` and `RPC API` support any serialization format for the transmission of the data or arguments for maximum flexibility.
However, to ensure compatibility with other simulation participants, it is strongly recommended to use `SerDes` for all data transferred via the 
:doc:`Data Publish/Subscribe API</api/pubsub>` and the :doc:`RPC API</api/rpc>`.

.. admonition:: Caution

  If other de/serialization schemes are used, it must be ensured that all other participants use this same scheme to avoid interoperability errors.

`SerDes` supports the following types: 

- Integer values: ``uint8_t``/``sint8_t`` to ``uint64_t``/``sint64_t``
- Boolean values: ``bool``
- Floating-point values: ``float``, ``double``
- Strings: ``std::string`` 
- Static and dynamic arrays aka. lists: ``std::vector<uint8_t>``
- Dynamic byte arrays: ``std::vector<uint8_t>``
- Structs
- Optional values

Unions are currently not supported.

Usage Example
~~~~~~~~~~~~~

Consider an example type from the :ref:`Publish/Subscribe Demo<sec:UTIL-pubsub-demo>`:

.. code-block:: cpp

    struct GpsData
    {
        double latitude;
        double longitude;
        std::string signalQuality;
    };

Functions to serialize and deserialize this type can be written this way:

.. code-block:: cpp

    std::vector<uint8_t> Serialize(const GpsData& gpsData)
    {
        SilKit::Util::SerDes::Serializer serializer;
        serializer.BeginStruct();
        serializer.Serialize(gpsData.latitude);
        serializer.Serialize(gpsData.longitude);
        serializer.Serialize(gpsData.signalQuality);
        serializer.EndStruct();

        return serializer.ReleaseBuffer();
    }

.. code-block:: cpp

    GpsData Deserialize(const std::vector<uint8_t>& data)
    {
        GpsData gpsData;

        SilKit::Util::SerDes::Deserializer deserializer(data);
        deserializer.BeginStruct();
        gpsData.latitude = deserializer.Deserialize<double>();
        gpsData.longitude = deserializer.Deserialize<double>();
        gpsData.signalQuality = deserializer.Deserialize<std::string>();
        deserializer.EndStruct();

        return gpsData;
    }

API and Data Type Reference
---------------------------

The |Serializer| enables the serialization of data. It encapsulates a data buffer that is initially empty, with methods to consecutively serialize data into the buffer, and a method to 
query the serialized data as a byte vector.

The |Deserializer| enables the deserialization of data. It encapsulates a data buffer that is constructed from a byte vector, and provides methods to consecutively retrieve data elements from the buffer.

Serializer API
~~~~~~~~~~~~~~

.. Use :no-link: to fix 'CRITICAL: Duplicate ID' and 'WARNING: Duplicate explicit target name' (https://github.com/breathe-doc/breathe/issues/594)
.. doxygenclass:: SilKit::Util::SerDes::v1::Serializer
   :members:
   :no-link:

Deserializer API
~~~~~~~~~~~~~~~~

.. Use :no-link: to fix 'CRITICAL: Duplicate ID' and 'WARNING: Duplicate explicit target name' (https://github.com/breathe-doc/breathe/issues/594)
.. doxygenclass:: SilKit::Util::SerDes::v1::Deserializer
   :members:
   :no-link:

Constants
~~~~~~~~~

.. doxygenfunction:: SilKit::Util::SerDes::v1::MediaTypeData

.. doxygenfunction:: SilKit::Util::SerDes::v1::MediaTypeRpc
