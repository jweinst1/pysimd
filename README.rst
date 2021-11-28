PySIMD
======

.. image:: images/pysimd.png
  :width: 800
  :alt: PySimd logo

``simd`` is the python module for SIMD computing and programming. It
prodives an extensive interface to SIMD instruction sets on several different 
architectures, and fallback scalar implementations when no SIMD instructions 
are available for a particular operation. ``simd`` provides vector objects, 
which are collections of bytes aligned on a minimum boundary. The main advantage
of using SIMD instructions is the potentially masive performance
improvements over regular, scalar instructions.

``simd`` is a C extension, that is only compatible with Python 3. When
built, it will do compile time checks to see what SIMD instructions are
available on the current CPU. The advantage of using ``simd`` over other
libraries or implementations is this module auto-detects the best
instructions available for specific operations at install time and uses
them anytime the module is imported into a Python program.

Installation
------------

To install ``simd``, run the following ``pip`` command

::

    $ pip install simd

Tests
-----

To run the tests, run the following command

::

    $ python tests

This will build the ``.c`` test files , link the python library into each one, and run them.

Usage
-----

The ``simd`` module can be used primarily through vector objects. Vector
objects are special C objects that contain a portion of bytes aligned on
at least a 16 byte boundary. This alignment allows SIMD operations to be
performed at a higher scale, without needing to worry about the leftover
bytes at the end of a data segment.

Creation
~~~~~~~~

You can make an empty vector with a size

.. code:: py

    >>> import simd
    >>> a = simd.Vec(size=32)
    >>> a
    [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]

Or make a vector from a repeated value

.. code:: py

    >>> a = simd.Vec(size=32, repeat_value=64, repeat_size=2)
    >>> a
    [40,0,40,0,40,0,40,0,40,0,40,0,40,0,40,0,40,0,40,0,40,0,40,0,40,0,40,0,40,0,40,0]

Note: the ``__repr__`` method of ``Vec`` , implemented in C, displays a
hexadecimal byte representation of the vector.

However, if a size used cannot be aligned by 16 bytes, an error is
thrown

.. code:: py

    >>> a = simd.Vec(size=31, repeat_value=64, repeat_size=2)
    Traceback (most recent call last):
    File "<stdin>", line 1, in <module>
    simd.SimdError: The size '31' cannot be aligned by at least 16 bytes

Operations
~~~~~~~~~~

The ``simd`` module supports a wide variety of operations, such as plain
addition:

.. code:: py

    >>> a = simd.Vec(size=32, repeat_value=1, repeat_size=1)
    >>> b = simd.Vec(size=32, repeat_value=1, repeat_size=1)
    >>> a
    [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
    >>> b
    [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
    >>> a.add(b, width=1)
    >>> a
    [2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2]

There are also operations to retrieve the data in the vector back as
python types, such as with ``as_bytes()``

.. code:: py

    >>> a = simd.Vec(size=16, repeat_value=4, repeat_size=2)
    >>> a
    [4,0,4,0,4,0,4,0,4,0,4,0,4,0,4,0]
    >>> a.as_bytes(start = 3)
    b'\x00\x04\x00\x04\x00\x04\x00\x04\x00\x04\x00\x04\x00'
    >>> a.as_bytes()
    b'\x04\x00\x04\x00\x04\x00\x04\x00\x04\x00\x04\x00\x04\x00\x04\x00'
    >>> a.as_bytes(start=40)
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
    simd.SimdError: start: '40', is out of bounds for vector of size 16

The data inside a vector can also be retrieved as a collection type, like a ``tuple`` , 

.. code:: py

    >>> a = simd.Vec(size=32, repeat_value=5, repeat_size=4)
    >>> a
    [5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0]
    >>> a.as_tuple(type=int, width=4)
    (5, 5, 5, 5, 5, 5, 5, 5)
    >>> a.as_tuple(type=int, width=1)
    (5, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0)
    >>> a.as_tuple(type=int, width=8)
    (21474836485, 21474836485, 21474836485, 21474836485)

The above example shows the pure ``__repr__`` method of ``Vec`` only depicts a hexadecimal, byte level representation of the vector data, but a method like ``as_tuple`` allows the viewing of data with different types. One unique aspect of the ``simd`` module is it treats data and memory similar to that of C, where a chunk of 16 bytes could be two 64 bit integers, four 32 bit integers, and so on.


Math
~~~~

The ``simd`` module supports simd operations that involve artihmetic and math on integers and floating point numbers. Operations like ``add`` or ``sub`` work off another vector and a ``width``. The ``width`` indicates the width of the data lane the simd instruction applies to, such as ``8`` for 64 bit operations. Here are a few examples:

.. code:: py

    >>> v = simd.Vec(size=16, repeat_value=5, repeat_size=4)
    >>> v2 = simd.Vec(size=16, repeat_value=10, repeat_size=4)
    >>> v.add(v2)
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
    TypeError: function missing required argument 'width' (pos 2)
    >>> v.add(v2, width=4)
    >>> v.as_tuple(type=int, width=4)
    (15, 15, 15, 15)
    >>> v.sub(v2, width=4)
    >>> v.sub(v2, width=4)
    >>> v.as_tuple(type=int, width=4)
    (-5, -5, -5, -5)


