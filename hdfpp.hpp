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
class H5pp {
	hid_t file;
public:
	H5pp(const char *fname);
	~H5pp();
	void close();
	SWObject dump();
};
#endif
