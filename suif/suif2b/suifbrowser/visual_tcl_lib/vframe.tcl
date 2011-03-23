#-------------------------------------------------------------------------
# vframe.tcl
#
#-------------------------------------------------------------------------

proc vframe_create {w obj class { expand 0 } } {
    if {$class != ""} {
	regsub -all " " $class "_" class_name
	frame $w -class $class_name
    } else {
	frame $w
    }
    $w configure -relief raised
    pack $w -fill both -expand $expand

    bind $w <Destroy> "vframe_cleanup $w; vframe destroy $obj"
}

proc vframe_destroy {w} {
    if {[winfo exist $w] == 1} {
	destroy $w
    }
}

# clean up global variables
proc vframe_cleanup {w} {

}
