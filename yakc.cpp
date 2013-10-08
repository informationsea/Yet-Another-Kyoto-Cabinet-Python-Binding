#include <string.h>

#include <Python.h>
#include <kcpolydb.h>

enum KyotoCursorType {
    KYOTO_KEY, KYOTO_VALUE, KYOTO_ITEMS
};

typedef struct {
    PyObject_HEAD
    kyotocabinet::BasicDB *m_db;
    bool use_pickle;
} KyotoDB;

typedef struct {
    PyObject_HEAD
    KyotoDB *m_db;
    kyotocabinet::BasicDB::Cursor *m_cursor;
    enum KyotoCursorType m_type;
} KyotoCursor;

extern PyTypeObject yakc_CursorType;
int Cursor_init(KyotoCursor *self, PyObject *args, PyObject *kwds);


class AutoPythonRef
{
private:
    PyObject* m_p;
    bool m_increment;
public:
    explicit AutoPythonRef(PyObject* p = NULL, bool increment = false) :
        m_p(p), m_increment(increment) {
        
        if (increment) {
            Py_INCREF(p);
        }
    }
    ~AutoPythonRef() {
        if (m_p)
            Py_DECREF(m_p);
    }

    AutoPythonRef& operator=(PyObject *newp) {
        if (m_increment)
            Py_INCREF(newp);
        if (m_p)
            Py_DECREF(m_p);
        m_p = newp;
        return *this;
    }

    AutoPythonRef& operator++() {
        Py_INCREF(m_p);
        return *this;
    }

    AutoPythonRef& operator--() {
        Py_DECREF(m_p);
        return *this;
    }

    bool operator==(PyObject *obj) {return m_p == obj;}
    bool operator!=(PyObject *obj) {return m_p != obj;}
    operator PyObject*() {return m_p;}
    
    PyObject* operator->() const {return m_p;}
    PyObject* get() const {return m_p;}
};

typedef AutoPythonRef APR;


static PyObject *pickle_dumps;
static PyObject *pickle_loads;

/* Internal use only */
static PyObject *
KyotoDB_load(std::string data, bool use_pickle)
{
    if (use_pickle) {
        APR pydata(PyString_FromStringAndSize(data.data(), data.size()));
        if (PyErr_Occurred() != NULL)
            return NULL;
        return PyObject_CallFunctionObjArgs(pickle_loads, (PyObject *)pydata, NULL);
    } else {
        return PyString_FromStringAndSize(data.data(), data.size());
    }
}

static std::string
KyotoDB_dump(PyObject *obj, bool use_pickle, bool *ok)
{
    *ok = false;
    if (use_pickle) {
        APR pydata(PyObject_CallFunctionObjArgs(pickle_dumps, obj, NULL));
        if (PyErr_Occurred() != NULL)
            return "";
        
        char *buffer;
        Py_ssize_t size;
        if (PyString_AsStringAndSize(pydata, &buffer, &size) < 0) {
            return "";
        }
        *ok = true;
        return std::string(buffer, size);
    } else {
        char *buffer;
        Py_ssize_t size;
        if (PyString_AsStringAndSize(obj, &buffer, &size) < 0) {
            return "";
        }
        *ok = true;
        return std::string(buffer, size);
    }
}

/* ---------------- KyotoDB -------------------*/

static void
KyotoDB_dealloc(KyotoDB *self)
{
    delete self->m_db;
}

static PyObject*
KyotoDB_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    KyotoDB *self;
    self = (KyotoDB *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->m_db = NULL;
    }
    return (PyObject *)self;
}

