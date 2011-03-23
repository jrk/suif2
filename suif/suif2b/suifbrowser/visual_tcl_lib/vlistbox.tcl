#----------------------------------------------------------------------
# vlistbox.tcl
#
#----------------------------------------------------------------------

proc vlistbox_create {w title obj { horizontal_slider 0 }  } {
    global visual
    label $w.title -text $title -relief raised \
	    -background $visual(LISTBOX_TITLE_BACKGROUND) \
	    -foreground $visual(LISTBOX_TITLE_FOREGROUND)
    pack $w.title -side top -anchor w -padx 4 -pady 4 -fill x

    scrollbar $w.yscroll -command "$w.l yview" -width 12
    pack $w.yscroll -side right -fill y

    if { $horizontal_slider == 1 }  {
      scrollbar $w.xscroll -orient horizontal -command "$w.l xview" -width 12
      pack $w.xscroll -side bottom -fill x
    }

    listbox $w.l -relief sunken -yscrollcommand "$w.yscroll set"
    pack $w.l -fill both -expand 1 -padx 2 -pady 2

    bind $w.l <Button-1> "vlist_select $w %x %y {vlistbox select $obj}"
    bind $w.l <B1-Motion> "vlist_select $w %x %y {vlistbox select $obj}"
}

proc vlistbox_destroy {w} {
    if {[winfo exists $w.l] == 1} {
	destroy $w.l
    }
}

proc vlistbox_add {w text} {
    $w.l insert end $text
}

proc vlistbox_clear {w} {
    $w.l delete 0 end
}

proc vlist_select {w x y script} {
    set index [$w.l index @$x,$y]
    $w.l activate $index
    eval "$script $index"
}

#----------------------------------------------------------------------
#toplevel .t
#wm minsize .t 100 100
#vlistbox_create .t "List of words"
#vlistbox_add .t "Hi"
#vlistbox_add .t "Hello"
