load ./hdfpp64[info sharedlibextension]

source hformat.tcl
proc bessy_reshape {fn} {
	set hdf [HDFpp %AUTO% $fn]
	set hlist [$hdf dump]
	$hdf -delete
	foreach dataset $hlist {
		set dname [dict get $dataset name]
		dict unset dataset name
		if {[catch {dict get $dataset attrs Name} name]} {
			# there is no name - put it directly
			set key [list $dname]
		} else {
			# sub-name in the attrs - put it as a subdict
			dict unset dataset attrs Name
			set key [list $dname $name]
		}
		
		if {[llength [dict get $dataset data]]==0} {
			# key contains only attrs -- put it directly there
			set dataset [dict get $dataset attrs]
		}

		dict set hdict {*}$key $dataset
	}
	return $hdict
}

puts [hformat [bessy_reshape fcm_201209_078.hdf]]
