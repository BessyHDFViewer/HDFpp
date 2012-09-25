#include "mfhdf.h"
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

    char sds_name[64]; int32 rank; int32 dimsizes[MAX_VAR_DIMS]; int32 data_type; int32 num_attrs;

    if (SDgetinfo(sds_id, sds_name, &rank, dimsizes, &data_type, &num_attrs)==FAIL) {
        STHROW("Error getting information for data set "<<index);
    }
	
	return string(sds_name);
}


static SWList readdata_internal(int32 sds_id, int32 rank, int32 *dimsizes, int32 data_type, int32 index) {
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
            STHROW("Data set "<<index<<" has data type "<<data_type<<" can only read 64bit float and 32bit int");
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

    char sds_name[64]; int32 rank; int32 dimsizes[MAX_VAR_DIMS]; int32 data_type; int32 num_attrs;

    if (SDgetinfo(sds_id, sds_name, &rank, dimsizes, &data_type, &num_attrs)==FAIL) {
        STHROW("Error getting information for data set "<<index);
    }
	
	return readdata_internal(sds_id, rank, dimsizes, data_type, index);
}


static SWDict readattr_internal(int32 sds_id, int32 num_attrs, int32 dset_index) {
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

    char sds_name[64]; int32 rank; int32 dimsizes[MAX_VAR_DIMS]; int32 data_type; int32 num_attrs;

    if (SDgetinfo(sds_id, sds_name, &rank, dimsizes, &data_type, &num_attrs)==FAIL) {
        STHROW("Error getting information for data set "<<index);
    }

    return readattr_internal(sds_id, num_attrs, index);
}


SWObject HDFpp::dump() {
	// dump the data sets as one big dictionary
	SWList result;
	for (size_t index = 0; index < get_num_datasets(); index++) {
		
		int32 sds_id;
		if ((sds_id=SDselect(hdf_id, index))==FAIL) {
			STHROW("Can't select data set nr. "<<index);
		}
    
		sds_release srelease(sds_id);

		char sds_name[64]; int32 rank; int32 dimsizes[MAX_VAR_DIMS]; int32 data_type; int32 num_attrs;
		if (SDgetinfo(sds_id, sds_name, &rank, dimsizes, &data_type, &num_attrs)==FAIL) {
			STHROW("Error getting information for data set "<<index);
		}

		SWDict entry; 
		SWDict attrs = readattr_internal(sds_id, num_attrs, index);
		SWList data = readdata_internal(sds_id, rank, dimsizes, data_type, index);
		entry.insert("name", sds_name);
		entry.insert("attrs", attrs);
		entry.insert("data", data);
		result.push_back(entry);
	}
	return result;
}

