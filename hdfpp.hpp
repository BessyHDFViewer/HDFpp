#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include "SWObject.hpp"


#define STHROW(msg) { \
	 std::ostringstream err;\
	 err<<msg; \
	 throw std::runtime_error(err.str()); }


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

