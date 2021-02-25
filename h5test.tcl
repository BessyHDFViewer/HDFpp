lappend auto_path .
load ./hdfpp64[info sharedlibextension]
#package require HDFpp

source hformat.tcl

# open H5 file
H5pp h normiert00075.h5

# dump all data into nested dict structure
set data [h dump]

puts "============= All data =============="
puts [hformat $data]
puts "============= One key ==============="
puts [hformat [dict get $data data c1 attrs]]

# optional parameters: max depth of the tree and start path
set subdata [h dump 1 "/c1"]
puts "============ Subtree with limited depth =============="
puts [hformat $subdata]

# close H5 file
h -delete

