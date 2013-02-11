#ifndef SWOBJECT_HPP
#define SWOBJECT_HPP

#ifdef SWIGTCL

#ifdef SWIG
%typemap(out) SWObject {
	Tcl_SetObjResult(interp, $1.getObj());
}   

%typemap(out) SWList {
	Tcl_SetObjResult(interp, $1.getObj());
}   

%typemap(out) SWDict {
	Tcl_SetObjResult(interp, $1.getObj());
}   


%{
#include "SWObject.hpp"
%}

#else
// C++-compiler Tcl
#include <tcl.h>
#include <tclTomMath.h>
#include <string>

// create Tcl_Obj by overloaded functions
class SWList;
class SWDict;

inline Tcl_Obj* MakeBaseSWObj() {
    // create empty object. Easy in Tcl - Python?
    return Tcl_NewObj();
}

inline Tcl_Obj* MakeBaseSWObj(int i) {
    // create integer object
    return Tcl_NewIntObj(i);
}

inline Tcl_Obj* MakeBaseSWObj(long i) {
    // create integer object
    return Tcl_NewLongObj(i);
}

inline Tcl_Obj* MakeBaseSWObj(long long i) {
    // create integer object
    return Tcl_NewWideIntObj(i);
}

inline Tcl_Obj* MakeBaseSWObj(unsigned long long i) {
    // create integer object
    return Tcl_NewWideIntObj(i); /** potential overflow **/
}

inline Tcl_Obj* MakeBaseSWObj(float d) {
    // create double object.
    return Tcl_NewDoubleObj(d);
}


inline Tcl_Obj* MakeBaseSWObj(double d) {
    // create double object.
    return Tcl_NewDoubleObj(d);
}

inline Tcl_Obj* MakeBaseSWObj(const std::string &s) {
    // create string object
    return Tcl_NewStringObj(s.c_str(), s.size());
}


Tcl_Obj* MakeBaseSWObj(const SWList &o);

Tcl_Obj* MakeBaseSWObj(const SWDict &o);
    
class SWObject {
protected:
    mutable Tcl_Obj *ptr;
public:
    SWObject() : ptr(NULL) { }

    template <typename TObject> 
    SWObject(const TObject& obj) : ptr(obj.getObj()) {
        if (ptr) Tcl_IncrRefCount(ptr);
    }

    ~SWObject() {
		if (ptr) Tcl_DecrRefCount(ptr);
	}
    
    SWObject(const SWObject& obj) : ptr(obj.ptr) {
		if (ptr) Tcl_IncrRefCount(ptr);
	}

	SWObject& operator = (const SWObject& obj) {
		if (ptr) Tcl_DecrRefCount(ptr);
		ptr = obj.ptr;
		if (ptr) Tcl_IncrRefCount(ptr);
		return *this;
	}

	template <typename T>
	SWObject& MakeBasic(const T& what) {
		if (ptr) Tcl_DecrRefCount(ptr);
		ptr = MakeBaseSWObj(what);
		Tcl_IncrRefCount(ptr);
		return *this;
	}

    Tcl_Obj* getObj() const {
        ensure_exists();
		return ptr;
	}

    void ensure_exists() const {
        if (!ptr) {
			ptr = MakeBaseSWObj();
			Tcl_IncrRefCount(ptr);
		}

    }
};

class SWList : public SWObject {
public:
    SWList() : SWObject() { }
    SWList(const SWList& l) : SWObject(l) { }
    SWList& operator = (const SWList& obj) {
		SWObject::operator = (obj);
		return *this;
	}
	
	template <typename T>
	SWList(const std::vector<T> & vec) : SWObject() {
		for (size_t i=0; i<vec.size(); i++) {
			push_back(vec[i]);
		}
	}

    void ensure_exists() const {
		if (!ptr) {
			ptr = Tcl_NewListObj(0, NULL);
			Tcl_IncrRefCount(ptr);
		}
	}

    template <typename TPOD> 
    void push_back(const TPOD& what) {
		ensure_exists();
		Tcl_ListObjAppendElement(NULL, ptr, MakeBaseSWObj(what));
    }

    //template <> 
    void push_back(const SWObject& what) {
		ensure_exists();
		Tcl_ListObjAppendElement(NULL, ptr, what.getObj());
    }

    Tcl_Obj* getObj() const {
        ensure_exists();
		return ptr;
	}
};

inline Tcl_Obj* MakeBaseSWObj(const SWList &l) {
    // create string object
    return l.getObj();
}


class SWDict : public SWObject {
public:
    SWDict() : SWObject() { }
    SWDict(const SWDict& l) : SWObject(l) { }
    SWDict& operator = (const SWDict& obj) {
		SWObject::operator = (obj);
		return *this;
	}

    void ensure_exists() const {
		if (!ptr) {
			ptr = Tcl_NewDictObj();
			Tcl_IncrRefCount(ptr);
		}
	}
    
