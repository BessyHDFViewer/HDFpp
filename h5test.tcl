load ./hdfpp64[info sharedlibextension]

source hformat.tcl

H5pp h normiert00075.h5
set data [h dump]
h -delete

puts [hformat $data]
puts [hformat [dict get $data data c1 attrs]]
