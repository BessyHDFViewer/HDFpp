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

string HDFpp::getname(size_t index) {
	if (index>=ndatasets) STHROW("Only "<<ndatasets<<" data sets available, requested nr "<<index);
    int32 sds_id;
    
    if ((sds_id=SDselect(hdf_id, index))==FAIL) {
        STHROW("Can't select data set nr. "<<index);
    }
    
    char sds_name[64]; int32 rank; int32 dimsizes[MAX_VAR_DIMS]; int32 data_type; int32 num_attrs;

    if (SDgetinfo(sds_id, sds_name, &rank, dimsizes, &data_type, &num_attrs)==FAIL) {
        STHROW("Error getting information for data set "<<index);
    }
	
	return string(sds_name);
}


SWList HDFpp::readdata(size_t index) { 
    if (index>=ndatasets) STHROW("Only "<<ndatasets<<" data sets available, requested nr "<<index);
    int32 sds_id;
    
    if ((sds_id=SDselect(hdf_id, index))==FAIL) {
        STHROW("Can't select data set nr. "<<index);
    }
    
    char sds_name[64]; int32 rank; int32 dimsizes[MAX_VAR_DIMS]; int32 data_type; int32 num_attrs;

    if (SDgetinfo(sds_id, sds_name, &rank, dimsizes, &data_type, &num_attrs)==FAIL) {
        STHROW("Error getting information for data set "<<index);
    }

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
                STHROW("Error reading 64 bit float values from data set "<<sds_name<<" "<<index);
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
                STHROW("Error reading 32 bit int values from data set "<<sds_name<<" "<<index);
            }
            
            return SWList(buf);

            break;
        }

        default: {
            STHROW("Data set "<<sds_name<<" has data type "<<data_type<<" can only read 64bit float and 32bit int");
        }
    }
}


SWDict HDFpp::readattrs(size_t index) { 
    if (index>=ndatasets) STHROW("Only "<<ndatasets<<" data sets available, requested nr "<<index);
    int32 sds_id;
    
    if ((sds_id=SDselect(hdf_id, index))==FAIL) {
        STHROW("Can't select data set nr. "<<index);
    }
    
    char sds_name[64]; int32 rank; int32 dimsizes[MAX_VAR_DIMS]; int32 data_type; int32 num_attrs;

    if (SDgetinfo(sds_id, sds_name, &rank, dimsizes, &data_type, &num_attrs)==FAIL) {
        STHROW("Error getting information for data set "<<index);
    }

    SWDict result;

    //result.insert("Dataset name", sds_name);

    for (int i=0; i<num_attrs; i++) {
        char attr_name[64]; int32 data_type; int32 count;
        if (SDattrinfo(sds_id, i, attr_name, &data_type, &count)==FAIL) {
            STHROW("Error getting information on attribute "<<i<<", dataset "<<index<<" '"<<sds_name<<"'");
        }

        switch (data_type) {
            case DFNT_FLOAT32: {
                vector<float> buf(count);
                if (SDreadattr(sds_id, i, &(buf[0])) == FAIL) {
                    STHROW("Error reading 32 bit float values from attribute "<<attr_name<<" "<<i<<", data set "<<sds_name<<" "<<index);
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
                    STHROW("Error reading 64 bit float values from attribute "<<attr_name<<" "<<i<<", data set "<<sds_name<<" "<<index);
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
                    STHROW("Error reading 32 bit int values from attribute "<<attr_name<<" "<<i<<", data set "<<sds_name<<" "<<index);
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
                    STHROW("Error reading char values (string) from attribute "<<attr_name<<" "<<i<<", data set "<<sds_name<<" "<<index);
                }
                
                result.insert(attr_name, string(&(buf[0]), count));
                
                break;


            }

            default: {
                STHROW("Attribute "<<i<<" of data set "<<sds_name<<" has data type "<<data_type<<" can only read 64bit float, 32bit int and string (char8)");
            }
        }
    }

    return result;

}


SWObject HDFpp::dump() {
	// dump the data sets as one big dictionary
	SWList result;
	for (size_t i = 0; i < get_num_datasets(); i++) {
		SWDict entry; 
		string name = getname(i);
		SWDict attrs = readattrs(i);
		SWList data = readdata(i);
		entry.insert("name", name);
		entry.insert("attrs", attrs);
		entry.insert("data", data);
		result.push_back(entry);
	}
	return result;
}