static int
KyotoDB_init(KyotoDB *self, PyObject *args, PyObject *kwds)
{
    static char* kwlist[5] = {
        strdup("path"), strdup("mode"), strdup("type"), strdup("pickle"), NULL
    };
    
    const char *path = NULL;
    int mode = kyotocabinet::BasicDB::OWRITER|kyotocabinet::BasicDB::OCREATE;
    const char *type = NULL;
    int pickle = true;

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|sisi", kwlist,
                                      &path, &mode, &type, &pickle))
        return -1;

    // printf("path: %s\nmode: %d\ntype: %s\npickle: %d\n",
    //        path, mode, type, pickle);

    if (type == NULL)
        self->m_db = new kyotocabinet::PolyDB();
    else if (strcmp(type, "TreeDB") == 0)
        self->m_db = new kyotocabinet::TreeDB();
    else if (strcmp(type, "HashDB") == 0)
        self->m_db = new kyotocabinet::HashDB();
    else if (strcmp(type, "DirDB") == 0)
        self->m_db = new kyotocabinet::DirDB();
    else if (strcmp(type, "ForestDB") == 0)
        self->m_db = new kyotocabinet::ForestDB();
    else if (strcmp(type, "PolyDB") == 0)
        self->m_db = new kyotocabinet::PolyDB();
    else {
        APR str(PyString_FromFormat("Database %s is not supported", type));
        PyErr_SetObject(PyExc_RuntimeError, str);
        return -1;
    }

    if (self->m_db == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot create database object");
        return -1;
    }

    bool suceed = self->m_db->open(path, mode);
    if (!suceed) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot open database");
        return -1;
    }

    self->use_pickle = pickle;

    return 0;
}

static PyObject *
KyotoDB_size(KyotoDB *self)
{
    PyObject *result;
    result = PyInt_FromLong(self->m_db->count());
    return result;
}

static Py_ssize_t
KyotoDB__len__(KyotoDB *self)
{
    return self->m_db->count();
}

static int
KyotoDB__set__(KyotoDB *self, PyObject* key, PyObject *v)
{
    bool ok;
    std::string ckey = KyotoDB_dump(key, self->use_pickle, &ok);
    if (!ok)
        return 0;

    std::string cvalue = KyotoDB_dump(v, self->use_pickle, &ok);
    if (!ok)
        return 0;

    if (self->m_db->set(ckey, cvalue))
        return 0;

    PyErr_SetString(PyExc_RuntimeError, "KyotoCabinet Error");
    return 0;
}

static PyObject *
KyotoDB__get__(KyotoDB *self, PyObject *key)
{
    bool ok;
    std::string ckey = KyotoDB_dump(key, self->use_pickle, &ok);
    if (!ok) return 0;

    std::string value;
    bool success = self->m_db->get(std::string(ckey), &value);
    if (success) {
        return KyotoDB_load(value, self->use_pickle);
    }
    PyErr_SetObject(PyExc_KeyError, key);
    return NULL;
}

static PyObject *
KyotoDB_path(KyotoDB *self)
{
    PyObject *result;
    std::string path = self->m_db->path();
    result = PyString_FromStringAndSize(path.data(), path.size());
    return result;
}

static PyObject *
KyotoDB_del(KyotoDB *self, PyObject *args, PyObject *kwds)
{
    PyObject *result;
    static char* kwlist[7] = {
        strdup("key"), NULL
    };

    PyObject* pykey;

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "o", kwlist,
                                      &pykey)) {
        return NULL;
    }

    bool ok;
    std::string ckey = KyotoDB_dump(pykey, self->use_pickle, &ok);
    if (!ok) return 0;

    result = PyBool_FromLong(self->m_db->remove(ckey));
    return result;
}

