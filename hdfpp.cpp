#include "mfhdf.h"
#include "hdf5.h"
#include "hdfpp.hpp"
using namespace std;

HDFpp::HDFpp(const char *fname) : hdf_id(0) {
    hdf_id = SDstart(fname, DFACC_READ);
    if (hdf_id==FAIL) STHROW("Can't open "<<fname);
    int32 num_datasets; int32 num_global_attrs;
    
    if (SDfileinfo(hdf_id, &num_datasets, &num_global_attrs) == FAIL) {
        STHROW("Error reading file information for "<<fname);
    }

    ndatasets = num_datasets;
    nglobal_attrs = num_global_attrs;
}

HDFpp::~HDFpp() {
    close();
}

void HDFpp::close() {
    SDend(hdf_id);
}

// ensure that SDendaccess is called for every possible exit - grrr
class sds_release {
	int32 id;
public:
	sds_release(int32 id) : id(id) { }
	~sds_release() {
		SDendaccess(id);
	}
};



string HDFpp::getname(size_t index) {
	if (index>=ndatasets) STHROW("Only "<<ndatasets<<" data sets available, requested nr "<<index);
    int32 sds_id;

    if ((sds_id=SDselect(hdf_id, index))==FAIL) {
        STHROW("Can't select data set nr. "<<index);
    }
    
    sds_release srelease(sds_id);

	uint16 nlen;
	if (SDgetnamelen(sds_id, &nlen)==FAIL) {
        STHROW("Error getting name length for data set "<<index);
	}

	vector <char> sds_name(nlen+1);
    int32 rank; int32 dimsizes[MAX_VAR_DIMS]; int32 data_type; int32 num_attrs;

    if (SDgetinfo(sds_id, &(sds_name[0]), &rank, dimsizes, &data_type, &num_attrs)==FAIL) {
        STHROW("Error getting information for data set "<<index);
    }
	
	return string(&(sds_name[0]), nlen);
}


static SWList readdata4_internal(int32 sds_id, int32 rank, int32 *dimsizes, int32 data_type, int32 index) {
    if (rank != 1) 
        STHROW("Data set has rank "<<rank<<", expected 1d array.");

    size_t nelements = dimsizes[0];

    int32 start = 0;

    switch (data_type) {
        case DFNT_FLOAT64: {
			if (nelements == 0) {
				return SWList();
			}

			vector<double> buf(nelements);
            if (SDreaddata(sds_id, &start, NULL, dimsizes, &(buf[0])) == FAIL) {
                STHROW("Error reading 64 bit float values from data set "<<index);
            }

			return SWList(buf);

            break;
        }
		
		case DFNT_FLOAT32: {
			if (nelements == 0) {
				return SWList();
			}

			vector<float> buf(nelements);
            if (SDreaddata(sds_id, &start, NULL, dimsizes, &(buf[0])) == FAIL) {
                STHROW("Error reading 32 bit float values from data set "<<index);
            }

			return SWList(buf);

            break;
        }

        case DFNT_INT32: {
			if (nelements == 0) {
				return SWList();
			}

            vector<int32> buf(nelements);
            if (SDreaddata(sds_id, &start, NULL, dimsizes, &(buf[0])) == FAIL) {
                STHROW("Error reading 32 bit int values from data set "<<index);
            }
            
            return SWList(buf);

            break;
        }

        default: {
            STHROW("Data set "<<index<<" has data type "<<data_type<<" can only read 64bit float, 32bit float  and 32bit int");
        }
    }
}

SWList HDFpp::readdata(size_t index) { 
    if (index>=ndatasets) STHROW("Only "<<ndatasets<<" data sets available, requested nr "<<index);
    int32 sds_id;
    
    if ((sds_id=SDselect(hdf_id, index))==FAIL) {
        STHROW("Can't select data set nr. "<<index);
    }
    
    sds_release srelease(sds_id);

    int32 rank; int32 dimsizes[MAX_VAR_DIMS]; int32 data_type; int32 num_attrs;

    if (SDgetinfo(sds_id, NULL, &rank, dimsizes, &data_type, &num_attrs)==FAIL) {
        STHROW("Error getting information for data set "<<index);
    }
	
	return readdata4_internal(sds_id, rank, dimsizes, data_type, index);
}


