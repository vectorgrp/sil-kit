==============
IO Service API
==============

The IO service API allows the creation of input and ouput ports. The IO Ports
are connected in a 1-to-N fashion, i.e., the values of one outport can be
distributed to multiple input ports. A number of different data types are
supported for the IO ports:

.. list-table:: I/O Port Data Type
 :widths: 20 80

 * - Digital
   - Boolean value (``bool``)
 * - Analog
   - Scalar value (``double``)
 * - PWM
   - Pulse width modulation value consisting of frequency and duty cycle (:cpp:class:`PwmValue<ib::sim::io::PwmValue>`)
 * - Pattern
   - Byte vector

Output and input ports are connected by
:ref:`links<sec:cfg-links>`. Participants can write new values to the *out*
ports and receive value updates on the *in* ports. That is, an instance of
:cpp:class:`IOutPort<template\<typename MsgT>ib::sim::io::IOutPort>` is
connected to one or more instances of :cpp:class:`IInPort<template\<typename
MsgT>ib::sim::io::IInPort>`.

Both OutPorts and InPorts are implemented as C++ templates, and exchange
simulation messages of type :cpp:class:`IoMessage\<T\><template\<typename
ValueT> ib::sim::io::IoMessage>` to model the value changes.  In contrast to
generic messages, IoMessages can be annotated with timing information. However,
IoMessages are always delivered immediately to connected InPorts independently
of the annotated delay. I.e., it is up to the receivers implementation to honor
the timing information if desired.


Using the IO Service API
-----------------------------
IO Ports can be created directly on an instance of an 
:cpp:class:`IComAdapter<ib::mw::IComAdapter>` interface::

   auto* dioOut = comAdapter->CreateDigitalOut("DIO");
   auto* dioIn = comAdapter->CreateDigitalIn("DIO");

Ports are connected by giving them the same, canonical name when creating corresponding In- and OutPorts.

Sending Data
~~~~~~~~~~~~~~~~~~~~~~~~~~
To send data, the generic :cpp:func:`Write()<ib::sim::io::IOutPort::Write()>` method can be
invoked on the OutPort::

   dioOut->Write(true, std::chrono::nanoseconds{10});

In this example, the IDigitalOutPort takes a boolean value of true.
The port's value and timestamp are combined into an
:cpp:class:`IO message<template\<typename ValueT>ib::sim::io::IoMessage>` which is then transmitted
through the connected :cpp:class:`IComAdapter<ib::mw::IComAdapter>`.

.. admonition:: Note

   OutPorts retain the last value written to them and can have a :cpp:struct:`configured initial value<ib::cfg::IoPort>`.

The :cpp:func:`Read()<ib::sim::io::IOutPort::Read()>` method returns the last value that was
written to the OutPort or an initial value, if it was configured in the VIB Config.

Receiving Data
~~~~~~~~~~~~~~~~~~~~~~~~~~
To receive data, the generic :cpp:func:`Read()<ib::sim::io::IInPort::Read()>` method can be
invoked on an IInPort instance::

   auto data = dioIn->Read();

Callbacks can be registered to get notifications whenever a new IO message or value is available.
When the timestamp of the data is required, the :cpp:type:`MessageHandler<ib::sim::io::IInPort::MessageHandler>`
can be used to retrieve the complete IO message.
When only the data value is of interest the :cpp:type:`ValueHandler<ib::sim::io::IInPort::ValueHandler>` can be used::

   //register reception handlers
   dioIn->RegisterHandler([](IInPort* port, const bool& data) {
      // handle data in callback
   });

IO Port Configuration
~~~~~~~~~~~~~~~~~~~~~
The current configuration of an IO port can be retreived using
the :cpp:func:`IInPort::Config()<ib::sim::io::IInPort::Config()>`
and :cpp:func:`IOutPort::Config()<ib::sim::io::IOutPort::Config()>` methods, respectively.
It returns a VIB :cpp:func:`configuration object<template\<typename ValueT> ib::cfg::IoPort>` which contains information
such as the initial value and the name.

Usage Examples
~~~~~~~~~~~~~~
To demonstrate the usage of the IO ports a simple application that updates analog values is presented here:

.. literalinclude::
    examples/io/Analog_Ports.cpp
    :language: cpp
    :lines: 16-63

The application creates two threads, one data writer and one reader.
The writer thread updates the analog value in every simulation step.
Value changes trigger a notification in the reader thread, which invokes its callback.

The complete source code of this sample: :download:`Analog_Ports.cpp<examples/io/Analog_Ports.cpp>`

API and Data Type Reference
--------------------------------------------------
The :cpp:class:`IInPort<template\<typename MsgT> ib::sim::io::IInPort>` and
:cpp:class:`IOutPort<template\<typename MsgT> ib::sim::io::IOutPort>` interfaces are generic.
Specializations for all supported message types exist.
The underlying :cpp:class:`IoMessage<template\<typename ValueT> ib::sim::io::IoMessage>` that is used 
by the VIB internally is accessible for users.
One use case is the registration of message handlers.

Output Ports
~~~~~~~~~~~~~~
    .. doxygenclass:: ib::sim::io::IOutPort
       :members:

    .. doxygentypedef:: ib::sim::io::IDigitalOutPort

    .. doxygentypedef:: ib::sim::io::IAnalogOutPort

    .. doxygentypedef:: ib::sim::io::IPwmOutPort

    .. doxygentypedef:: ib::sim::io::IPatternOutPort

Input Ports
~~~~~~~~~~~~~~
    .. doxygenclass:: ib::sim::io::IInPort
       :members:

    .. doxygentypedef:: ib::sim::io::IDigitalInPort

    .. doxygentypedef:: ib::sim::io::IAnalogInPort

    .. doxygentypedef:: ib::sim::io::IPwmInPort

    .. doxygentypedef:: ib::sim::io::IPatternInPort

IO Message Types
~~~~~~~~~~~~~~~~
    .. doxygenstruct:: ib::sim::io::IoMessage
       :members:

    .. doxygentypedef:: ib::sim::io::AnalogIoMessage

    .. doxygentypedef:: ib::sim::io::DigitalIoMessage

    .. doxygentypedef:: ib::sim::io::PatternIoMessage

    .. doxygentypedef:: ib::sim::io::PwmIoMessage

    .. doxygenstruct:: ib::sim::io::PwmValue
       :members:
