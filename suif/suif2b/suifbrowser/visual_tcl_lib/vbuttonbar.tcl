#-------------------------------------------------------------------------
# vbuttonbar.tcl
#
#-------------------------------------------------------------------------

#-------------------------------------------------------------------------
#
proc vbuttonbar_create {w obj} {
    global visual

    frame $w.b
    pack $w.b -fill x

    # local variables
    global $w.b
    set $w.b(num_buttons) 0
}

#----------------------------------------------------------------------
proc vbuttonbar_destroy {w} {
    if {[winfo exist $w.b] == 1} {
	destroy $w.b
    }
}

#----------------------------------------------------------------------
proc vbuttonbar_cleanup {w} {
    global $w.b
    catch {unset $w.b}
}

#----------------------------------------------------------------------
# clear the button bar
#
proc vbuttonbar_clear {w} {
    foreach child [winfo children $w.b] {
	destroy $child
    }
    global $w.b
    set $w.b(num_buttons) 0
}

#----------------------------------------------------------------------
# add a button
#
proc vbuttonbar_add_button {w buttonobj text} {
    global $w.b
    set but_num [set $w.b(num_buttons)]
    set buttonpath $w.b.but$but_num]
    incr $w.b(num_buttons)
    button $buttonpath -text $text -command \
	    "vbuttonbar invoke $buttonobj $but_num" \
	    -font "-*-courier-bold-r-normal--*-120-*"
    pack $buttonpath -side left -fill x -padx 4 -expand 1
}

#-------------------------------------------------------------------------
#source vdefaults.tcl
#toplevel .t
#vbuttonbar_create .t 0x0 
#vbuttonbar_add_button .t 0x0 "Hi"
#vbuttonbar_add_button .t 0x0 "Hello"