static PyObject *
KyotoDB_array(KyotoDB *self, int type)
{
    PyObject *result = PyList_New(self->m_db->count());
    Py_ssize_t i = 0;

    kyotocabinet::BasicDB::Cursor *cursor = self->m_db->cursor();
    cursor->jump();
    std::string key;
    std::string value;
    while (cursor->get(&key, &value, true)) {
        switch (type) {
        case 0: {
            PyObject *pkey = KyotoDB_load(key, self->use_pickle);
            if (pkey == NULL)
                goto onerror;
            PyList_SetItem(result, i++, pkey);
            break;
        }
        case 1: {
            PyObject *pvalue = KyotoDB_load(value, self->use_pickle);
            if (pvalue == NULL)
                goto onerror;
            PyList_SetItem(result, i++, pvalue);
            break;
        }
        case 2: {
            APR pkey(KyotoDB_load(key, self->use_pickle));
            if (pkey == NULL)
                goto onerror;

            APR pvalue(KyotoDB_load(value, self->use_pickle));
            if (pvalue == NULL)
                goto onerror;

            PyList_SetItem(result, i++,
                           PyTuple_Pack(2, (PyObject *)pkey, (PyObject *)pvalue));
            break;
        }
        }
    }
    delete cursor;
    return result;
  onerror:
    for (Py_ssize_t j = 0; j < i; j++) {
        PyObject *obj = PyList_GetItem(result, j);
        Py_DECREF(obj);
    }
    delete cursor;
    return NULL;
}

static PyObject *
KyotoDB_keys(KyotoDB *self)
{
    return KyotoDB_array(self, 0);
}

static PyObject *
KyotoDB_items(KyotoDB *self)
{
    return KyotoDB_array(self, 2);
}

static PyObject *
KyotoDB_values(KyotoDB *self)
{
    return KyotoDB_array(self, 1);
}

static PyObject *
KyotoDB_close(KyotoDB *self)
{
    self->m_db->close();
    Py_RETURN_NONE;
}

static PyObject *
KyotoDB_iter(KyotoDB *self)
{
    KyotoCursor *cursor = PyObject_New(KyotoCursor, &yakc_CursorType);
    APR tuple(PyTuple_Pack(1, self));
    int re = Cursor_init(cursor, tuple.get(), NULL);
    if (re == 0)
        return (PyObject *)cursor;
    PyErr_SetString(PyExc_RuntimeError, "Cannot create cursor");
    return NULL;
}

static PyObject *
KyotoDB_itervalues(KyotoDB *self)
{
    KyotoCursor *cursor = PyObject_New(KyotoCursor, &yakc_CursorType);
    APR type(PyInt_FromLong((long)KYOTO_VALUE));
    APR tuple(PyTuple_Pack(2, self, (PyObject *)type));
    int re = Cursor_init(cursor, tuple.get(), NULL);
    if (re == 0)
        return (PyObject *)cursor;
    PyErr_SetString(PyExc_RuntimeError, "Cannot create cursor");
    return NULL;
}

static PyObject *
KyotoDB_iteritems(KyotoDB *self)
{
    KyotoCursor *cursor = PyObject_New(KyotoCursor, &yakc_CursorType);
    APR type(PyInt_FromLong((long)KYOTO_ITEMS));
    APR tuple(PyTuple_Pack(2, self, type.get()));
    int re = Cursor_init(cursor, tuple.get(), NULL);
    if (re == 0)
        return (PyObject *)cursor;
    PyErr_SetString(PyExc_RuntimeError, "Cannot create cursor");
    return NULL;
}

static int
KyotoDB_contains(KyotoDB *self, PyObject *obj)
{
    bool ok;
    std::string ckey = KyotoDB_dump(obj, self->use_pickle, &ok);
    if (ok)
        return self->m_db->check(ckey) < 0 ? 0 : 1;
    return 0;
}

static PyObject *
KyotoDB_has_key(KyotoDB *self, PyObject *obj)
{
    if (KyotoDB_contains(self, obj))
        return PyBool_FromLong(1);
    return PyBool_FromLong(0);
}

static PyObject *
KyotoDB_clear(KyotoDB *self)
{
    kyotocabinet::BasicDB::Cursor *cursor = self->m_db->cursor();
    if (cursor == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot create cursor");
        return 0;
    }
    cursor->jump();
    while (cursor->remove());
    delete cursor;
    Py_RETURN_NONE;
}

