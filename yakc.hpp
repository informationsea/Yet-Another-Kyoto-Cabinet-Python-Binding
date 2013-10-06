#ifndef _YAKC_H_
#define _YAKC_H_

#include <Python.h>
#include <kcpolydb.h>

typedef struct {
    PyObject_HEAD
    kyotocabinet::BasicDB *m_db;
} KyotoDB;

#endif /* _YAKC_H_ */
