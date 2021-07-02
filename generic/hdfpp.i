/*  hdfpp.i
*   
*   (C) Copyright 2021 Physikalisch-Technische Bundesanstalt (PTB)
*   Christian Gollwitzer
*  
*   This file is part of BessyHDFViewer.
*
*   BessyHDFViewer is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   BessyHDFViewer is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with BessyHDFViewer.  If not, see <https://www.gnu.org/licenses/>.
** 
*/

/** SWIG include file to wrap the HDF reader classes
 **/


%{
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

%include SWObject.hpp
%include hdfpp.hpp
