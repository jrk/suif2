#-------------------------------------------------------------------------
# vtoplevel.tcl
#
#-------------------------------------------------------------------------

proc vtoplevel_create {w obj class} {
    if {$class != ""} {
	regsub -all " " $class "_" class_name
	toplevel $w -class $class_name
    } else {
	toplevel $w
    }
    catch {lower $w .vprogress}
    wm minsize $w 200 200
    bind $w <Destroy> "vtoplevel destroy $obj %W"
}

proc vtoplevel_place {w title} {
    # set geometry
    global visual
    foreach i $visual(geometry) {
	if {[lindex $i 0] == $title} {
	    wm geometry $w [lindex $i 1]
	    break
	}
    }
}

proc vtoplevel_destroy {w} {
    destroy $w
}

proc vtoplevel_set_title {w title} {
    wm title $w $title
}

proc vtoplevel_iconify {w} {
    wm iconify $w
}

proc vtoplevel_deiconify {w} {
    wm deiconify $w
}

proc vtoplevel_withdraw {w} {
    wm withdraw $w
}

proc vtoplevel_raise {w} {
    wm deiconify $w
    raise $w
}

proc vtoplevel_lower {w} {
    lower $w
}