static SWDict readattr4_internal(int32 sds_id, int32 num_attrs, int32 dset_index) {
	SWDict result;

    for (int i=0; i<num_attrs; i++) {
	
		char attr_name[64]; int32 data_type; int32 count;
		if (SDattrinfo(sds_id, i, attr_name, &data_type, &count)==FAIL) {
			STHROW("Error getting information on attribute "<<i<<", dataset "<<dset_index);
		}

		switch (data_type) {
			case DFNT_FLOAT32: {
				vector<float> buf(count);
				if (SDreadattr(sds_id, i, &(buf[0])) == FAIL) {
					STHROW("Error reading 32 bit float values from attribute "<<i<<", dataset "<<dset_index);
				}

				if (count == 1) {
					result.insert(attr_name, buf[0]);
				} else {
					result.insert(attr_name, SWList(buf));
				}
				break;
			}


			case DFNT_FLOAT64: {
				vector<double> buf(count);
				if (SDreadattr(sds_id, i, &(buf[0])) == FAIL) {
					STHROW("Error reading 64 bit float values from attribute "<<i<<", dataset "<<dset_index);
				}

				if (count == 1) {
					result.insert(attr_name, buf[0]);
				} else {
					result.insert(attr_name, SWList(buf));
				}

				break;
			}

			case DFNT_INT32: {
				vector<int32> buf(count);
				if (SDreadattr(sds_id, i, &(buf[0])) == FAIL) {
					STHROW("Error reading 32 bit int values from attribute "<<i<<", dataset "<<dset_index);
				}
				
				if (count == 1) {
					result.insert(attr_name, buf[0]);
				} else {
					result.insert(attr_name, SWList(buf));
				}
				
				break;
			}

			case DFNT_CHAR8: {
				vector<char> buf(count);
				if (SDreadattr(sds_id, i, &(buf[0])) == FAIL) {
					STHROW("Error reading char values (string) from attribute "<<i<<", dataset "<<dset_index);
				}
				
				result.insert(attr_name, string(&(buf[0]), count));
				
				break;


			}

			default: {
				STHROW("Attribute "<<i<<" of data set "<<dset_index<<" has data type "<<data_type<<" can only read 64bit float, 32bit int and string (char8)");
			}
		}
	}

	return result;
}


SWDict HDFpp::readattrs(size_t index) { 
    if (index>=ndatasets) STHROW("Only "<<ndatasets<<" data sets available, requested nr "<<index);
    int32 sds_id;
    
    if ((sds_id=SDselect(hdf_id, index))==FAIL) {
        STHROW("Can't select data set nr. "<<index);
    }
    
    sds_release srelease(sds_id);

    int32 rank; int32 dimsizes[MAX_VAR_DIMS]; int32 data_type; int32 num_attrs;

    if (SDgetinfo(sds_id, NULL, &rank, dimsizes, &data_type, &num_attrs)==FAIL) {
        STHROW("Error getting information for data set "<<index);
    }

    return readattr4_internal(sds_id, num_attrs, index);
}

SWDict HDFpp::readglobalattrs() {
	// reading global attributes is implemented
	// by reading the attributes from the hdf_id
	return readattr4_internal(hdf_id, nglobal_attrs, -1);
	// -1 is just a fudge value, only for the error messages
}


SWObject HDFpp::dump() {
	// dump the data sets as one big dictionary
	SWList result;
	if (nglobal_attrs != 0) {
		// if we have global attributes, insert them as a first dataset
		// with an empty name
		SWDict entry;
		SWDict attrs = readglobalattrs();
		SWList data; // empty list
		entry.insert("name", string());
		entry.insert("attrs", attrs);
		entry.insert("data", data);
		result.push_back(entry);
	}
	for (size_t index = 0; index < get_num_datasets(); index++) {
		
		int32 sds_id;
		if ((sds_id=SDselect(hdf_id, index))==FAIL) {
			STHROW("Can't select data set nr. "<<index);
		}
    
		sds_release srelease(sds_id);
		
		uint16 nlen;
		if (SDgetnamelen(sds_id, &nlen)==FAIL) {
			STHROW("Error getting name length for data set "<<index);
		}

		vector <char> sds_name(nlen+1);
		int32 rank; int32 dimsizes[MAX_VAR_DIMS]; int32 data_type; int32 num_attrs;

		if (SDgetinfo(sds_id, &(sds_name[0]), &rank, dimsizes, &data_type, &num_attrs)==FAIL) {
			STHROW("Error getting information for data set "<<index);
		}
		
		SWDict entry; 
		SWDict attrs = readattr4_internal(sds_id, num_attrs, index);
		SWList data = readdata4_internal(sds_id, rank, dimsizes, data_type, index);
		entry.insert("name", string(&(sds_name[0]), nlen));
		entry.insert("attrs", attrs);
		entry.insert("data", data);
		result.push_back(entry);
	}
	return result;
}


