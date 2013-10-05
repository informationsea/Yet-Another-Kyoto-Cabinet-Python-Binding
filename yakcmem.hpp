#ifndef _YAKCMEM_H_
#define _YAKCMEM_H_

#include <Python.h>

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

#endif /* _YAKCMEM_H_ */
