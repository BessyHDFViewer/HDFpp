
%module hdfpp
%{
#undef SWIG_version
#define SWIG_version "0.5"
#undef SWIG_TCL_STUBS_VERSION
#define SWIG_TCL_STUBS_VERSION "8.5"
%}

%include exception.i
%include typemaps.i
%include std_string.i
%{
#include "hdfpp.hpp"
%}

%init {
}

%exception {
  try {
    $function
  } 
  catch (const std::string &msg) {
    SWIG_exception(SWIG_RuntimeError, const_cast<char*>(msg.c_str()));
  } 
  SWIG_CATCH_STDEXCEPT
  catch (...) {
    SWIG_exception(SWIG_RuntimeError, "Some undefined C++-Error");
  }
}

#ifdef HAVE_64BIT
typedef unsigned long int size_t;
#else
typedef unsigned int size_t;
#endif


%include "std_vector.i"
namespace std {
    %template(dvec) vector<double>;
    %template(stringvec) vector<string>;
}

typedef std::vector<double> dvec;

%include SWObject.hpp
%include hdfpp.hpp