H5pp::H5pp(const char *fname) : file(-1) {
	file = H5Fopen (fname, H5F_ACC_RDONLY, H5P_DEFAULT);
	if (file<0) {
		// can't read the file
		STHROW("Can't open "<<fname);
	}
}

H5pp::~H5pp () { close(); }


void H5pp::close() {
	if (file>=0) {
		H5Fclose(file);
	}
}

void readlink5_internal(hid_t loc_id, const char *name, const H5L_info_t* info, SWDict& linkdata);
void readattr5_internal(hid_t loc_id, const char *name, SWDict& attrs);
void readdataset5_internal(hid_t loc_id, const char *name, SWDict& datasetdata);
void readdatatype5_internal(hid_t loc_id, const char *name, SWDict& datatypedata);

void readgroup5_recursive(hid_t loc_id, const char *name, SWDict& groupdump);

SWObject H5pp::dump() {
	SWDict result;
	// read root group of HDF5
	readgroup5_recursive(file, "/", result);
	return result;
}

extern "C" herr_t dumpgroup_callback (hid_t loc_id, const char *name, const H5L_info_t *info, void *operator_data);

void readgroup5_recursive(hid_t loc_id, const char *name, SWDict& groupdump) {
	groupdump.insert("type", "GROUP");
	groupdump.insert("name", name);
	
	SWDict attrs;
	readattr5_internal(loc_id, name, attrs);
	groupdump.insert("attrs", attrs);

	SWList data;
	H5Literate_by_name (loc_id, name, H5_INDEX_NAME,
                            H5_ITER_NATIVE, NULL, dumpgroup_callback, (void *) &data,
                            H5P_DEFAULT);

	groupdump.insert("data", data);

}

herr_t dumpgroup_callback (hid_t loc_id, const char *name, const H5L_info_t *info, void *operator_data) {
	SWList& groupdata = *((SWList*) operator_data);

	if (info->type == H5L_TYPE_SOFT) {
		// soft link. 
		SWDict sldata;
		readlink5_internal(loc_id, name, info, sldata);
		// Insert soft link data into list
		groupdata.push_back(sldata);
		return 0; // Success
	}

	// other types: get object info
	H5O_info_t      infobuf;
	herr_t status = H5Oget_info_by_name (loc_id, name, &infobuf, H5P_DEFAULT);
    
	if (status<0) {
		// pass error on
		return status;
	}

    switch (infobuf.type) {
        case H5O_TYPE_GROUP: {
			/* Loop detector - disabled for now ***
            if ( group_check (od, infobuf.addr) ) {
                printf ("%*s  Warning: Loop detected!\n", spaces, "");
            } **/

			SWDict subgroupdata;
			readgroup5_recursive(loc_id, name, subgroupdata);
			groupdata.push_back(subgroupdata);
            break;
		}
        case H5O_TYPE_DATASET: {
            SWDict datasetdata;
			readdataset5_internal(loc_id, name, datasetdata);
			groupdata.push_back(datasetdata);
            break;
		}
        case H5O_TYPE_NAMED_DATATYPE: {
            SWDict datatypedata;
			readdatatype5_internal(loc_id, name, datatypedata);
			groupdata.push_back(datatypedata);
            break;
		}
        default: {
            // Unknown. Append mock object
			SWDict unknown;
			unknown.insert("type", "UNKNOWN");
			unknown.insert("name", name);
			groupdata.push_back(unknown);
		}
    }

    return 0; //Success
}

