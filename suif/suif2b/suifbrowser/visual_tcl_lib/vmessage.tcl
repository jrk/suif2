#-------------------------------------------------------------------------
# vmessage.tcl
#
#-------------------------------------------------------------------------

proc vmessage_create {w} {
    message $w.mesg -width 6i -foreground blue
    pack $w.mesg -anchor w
}

proc vmessage_destroy {w} {
    if {[winfo exists $w.mesg] == 1} {
	destroy $w.mesg
    }
}

proc vmessage_set {w text} {
    $w.mesg configure -text $text
}
