load ./hdfpp64.so

source hformat.tcl

H5pp h 00001.h5
set data [h dump]
h -delete

puts [hformat $data]