static PyObject *
KyotoDB_pop(KyotoDB *self, PyObject *args, PyObject *kwds)
{
    static char* kwlist[5] = {
        strdup("key"), strdup("default"), NULL
    };

    PyObject *key = NULL;
    PyObject *defaultvalue = NULL;
    
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist,
                                      &key, &defaultvalue))
        return 0;

    bool ok;
    std::string ckey = KyotoDB_dump(key, self->use_pickle, &ok);
    if (!ok)
        return 0;

    std::string value;
    if (self->m_db->seize(ckey, &value))
        return KyotoDB_load(value, self->use_pickle);

    if (defaultvalue != NULL) {
        Py_INCREF(defaultvalue);
        return defaultvalue;
    }

    PyErr_SetObject(PyExc_KeyError, key);
    return 0;
}

static bool
KyotoDB_update_with_mapping(KyotoDB *self, PyObject *mapping)
{
    APR iterator(PyObject_GetIter(mapping));
    APR item(NULL);

    if (iterator == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "object is not iterable");
        return false;
    }

    while ((item = PyIter_Next(iterator.get())) != NULL) {
        APR value(PyObject_GetItem(mapping, item.get()));
        if (value == NULL) {
            PyErr_SetString(PyExc_RuntimeError, "Value is not found");
            return false;
        }
        bool ok;
        std::string ckey = KyotoDB_dump(item, self->use_pickle, &ok);
        if (!ok)
            return false;

        std::string cvalue = KyotoDB_dump(value, self->use_pickle, &ok);
        if (!ok)
            return false;
        
        self->m_db->set(ckey, cvalue);
    }

    return true;
}

static PyObject *
KyotoDB_update(KyotoDB *self, PyObject *args, PyObject *kwds)
{
    Py_ssize_t size = PyTuple_Size(args);
    if (size > 1) {
        APR str(PyString_FromFormat("update expected at most 1 arguments, got %zd", size));
        PyErr_SetObject(PyExc_TypeError, str.get());
        return NULL;
    }

    if (size == 1) {
        APR obj(PyTuple_GetItem(args, 0));

        if (PyMapping_Check(obj.get())) {
            if (!KyotoDB_update_with_mapping(self, obj.get()))
                return NULL;
        } else {
            APR iterator (PyObject_GetIter(obj.get()));
            APR item;

            if (iterator == NULL) {
                PyErr_SetString(PyExc_RuntimeError, "object is not iterable");
                return NULL;
            }

            Py_ssize_t i = 0;
            while ((item = PyIter_Next(iterator.get())) != NULL) {
                if (!PySequence_Check(item.get())) {
                    APR str(PyString_FromFormat("cannot convert dictionary update sequence element #%zd to a sequence", i));
                    PyErr_SetObject(PyExc_TypeError, str.get());
                    return NULL;
                }
                if (PySequence_Size(item.get()) != 2) {
                    APR str(PyString_FromFormat("dictionary update sequence element #%zd has length %zd; 2 is required",
                                                i, PySequence_Size(item.get())));
                    PyErr_SetObject(PyExc_TypeError, str.get());
                    return NULL;
                }
                APR key(PySequence_GetItem(item.get(), 0));
                APR value(PySequence_GetItem(item.get(), 1));

                bool ok;
                std::string ckey = KyotoDB_dump(key, self->use_pickle, &ok);
                if (!ok)
                    return NULL;

                std::string cvalue = KyotoDB_dump(value, self->use_pickle, &ok);
                if (!ok)
                    return NULL;

                self->m_db->set(ckey, cvalue);
                i++;
            }
        }
    }

    if (kwds) {
        if (!KyotoDB_update_with_mapping(self, kwds))
            return NULL;
    }
    
    Py_RETURN_NONE;
}


