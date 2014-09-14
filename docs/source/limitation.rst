Limitations
===========

Not implemented methods because of technical reasons
----------------------------------------------------

Some methods in dict are not implemented because of technical
reasons. These methods will not implement in the future.


.. describe:: del d[key]

   There not method to implement this feature with C.

.. method:: copy()
            
.. method:: fromkeys(seq[, value])

   Since yakc stores data values of data, we cannot implement shallow copy.

Not implemented, but will implement methods
-------------------------------------------

.. method:: setdefault(key[, default])

Value store vs. Reference store
-------------------------------

Since yakc stores a value of a data, not a reference of a data, some
codes not works like Python dict. For example, the function shown in
below results different between Python dict and yakc.

::

    >>> import yakc
    >>> def diffresult(d):
    ...     l = list()
    ...     d[1] = l
    ...     l.append(1)
    ...     return d[1]
    ... 
    >>> print diffresult(dict())
    [1]
    >>> print diffresult(yakc.KyotoDB('test.kch'))
    []

