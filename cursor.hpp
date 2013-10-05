#ifndef _CURSOR_H_
#define _CURSOR_H_

#include <Python.h>
#include <kcpolydb.h>

enum KyotoCursorType {
    KYOTO_KEY, KYOTO_VALUE, KYOTO_ITEMS
};

typedef struct {
    PyObject_HEAD
    PyObject *m_db;
    kyotocabinet::BasicDB::Cursor *m_cursor;
    enum KyotoCursorType m_type;
} KyotoCursor;

extern PyTypeObject yakc_CursorType;

int Cursor_init(KyotoCursor *self, PyObject *args, PyObject *kwds);

void inityakc_cursor(PyObject *m);

#endif /* _CURSOR_H_ */