static PyMethodDef KyotoDB_methods[] = {
    {"size", (PyCFunction)KyotoDB_size, METH_NOARGS,
     "Get size of database"},
    {"path", (PyCFunction)KyotoDB_path, METH_NOARGS,
     "Get path of database"},
    {"remove", (PyCFunction)KyotoDB_del, METH_KEYWORDS,
     "Delete value"},
    {"keys", (PyCFunction)KyotoDB_keys, METH_NOARGS,
     "List of keys"},
    {"values", (PyCFunction)KyotoDB_values, METH_NOARGS,
     "List of keys"},
    {"items", (PyCFunction)KyotoDB_items, METH_NOARGS,
     "List of keys"},
    {"iterkeys", (PyCFunction)KyotoDB_iter, METH_NOARGS,
     "Iterator of keys"},
    {"itervalues", (PyCFunction)KyotoDB_itervalues, METH_NOARGS,
     "Iterator of keys"},
    {"iteritems", (PyCFunction)KyotoDB_iteritems, METH_NOARGS,
     "Iterator of keys"},
    {"close", (PyCFunction)KyotoDB_close, METH_NOARGS,
     "close database"},
    {"clear", (PyCFunction)KyotoDB_clear, METH_NOARGS,
     "remove all items"},
    {"has_key", (PyCFunction)KyotoDB_has_key, METH_O,
     "remove all items"},
    {"pop", (PyCFunction)KyotoDB_pop, METH_KEYWORDS,
     "pop item"},
    {"update", (PyCFunction)KyotoDB_update, METH_KEYWORDS,
     "update item"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static PyMappingMethods KyotoDB_mapping = {
    (lenfunc)KyotoDB__len__,
    (binaryfunc)KyotoDB__get__,
    (objobjargproc)KyotoDB__set__,
};

static PySequenceMethods KyotoDB_sequence = {
    (lenfunc)KyotoDB__len__,
    NULL,                       // concatenate
    NULL,                       // repeat
    NULL,                       // get item
    NULL,                       // slice
    NULL,                       // set item
    NULL,                       // slice
    (objobjproc)KyotoDB_contains,
    NULL,                       // in-place concatenate
    NULL                        // in-place repeat
};

// static PyMemberDef KyotoDB_members[] = {
//     {NULL}  /* Sentinel */
// };

static PyTypeObject KyotoDBType = {
    PyObject_HEAD_INIT(NULL)
    0,                          /*ob_size*/
    "yakc.KyotoDB",              /*tp_name*/
    sizeof(KyotoDB),            /*tp_basicsize*/
    0,                          /*tp_itemsize*/
    (destructor)KyotoDB_dealloc, /*tp_dealloc*/
    0,                          /*tp_print*/
    0,                          /*tp_getattr*/
    0,                          /*tp_setattr*/
    0,                          /*tp_compare*/
    0,                          /*tp_repr*/
    0,                          /*tp_as_number*/
    &KyotoDB_sequence,          /*tp_as_sequence*/
    &KyotoDB_mapping,           /*tp_as_mapping*/
    0,                          /*tp_hash */
    0,                          /*tp_call*/
    0,                          /*tp_str*/
    0,                          /*tp_getattro*/
    0,                          /*tp_setattro*/
    0,                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Kyoto Cabinet",     /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    (getiterfunc)KyotoDB_iter,  /* tp_iter */
    0,                          /* tp_iternext */
    KyotoDB_methods,            /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)KyotoDB_init,     /* tp_init */
    0,                          /* tp_alloc */
    KyotoDB_new,                /* tp_new */
};

/* ---------------- Cursor -------------------*/

static void
Cursor_dealloc(KyotoCursor* self)
{
    Py_DECREF(self->m_db);
    if (self->m_cursor)
        delete self->m_cursor;
}

static PyObject *
Cursor_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    KyotoCursor *self;
    self = (KyotoCursor *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->m_cursor = NULL;
        self->m_type = KYOTO_KEY;
    }

    return (PyObject *) self;
}