void readlink5_internal(hid_t loc_id, const char *name, const H5L_info_t* info, SWDict& linkdata) { 
	vector<char> targbuf(info->u.val_size+1);

	if (H5Lget_val(loc_id, name, &targbuf[0], info->u.val_size, H5P_DEFAULT)) {
		return; //Error
	}
	
	SWDict attrs;
	readattr5_internal(loc_id, name, attrs);

	linkdata.insert("type", "SOFTLINK");
	linkdata.insert("name", name);
	linkdata.insert("attrs", attrs);

	linkdata.insert("data", &targbuf[0]);

}


void readattr5_internal(hid_t loc_id, const char *name, SWDict& attrs) {
	
}

// replicate HDF5 native datatypes for 
// the use in switch statements
enum my_dtype {
	MY_UNKNOWN,
	MY_NATIVE_CHAR,
	MY_NATIVE_SHORT,
	MY_NATIVE_INT,
	MY_NATIVE_LONG,
	MY_NATIVE_LLONG,
	MY_NATIVE_UCHAR,
	MY_NATIVE_USHORT,
	MY_NATIVE_UINT,
	MY_NATIVE_ULONG,
	MY_NATIVE_ULLONG,
	MY_NATIVE_FLOAT,
	MY_NATIVE_DOUBLE,
	MY_NATIVE_LDOUBLE,
	MY_NATIVE_C_S1
};

my_dtype h5t_to_my(hid_t dtype) {
	// tedious task to ask for type identity
	if (H5Tequal(dtype, H5T_NATIVE_CHAR)) return MY_NATIVE_CHAR;
	if (H5Tequal(dtype, H5T_NATIVE_SHORT)) return MY_NATIVE_SHORT;
	if (H5Tequal(dtype, H5T_NATIVE_INT)) return MY_NATIVE_INT;
	if (H5Tequal(dtype, H5T_NATIVE_LONG)) return MY_NATIVE_LONG;
	if (H5Tequal(dtype, H5T_NATIVE_LLONG)) return MY_NATIVE_LLONG;
	if (H5Tequal(dtype, H5T_NATIVE_UCHAR)) return MY_NATIVE_UCHAR;
	if (H5Tequal(dtype, H5T_NATIVE_USHORT)) return MY_NATIVE_USHORT;
	if (H5Tequal(dtype, H5T_NATIVE_UINT)) return MY_NATIVE_UINT;
	if (H5Tequal(dtype, H5T_NATIVE_ULONG)) return MY_NATIVE_ULONG;
	if (H5Tequal(dtype, H5T_NATIVE_ULLONG)) return MY_NATIVE_ULLONG;
	if (H5Tequal(dtype, H5T_NATIVE_FLOAT)) return MY_NATIVE_FLOAT;
	if (H5Tequal(dtype, H5T_NATIVE_DOUBLE)) return MY_NATIVE_DOUBLE;
	if (H5Tequal(dtype, H5T_NATIVE_LDOUBLE)) return MY_NATIVE_LDOUBLE;
	if (H5Tget_class(dtype) == H5T_STRING) return MY_NATIVE_C_S1;
	return MY_UNKNOWN;
}

