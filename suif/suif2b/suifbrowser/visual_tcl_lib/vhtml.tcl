#----------------------------------------------------------------------
# vhtml.tcl
#
# Internals:
#
#----------------------------------------------------------------------

#----------------------------------------------------------------------
# vhtml_create
#
# create a text widget
#
proc vhtml_create {w obj} {
    # create text widget
    global visual

    text $w.text -relief groove -borderwidth 2 -width $visual(TEXT_COLUMNS) \
	    -height $visual(TEXT_ROWS) -wrap none\
	    -yscrollcommand "$w.yscroll set" -cursor {arrow}
    pack $w.text -fill both -expand 1
    scrollbar $w.yscroll -command "$w.text yview"
    pack $w.yscroll -side right -fill y
}

proc vhtml_destroy {w} {
    if {[winfo exist $w.text] == 1} {
	destroy $w.text $w.yscroll
    }
}

proc vhtml_cleanup {w} {
    global $w.text
    unset $w.text
}

#----------------------------------------------------------------------
# append text
#
proc vhtml_insert_text {w html} {
    HMinit_win $w.text
    HMparse_html $html "HMrender $w.text"
}

#----------------------------------------------------------------------
# clear the text buffer
#
proc vhtml_clear {w} {
    $w.text delete 1.0 end
}

#----------------------------------------------------------------------
# view a position
#
proc vhtml_view {w row col pickplace} {
    if {$pickplace == 1} {
	$w.text yview -pickplace $row.$col
    } else {
	$w.text yview $row.$col
    }
}