int Cursor_init(KyotoCursor *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {strdup("db"), strdup("type"), NULL};
    PyObject *db = NULL;
    int type = KYOTO_KEY;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist, &db, &type)) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot create cursor");
        return -1;
    }

    if (!db) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot create cursor");
        return -1;
    }

    if (strcmp(db->ob_type->tp_name, "yakc.KyotoDB") != 0) {
        PyErr_SetString(PyExc_TypeError, "First argument should be KyotoDB");
        return -1;
    }

    KyotoDB *kyotodb = (KyotoDB *)db;
    self->m_db = kyotodb;
    Py_INCREF((PyObject *)self->m_db);

    self->m_cursor = kyotodb->m_db->cursor();
    self->m_cursor->jump();
    self->m_type = (enum KyotoCursorType)type;

    return 0;
}

static PyObject *
Cursor_iter(KyotoCursor *self)
{
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *
Cursor_next(KyotoCursor *self)
{
    std::string key;
    std::string value;
    bool succeed = self->m_cursor->get(&key, &value, true);
    if (succeed) {
        switch (self->m_type) {
        case KYOTO_VALUE: {
            return KyotoDB_load(value, self->m_db->use_pickle);
        }
        case KYOTO_ITEMS:{
            APR pkey(KyotoDB_load(key, self->m_db->use_pickle));
            APR pvalue(KyotoDB_load(value, self->m_db->use_pickle));
            return PyTuple_Pack(2, (PyObject *)pkey, (PyObject *)pvalue);
        }
        case KYOTO_KEY:
        default:
            return KyotoDB_load(key, self->m_db->use_pickle);
        }
    }

    PyErr_SetString(PyExc_StopIteration, "");
    return NULL;
}


PyTypeObject yakc_CursorType = {
    PyObject_HEAD_INIT(NULL)
    0,                          /*ob_size*/
    "yakc.Cursor",              /*tp_name*/
    sizeof(KyotoCursor),        /*tp_basicsize*/
    0,                          /*tp_itemsize*/
    (destructor)Cursor_dealloc, /*tp_dealloc*/
    0,                          /*tp_print*/
    0,                          /*tp_getattr*/
    0,                          /*tp_setattr*/
    0,                          /*tp_compare*/
    0,                          /*tp_repr*/
    0,                          /*tp_as_number*/
    0,                          /*tp_as_sequence*/
    0,                          /*tp_as_mapping*/
    0,                          /*tp_hash */
    0,                          /*tp_call*/
    0,                          /*tp_str*/
    0,                          /*tp_getattro*/
    0,                          /*tp_setattro*/
    0,                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Kyoto DB Cursor",          /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    (getiterfunc)Cursor_iter,   /* tp_iter */
    (iternextfunc)Cursor_next,  /* tp_iternext */
    0,                          /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)Cursor_init,      /* tp_init */
    0,                          /* tp_alloc */
    Cursor_new,                 /* tp_new */
};

static PyMethodDef module_methods[] = {
    {NULL}  /* Sentinel */
};

#ifndef PyMODINIT_FUNC        /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

PyMODINIT_FUNC
inityakc(void)
{
    PyObject *m;

    if (PyType_Ready(&KyotoDBType) < 0)
        return;

    m = Py_InitModule("yakc", module_methods);
    if (m == NULL)
        return;

    Py_INCREF(&KyotoDBType);
    PyModule_AddObject(m, "KyotoDB", (PyObject *)&KyotoDBType);

    if (PyType_Ready(&yakc_CursorType) < 0)
        return;

    Py_INCREF(&yakc_CursorType);
    PyModule_AddObject(m, "Cursor", (PyObject *)&yakc_CursorType);

    APR cpickle_name(PyString_FromString("cPickle"));
    APR cpickle(PyImport_Import(cpickle_name));

    if (cpickle == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot import cpickle");
        return;
    }

    pickle_dumps = PyObject_GetAttrString(cpickle, "dumps");
    pickle_loads = PyObject_GetAttrString(cpickle, "loads");

    if (pickle_dumps == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot find cpickle.dumps");
        return;
    }

    if (pickle_loads == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot find cpickle.loads");
        return;
    }
}

