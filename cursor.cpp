#include "cursor.hpp"
#include "yakc.hpp"
#include "yakcmem.hpp"
#include <string.h>
#include <string>

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

    if (strcmp(db->ob_type->tp_name, "yakc.TreeDB") != 0) {
        PyErr_SetString(PyExc_TypeError, "First argument should be KyotoDB");
        return -1;
    }

    KyotoDB *kyotodb = (KyotoDB *)db;
    self->m_db = (PyObject *)kyotodb;
    Py_INCREF(self->m_db);

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
        case KYOTO_VALUE:
            return PyString_FromString(value.c_str());
        case KYOTO_ITEMS:{
            APR pkey(PyString_FromString(key.c_str()));
            APR pvalue(PyString_FromString(value.c_str()));
            return PyTuple_Pack(2, (PyObject *)pkey, (PyObject *)pvalue);
        }
        case KYOTO_KEY:
        default:
            return PyString_FromString(key.c_str());
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



void inityakc_cursor(PyObject *m)
{
    if (PyType_Ready(&yakc_CursorType) < 0)
        return;

    Py_INCREF(&yakc_CursorType);
    PyModule_AddObject(m, "Cursor", (PyObject *)&yakc_CursorType);
}