	template <typename TKey, typename TValue> 
    void insert(const TKey &key, const TValue &value) {
		ensure_exists();
		Tcl_DictObjPut(NULL, ptr, MakeBaseSWObj(key), MakeBaseSWObj(value));
    }

    template <typename TKey> 
    void insert(const TKey &key, const SWObject& value) {
		ensure_exists();
		Tcl_DictObjPut(NULL, ptr, MakeBaseSWObj(key), value.getObj());
    }

    Tcl_Obj* getObj() const {
        ensure_exists();
		return ptr;
	}
};

inline Tcl_Obj* MakeBaseSWObj(const SWDict &d) {
    // create string object
    return d.getObj();
}

#endif
#endif // SWIGTCL

#ifdef SWIGPYTHON

#ifdef SWIG
%typemap(out) SWObject {
	$result =  $1.getObj();
}   

%typemap(out) SWList {
	$result =  $1.getObj();
}   

%typemap(out) SWDict {
	$result =  $1.getObj();
}   


%{
#include "SWObject.hpp"
%}

#else
// C++-compiler - Python version
#include <Python.h>
#include <string>

// create PyObject by overloaded functions
class SWList;
class SWDict;

inline PyObject* MakeBaseSWObj() {
    // create empty object. Easy in Tcl - Python? Refcounting?
    return Py_None;
}

inline PyObject* MakeBaseSWObj(int i) {
    // create integer object
    return PyLong_FromLong(i);
}

inline PyObject* MakeBaseSWObj(long l) {
    // create integer object
    return PyLong_FromLong(l);
}
 
inline PyObject* MakeBaseSWObj(double d) {
    // create double object.
    return PyFloat_FromDouble(d);
}

inline PyObject* MakeBaseSWObj(const std::string &s) {
    // create string object
    return PyString_FromStringAndSize(s.c_str(), s.size());
}


PyObject* MakeBaseSWObj(const SWList &o);

PyObject* MakeBaseSWObj(const SWDict &o);
    
class SWObject {
protected:
    mutable PyObject *ptr;
public:
    SWObject() : ptr(NULL) { }

    template <typename TObject> 
    SWObject(const TObject& obj) : ptr(obj.getObj()) {
        if (ptr) Py_INCREF(ptr);
    }

    ~SWObject() {
		if (ptr) Py_DECREF(ptr);
	}
    
    SWObject(const SWObject& obj) : ptr(obj.ptr) {
		if (ptr) Py_INCREF(ptr);
	}

	SWObject& operator = (const SWObject& obj) {
		if (ptr) Py_DECREF(ptr);
		ptr = obj.ptr;
		if (ptr) Py_INCREF(ptr);
		return *this;
	}

    PyObject* getObj() const {
        ensure_exists();
		return ptr;
	}

    void ensure_exists() const {
        if (!ptr) {
			ptr = MakeBaseSWObj();
			Py_INCREF(ptr);
		}

    }
};

class SWList : public SWObject {
public:
    SWList() : SWObject() { }
    SWList(const SWList& l) : SWObject(l) { }
    SWList& operator = (const SWList& obj) {
		SWObject::operator = (obj);
		return *this;
	}
	
	template <typename T>
	SWList(const std::vector<T> & vec) : SWObject() {
		for (size_t i=0; i<vec.size(); i++) {
			push_back(vec[i]);
		}
	}

    void ensure_exists() const {
		if (!ptr) {
			ptr = PyList_New(0);
			Py_INCREF(ptr);
		}
	}

    template <typename TPOD> 
    void push_back(const TPOD& what) {
		ensure_exists();
		PyList_Append(ptr, MakeBaseSWObj(what));
    }

    PyObject* getObj() const {
        ensure_exists();
		return ptr;
	}
};

inline PyObject* MakeBaseSWObj(const SWList &l) {
    // create string object
    return l.getObj();
}


class SWDict : public SWObject {
public:
    SWDict() : SWObject() { }
    SWDict(const SWDict& l) : SWObject(l) { }
    SWDict& operator = (const SWDict& obj) {
		SWObject::operator = (obj);
		return *this;
	}

    void ensure_exists() const {
		if (!ptr) {
			ptr = PyDict_New();
			Py_INCREF(ptr);
		}
	}

    template <typename TKey, typename TValue> 
    void insert(const TKey &key, const TValue &value) {
		ensure_exists();
		PyDict_SetItem(ptr, MakeBaseSWObj(key), MakeBaseSWObj(value));
    }

    PyObject* getObj() const {
        ensure_exists();
		return ptr;
	}
};

inline PyObject* MakeBaseSWObj(const SWDict &d) {
    // create string object
    return d.getObj();
}


#endif // C++
#endif //SWIGPYTHON

#endif // SWOBJECT_HPP

