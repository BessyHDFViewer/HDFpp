/*  hdfpp.hpp
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

/** Header file for HDF reader libraries
 **/

#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include "SWObject.hpp"


#define STHROW(msg) { \
	 std::ostringstream err;\
	 err<<msg; \
	 throw std::runtime_error(err.str()); }


// reading HDF4 files into nested lists/dicts
class HDFpp {
    int hdf_id;
    size_t ndatasets;
    size_t nglobal_attrs;
public:
    HDFpp(const char *fname);
    ~HDFpp();
    void close();
    size_t get_num_datasets() {
        return ndatasets;
    }
	std::string getname(size_t index);
    SWList readdata(size_t index);
    SWDict readattrs(size_t index);
	SWDict readglobalattrs();
	SWObject dump();
};

#ifdef HAVE_HDF5
// reading HDF5 files into nested lists/dicts
#undef VOID
// Tcl's define VOID clashes with typedef VOID in HDF5
#include "hdf5.h"
class H5pp {
	hid_t file;
public:
	H5pp(const char *fname);
	~H5pp();
	void close();
	SWObject dump(int maxlevel = 0, const char *root="/");
};
#endif