void readdataset5_internal(hid_t loc_id, const char *name, SWDict& datasetdata) {
	SWDict attrs;
	readattr5_internal(loc_id, name, attrs);

	datasetdata.insert("type", "DATASET");
	datasetdata.insert("name", name);
	datasetdata.insert("attrs", attrs);

	// open data set
	hid_t dset = H5Dopen(loc_id, name, H5P_DEFAULT);
	// get data space & type
	hid_t dspace = H5Dget_space(dset);
	hid_t dtype  = H5Dget_type(dset);

	SWList dspace_list;
	int rank = H5Sget_simple_extent_ndims(dspace);
	vector<hsize_t> extents(rank);
	H5Sget_simple_extent_dims(dspace, &extents[0], NULL);
	dspace_list.push_back(extents);
	datasetdata.insert("dspace", dspace_list);

    hsize_t nelements = (rank > 0)?1:0;
	for (size_t ind = 0; ind < rank; ind++) {
		nelements *= extents[ind];
	}
	datasetdata.insert("ndata", nelements);

	hid_t native_dtype = H5Tget_native_type(dtype, H5T_DIR_ASCEND);
	bool isatomic=false;
	size_t nmembers=0;
	switch (H5Tget_class(native_dtype)) {
		case H5T_INTEGER : { 
			datasetdata.insert("dtype", "integer");
			isatomic=true;
			nmembers = 1;
			break;
		}
		case H5T_FLOAT: {
			datasetdata.insert("dtype", "float");
			isatomic=true;
			nmembers = 1;
			break;
		}
		case H5T_STRING: {
			datasetdata.insert("dtype", "string");
			isatomic=true;
			nmembers = 1;
			break;
		}
		case H5T_COMPOUND: {
			SWList idlist;
			// create list of identifiers
			nmembers = H5Tget_nmembers(native_dtype);
			for (int idx=0; idx<nmembers; idx++) {
				char * name = H5Tget_member_name(native_dtype, idx);
				idlist.push_back(name);
				free(name);
			}
			datasetdata.insert("dtype", idlist);
			isatomic=false;
			break;
		}
		default: {
		}
	}

	// now read the data into memory buffer
	size_t elsize = H5Tget_size(native_dtype);
	size_t memsize = elsize*nelements;
	SWList data;
	vector<char> bufferspace(memsize);
	char * buf = &bufferspace[0];
	H5Dread(dset, native_dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf);

	// compute offsets and data types as constants
	vector<ssize_t> eloffsets(nmembers);
	vector<my_dtype> eltypes(nmembers);
	if (isatomic) {
		// only one element
		eloffsets[0]=0;
		eltypes[0]=h5t_to_my(native_dtype);
	} else {
		// multiple elements
		for (size_t ind=0; ind<nmembers; ind++) {
			eloffsets[ind]=H5Tget_member_offset(native_dtype, ind);
			hid_t mtype = H5Tget_member_type(native_dtype, ind);
			eltypes[ind] = h5t_to_my(mtype);
			H5Tclose(mtype);
		}
	}
	// big conversion switch loop - puh

#define DATACONV(MY_TYPE, CTYPE, SWTYPE) \
		case MY_TYPE: {\
			data.push_back(static_cast<SWTYPE>(*(reinterpret_cast<CTYPE*>(dbuf))));\
			break;\
		}

	
	for (char * el = buf; el < buf+memsize; el +=elsize) {
		//iterate over all elements in this dataset
		for (size_t ind=0; ind<nmembers; ind++) {
			char * dbuf = el + eloffsets[ind];
			switch (eltypes[ind]) {
				// decide about datatype
			/*	case MY_NATIVE_CHAR: {
					data.push_back(static_cast<int>(*(reinterpret_cast<char*>(dbuf))));
					break;
				} */
				DATACONV(MY_NATIVE_CHAR, char, int)
				DATACONV(MY_NATIVE_SHORT,short, int)
				DATACONV(MY_NATIVE_INT, int, int)
				DATACONV(MY_NATIVE_LONG,long, long)
				DATACONV(MY_NATIVE_LLONG,long long, long long)
				DATACONV(MY_NATIVE_UCHAR,unsigned char, int)
				DATACONV(MY_NATIVE_USHORT,unsigned short, int)
				DATACONV(MY_NATIVE_UINT, unsigned int, long)
				DATACONV(MY_NATIVE_ULONG,unsigned long, long long)
				DATACONV(MY_NATIVE_ULLONG,unsigned long long, unsigned long long)
				DATACONV(MY_NATIVE_FLOAT,float, float)
				DATACONV(MY_NATIVE_DOUBLE,double, double)
				DATACONV(MY_NATIVE_LDOUBLE,long double, double)
				case MY_NATIVE_C_S1: {
					data.push_back(dbuf);
					break;
				} 

				default: {
					data.push_back("???");
				}

			}
		}
	}
	datasetdata.insert("data", data);

	// close type&space
	H5Tclose(native_dtype);
	H5Tclose(dtype);
	H5Sclose(dspace);
	// close data set
	H5Dclose(dset);
}

void readdatatype5_internal(hid_t loc_id, const char *name, SWDict& datatypedata) {	
	SWDict attrs;
	readattr5_internal(loc_id, name, attrs);

	datatypedata.insert("type", "DATATYPE");
	datatypedata.insert("name", name);
	datatypedata.insert("attrs", attrs);

	// now read the data
	SWList data;
	datatypedata.insert("data", data);
 }




