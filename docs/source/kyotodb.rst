yakc API Reference
==================

.. py:module:: yakc

.. py:class:: KyotoDB(path, [mode, type, pickle])

   :param path: a path of kyoto cabinet database
   :param mode: open mode
   :param type: a type of database. One of ``"PolyDB"``, ``"TreeDB"``,
                ``"HashDB"``, ``"DirDB"``, ``"ForestDB"``. The default
                type is ``"PolyDB"``
   :param pickle: If ``True``, use pickle to store data into
                  database. If ``False``, only string type will be
                  accepted.  The default value is ``True``.

   If you want to manipulate Kyoto Cabinet database with other
   tools such as ``kchashmgr`` or other bindings, please set
   ``pickle=False``.

   If you select ``PolyDB`` as a database type, the path of
   database should ends with one of ``.kch``, ``.kct``, ``.kcd``,
   ``.kcf``. Actual database type will be selected by the suffix of
   the path. Please read `PolyDB open`_ reference to learn details.


.. _Polydb open: http://fallabs.com/kyotocabinet/api/classkyotocabinet_1_1PolyDB.html#a09384a72e6a72a0be98c80a1856f34aa      

   .. describe:: len(d)

      Return the number of items in the database.

   .. describe:: d[key]

      Return the item of *d* with key *key*.  Raises a :exc:`KeyError`
      if *key* is not in the database.

   .. describe:: key in d

      Return ``True`` if *d* has a key *key*, else ``False``. This is
      a shortcut for :meth:`has_key`.

   .. describe:: iter(d)

      Return an iterator over the keys of the database.  This is a
      shortcut for :meth:`iterkeys`.

   .. method:: keys()

      Return a list of keys.

   .. method:: items()

      Return a list of items.

   .. method:: values()

      Return a list of values.

   .. method:: iterkeys()

      Return an iterator over the keys of the database.

   .. method:: iteritems()

      Return an iterator over the items of the database.

   .. method:: itervalues()

      Return an iterator over the values of the database.

   .. method:: get(key, [default])

      Return a value if key is exists, else return default. If default
      is not given, and key is not exists, this method raises KeyError.

   .. method:: has_key(key)

      Return ``True`` if the database has a  *key*, else ``False``

   .. method:: clear()

      Remove all items from the dictionary.
               
   .. method:: size()

      Return the number of items in the database.

   .. method:: path()

      Return the path of the database.

   .. method:: pop(key[, default])


      If *key* is in the database, remove it and return its value,
      else return *default*.  If *default* is not given and *key* is
      not in dictionary, a :exc:`KeyError` is raised.

   .. method:: update([other])

      Update the database with the key/value pairs from
      *other*. Please refer the description at Python dict.

               
              
